#include "http_parser.h"
#include "CommonCrossPlatform/CommonSpecifiers.h"
#include "CommonCrossPlatform/Common.h"
#include <assert.h>

#define NDEBUG

static http_parser *parser = nullptr;

static int currently_parsing_eof;

#define MAX_HEADERS 13
#define MAX_ELEMENT_SIZE 2048
#define MAX_CHUNKS 16

#define TRUE 1
#define FALSE 0

struct message {
	const char *name; // for debugging purposes
	const char *raw;
	enum http_parser_type type;
	enum http_method method;
	int status_code;
	char response_status[MAX_ELEMENT_SIZE];
	char request_path[MAX_ELEMENT_SIZE];
	char request_url[MAX_ELEMENT_SIZE];
	char fragment[MAX_ELEMENT_SIZE];
	char query_string[MAX_ELEMENT_SIZE];
	char body[MAX_ELEMENT_SIZE];
	size_t body_size;
	const char *host;
	const char *userinfo;
	uint16_t port;
	int num_headers;
	enum { NONE = 0, FIELD, VALUE } last_header_element;
	char headers[MAX_HEADERS][2][MAX_ELEMENT_SIZE];
	int should_keep_alive;

	int num_chunks;
	int num_chunks_complete;
	int chunk_lengths[MAX_CHUNKS];

	const char *upgrade; // upgraded body

	unsigned short http_major;
	unsigned short http_minor;

	int message_begin_cb_called;
	int headers_complete_cb_called;
	int message_complete_cb_called;
	int message_complete_on_eof;
	int body_is_final;
};

static struct message messages[16];
static int num_messages;
static http_parser_settings *current_pause_parser;

#ifdef _WIN32
#define my_memset memset
#define my_memcpy memcpy
#define my_strlen strlen
#define my_strnlen strnlen
#endif
/*
void my_memset(void *buffer, unsigned int value, unsigned int size) {
	for (unsigned int i = 0; i < size; ++i) {
		((char *)buffer)[i] = value;
	}
}

void my_memcpy(void *dest, const void *src, unsigned int size) {
	for (unsigned int i = 0; i < size; ++i) {
		((char *)dest)[i] = ((char *)src)[i];
	}
}

int my_strlen(const char *str) {
	const char *c = str;
	while (*c) c++;

	return c - str;
}

int my_strnlen(const char *str, int n) {
	for (int i = 0; i < n; ++i) {
		if ('\0' == str[i]) {
			return i;
		}
	}
	return n;
}*/

#ifdef __linux__
typedef void *(*my_memset_t) (void *s, int c, size_t n);
typedef void *(*my_memcpy_t) (void *dest, const void *src, size_t n);
typedef size_t (*my_strlen_t) (const char *s);
typedef size_t (*my_strnlen_t) (const char *s, size_t maxlen);
typedef void* (*my_malloc_t) (size_t size);
typedef void (*my_free_t) (void *ptr);
typedef void (*my_abort_t) (void);

my_memset_t my_memset;
my_memcpy_t my_memcpy;
my_strlen_t my_strlen;
my_strnlen_t my_strnlen;
my_malloc_t my_malloc;
my_free_t my_free;
my_abort_t my_abort;
#endif

void parser_init(enum http_parser_type type) {
	num_messages = 0;

	parser = (http_parser *)my_malloc(sizeof(*parser));
	my_memset(parser, 0, sizeof(struct http_parser));
	http_parser_init(parser, type);

	my_memset(&messages, 0, sizeof(messages));
}

void parser_free() {
	assert(parser);
	my_free(parser);
	parser = nullptr;
}

size_t strlncat(char *dst, size_t len, const char *src, size_t n) {
	size_t slen;
	size_t dlen;
	size_t rlen;
	size_t ncpy;

	slen = my_strnlen(src, n);
	dlen = my_strnlen(dst, len);

	if (dlen < len) {
		rlen = len - dlen;
		ncpy = slen < rlen ? slen : (rlen - 1);
		my_memcpy(dst + dlen, src, ncpy);
		dst[dlen + ncpy] = '\0';
	}

	assert(len > slen + dlen);
	return slen + dlen;
}

size_t strlcat(char *dst, const char *src, size_t len) {
	return strlncat(dst, len, src, (size_t)-1);
}

size_t strlncpy(char *dst, size_t len, const char *src, size_t n) {
	size_t slen;
	size_t ncpy;

	slen = my_strnlen(src, n);

	if (len > 0) {
		ncpy = slen < len ? slen : (len - 1);
		my_memcpy(dst, src, ncpy);
		dst[ncpy] = '\0';
	}

	assert(len > slen);
	return slen;
}

size_t strlcpy(char *dst, const char *src, size_t len) {
	return strlncpy(dst, len, src, (size_t)-1);
}


int message_begin_cb(http_parser *p) {
	assert(p == parser);
	messages[num_messages].message_begin_cb_called = TRUE;
	return 0;
}

int request_url_cb(http_parser *p, const char *buf, size_t len) {
	assert(p == parser);
	strlncat(messages[num_messages].request_url,
		sizeof(messages[num_messages].request_url),
		buf,
		len);
	return 0;
}

int response_status_cb(http_parser *p, const char *buf, size_t len)
{
	assert(p == parser);
	strlncat(messages[num_messages].response_status,
		sizeof(messages[num_messages].response_status),
		buf,
		len);
	return 0;
}

int
header_field_cb(http_parser *p, const char *buf, size_t len)
{
	assert(p == parser);
	struct message *m = &messages[num_messages];

	if (m->num_headers < MAX_HEADERS - 1) {
		return -1;
	}

	if (m->last_header_element != m->FIELD)
		m->num_headers++;

	strlncat(m->headers[m->num_headers - 1][0],
		sizeof(m->headers[m->num_headers - 1][0]),
		buf,
		len);

	m->last_header_element = m->FIELD;

	return 0;
}

int header_value_cb(http_parser *p, const char *buf, size_t len) {
	assert(p == parser);
	struct message *m = &messages[num_messages];

	strlncat(m->headers[m->num_headers - 1][1],
		sizeof(m->headers[m->num_headers - 1][1]),
		buf,
		len);

	m->last_header_element = m->VALUE;

	return 0;
}

void check_body_is_final(const http_parser *p) {
	if (messages[num_messages].body_is_final) {
		/*fprintf(stderr, "\n\n *** Error http_body_is_final() should return 1 "
			"on last on_body callback call "
			"but it doesn't! ***\n\n");*/
		assert(0);
		my_abort();
	}
	messages[num_messages].body_is_final = http_body_is_final(p);
}


int headers_complete_cb(http_parser *p) {
	assert(p == parser);
	messages[num_messages].method = (http_method)parser->method;
	messages[num_messages].status_code = parser->status_code;
	messages[num_messages].http_major = parser->http_major;
	messages[num_messages].http_minor = parser->http_minor;
	messages[num_messages].headers_complete_cb_called = TRUE;
	messages[num_messages].should_keep_alive = http_should_keep_alive(parser);
	return 0;
}

int body_cb(http_parser *p, const char *buf, size_t len) {
	assert(p == parser);
	strlncat(messages[num_messages].body,
		sizeof(messages[num_messages].body),
		buf,
		len);
	messages[num_messages].body_size += len;
	check_body_is_final(p);
	// printf("body_cb: '%s'\n", requests[num_messages].body);
	return 0;
}

int message_complete_cb(http_parser *p) {
	assert(p == parser);
	if (messages[num_messages].should_keep_alive != http_should_keep_alive(parser))
	{
		/*fprintf(stderr, "\n\n *** Error http_should_keep_alive() should have same "
			"value in both on_message_complete and on_headers_complete "
			"but it doesn't! ***\n\n");*/
		assert(0);
		my_abort();
	}

	if (messages[num_messages].body_size &&
		http_body_is_final(p) &&
		!messages[num_messages].body_is_final)
	{
		/*fprintf(stderr, "\n\n *** Error http_body_is_final() should return 1 "
			"on last on_body callback call "
			"but it doesn't! ***\n\n");*/
		assert(0);
		my_abort();
	}

	messages[num_messages].message_complete_cb_called = TRUE;

	messages[num_messages].message_complete_on_eof = currently_parsing_eof;

	if (num_messages < (sizeof(messages) / sizeof(messages[0]) - 1)) {
		num_messages++;
	} else {
		my_memset(messages + num_messages, 0, sizeof(messages[0]));
	}
	return 0;
}

int chunk_header_cb(http_parser *p) {
	assert(p == parser);
	int chunk_idx = messages[num_messages].num_chunks;
	messages[num_messages].num_chunks++;
	if (chunk_idx < MAX_CHUNKS) {
		messages[num_messages].chunk_lengths[chunk_idx] = p->content_length;
	}

	return 0;
}

int chunk_complete_cb(http_parser *p) {
	assert(p == parser);

	/* Here we want to verify that each chunk_header_cb is matched by a
	* chunk_complete_cb, so not only should the total number of calls to
	* both callbacks be the same, but they also should be interleaved
	* properly */
	assert(messages[num_messages].num_chunks ==
		messages[num_messages].num_chunks_complete + 1);

	messages[num_messages].num_chunks_complete++;
	return 0;
}

static http_parser_settings settings = { 
	message_begin_cb,
	request_url_cb,
	response_status_cb,
	header_field_cb,
	header_value_cb,
	headers_complete_cb,
	body_cb,
	message_complete_cb,
	chunk_header_cb,
	chunk_complete_cb
};

size_t parse (const char *buf, const size_t len) {
	size_t nparsed;
	currently_parsing_eof = (len == 0);
	nparsed = http_parser_execute(parser, &settings, buf, len);
	return nparsed;
}

void test_simple(const unsigned char *buf) {
	parser_init(HTTP_REQUEST);

	enum http_errno err;

	parse((const char *)buf, MAX_PAYLOAD_BUF);
	err = HTTP_PARSER_ERRNO(parser);

	parser_free();
	num_messages = 0;
}

extern "C" {
	DLL_PUBLIC unsigned char payloadBuffer[MAX_PAYLOAD_BUF];
	DLL_PUBLIC int Payload() {
		test_simple(payloadBuffer);
		return 0;
	}
};

#ifdef _WIN32
#include <Windows.h>
BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
) {
	return TRUE;
}
#else
#include <dlfcn.h>
__attribute__((constructor)) void somain(void) {
	void * libhandler = dlopen(("libc.so"), RTLD_LAZY);
	my_strlen = (my_strlen_t) dlsym(libhandler, "strlen");
	my_strnlen = (my_strnlen_t) dlsym(libhandler, "strnlen");
	my_memset = (my_memset_t) dlsym(libhandler, "memset");
	my_memcpy = (my_memcpy_t) dlsym(libhandler, "memcpy");
	my_malloc = (my_malloc_t) dlsym(libhandler, "malloc");
	my_free = (my_free_t) dlsym(libhandler, "free");
	my_abort = (my_abort_t) dlsym(libhandler, "abort");
}
#endif
