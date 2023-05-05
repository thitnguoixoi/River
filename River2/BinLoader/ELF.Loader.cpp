#include "Common.h"
#include "ELF.Loader.h"

#include <string.h>
#include <algorithm>

#define DONOTPRINT

#ifdef DONOTPRINT
#define dbg_log(fmt,...) ((void)0)
#else
#define dbg_log(fmt,...) {printf(fmt, ##__VA_ARGS__); fflush(stdout);}
#endif

#define SHT_NULL	 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB	 2
#define SHT_STRTAB	 3
#define SHT_RELA	 4
#define SHT_HASH	 5
#define SHT_DYNAMIC	 6
#define SHT_NOTE	 7
#define SHT_NOBITS	 8
#define SHT_REL		 9
#define SHT_SHLIB	 10
#define SHT_DYNSYM	 11
#define SHT_GNU_HASH 0x6FFFFFF6
#define SHT_VERNEED	 0x6FFFFFFE
#define SHT_VERDEF	 0x6FFFFFFD
#define SHT_VERSYM	 0x6FFFFFFF
#define SHT_LOPROC   0x70000000
#define SHT_HIPROC   0x7fffffff
#define SHT_LOUSER   0x80000000
#define SHT_HIUSER   0xffffffff

#define SHF_WRITE		0x1
#define SHF_ALLOC		0x2
#define SHF_EXECINSTR	0x4
#define SHF_MASKPROC	0xf0000000

namespace ldr {
	ELFSection::ELFSection() {
		data = nullptr;
		versions = nullptr;
	}

	bool ELFSection::Load(FILE *fModule) {
		if (SHT_NOBITS == header.sh_type) {
			return true;
		}

		data = new unsigned char[header.sh_size];

		fseek(fModule, header.sh_offset, SEEK_SET);
		if (header.sh_size != fread(data, 1, header.sh_size, fModule)) {
			dbg_log("read error, in section %s\n", (char *)header.sh_name);
			return false;
		}

		return true;
	}

	void ELFSection::Unload() {
		delete data;
	}

#define ELF_MAGIC 0x464C457F

#define ELFCLASSNONE		0
#define ELFCLASS32			1
#define ELFCLASS64			2

#define ELFDATANONE			0
#define ELFDATA2LSB			1
#define ELFDATA2MSB			2

#define ET_NONE				0
#define ET_REL				1
#define ET_EXEC				2
#define ET_DYN				3
#define ET_CORE				4
#define ET_LOPROC			0xff00
#define ET_HIPROC			0xffff

#define EM_NONE				0
#define EM_M32				1
#define EM_SPARC			2
#define EM_386				3
#define EM_68K				4
#define EM_88K				5
#define EM_860				7
#define EM_MIPS				8 

#define EV_NONE				0
#define EV_CURRENT			1

#define PT_NULL				0
#define PT_LOAD				1
#define PT_DYNAMIC			2
#define PT_INTERP			3
#define PT_NOTE				4
#define PT_SHLIB			5
#define PT_PHDR				6
#define PT_GNU_EH_FRAME		0x6474E550
#define PT_GNU_STACK		0x6474E551
#define PT_GNU_RELRO		0x6474E552
#define PT_LOPROC			0x70000000
#define PT_HIPROC			0x7fffffff

#define PF_R            0x4
#define PF_W            0x2
#define PF_X            0x1

#define ELF32RSYM(i) ((i)>>8)
#define ELF32RTYPE(i) ((unsigned char)(i))
#define ELF32RINFO(s,t) (((s)<<8)+(unsigned char)(t)

#define R_386_NONE			0 
#define R_386_32			1
#define R_386_PC32			2
#define R_386_GOT32			3
#define R_386_PLT32			4
#define R_386_COPY			5
#define R_386_GLOB_DAT		6
#define R_386_JMP_SLOT		7
#define R_386_RELATIVE		8
#define R_386_GOTOFF		9
#define R_386_GOTPC			10

#define DT_NULL				0   
#define DT_NEEDED			1   
#define DT_PLTRELSZ			2   
#define DT_PLTGOT			3   
#define DT_HASH				4   
#define DT_STRTAB			5   
#define DT_SYMTAB			6   
#define DT_RELA				7   
#define DT_RELASZ			8   
#define DT_RELAENT			9   
#define DT_STRSZ			10   
#define DT_SYMENT			11   
#define DT_INIT				12   
#define DT_INIT_ARRAY		25 /* Array with addresses Offf init fct */
#define DT_INIT_ARRAYSZ		27 /* Size in bytestes of DT_INIT_ARRAY */
#define DT_FINI				13   
#define DT_SONAME			14   
#define DT_RPATH			15   
#define DT_SYMBOLIC			16
#define DT_REL				17 
#define DT_RELSZ			18
#define DT_RELENT			19
#define DT_PLTREL			20
#define DT_DEBUG			21
#define DT_TEXTREL			22
#define DT_JMPREL			23
#define DT_LOPROC			0x70000000
#define DT_HIPROC			0x7fffffff

#define STB_LOCAL			0
#define STB_GLOBAL			1
#define STB_WEAK			2
#define STB_LOPROC			13
#define STB_HIPROC			15

#define SHN_UNDEF 0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_HIPROC 0xff1f
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff

#define VER_DEF_NONE		0
#define VER_DEF_CURRENT		1
#define VER_DEF_NUM			2

#define VER_FLG_BASE		1
#define VER_FLG_WEAK		2


#define ELF32STBIND(i) ((i)>>4)
#define ELF32STTYPE(i) ((i)&0xf

	struct ELF32Rel{
		DWORD r_offset;
		DWORD r_info;
	};

	struct ELF32Rela {
		DWORD r_offset;
		DWORD r_info;
		LONG r_addend;
	};

	struct ELF32Sym {
		DWORD st_name;
		DWORD st_value;
		DWORD st_size;
		unsigned char st_info;
		unsigned char st_other;
		WORD st_shndx;
	};

	struct ELF32Dyn {
		LONG d_tag;
		DWORD d_un;
	};

	struct ELF32VerNeed {
		WORD vn_version;
		WORD vn_cnt;
		DWORD vn_file;
		DWORD vn_aux;
		DWORD vn_next;
	};

	struct ELF32VerNeedAux {
		DWORD vna_hash;
		WORD vna_flags;
		WORD vna_other;
		DWORD vna_name;
		DWORD vna_next;
	};

	struct ELF32VerDef {
		WORD vd_version;
		WORD vd_flags;
		WORD vd_ndx;
		WORD vd_cnt;
		DWORD vd_hash;
		DWORD vd_aux;
		DWORD vd_next;
	};

	struct ELF32VerDefAux {
		DWORD vda_name;
		DWORD vda_next;
	};


	void *FloatingELF32::RVA(DWORD rva) const {
		for (auto i = sections.begin(); i != sections.end(); ++i) {
			if ((i->header.sh_addr <= rva) && (i->header.sh_addr + i->header.sh_size > rva) &&
					(i->header.sh_type != SHT_NOBITS)) {
				return &i->data[rva - i->header.sh_addr];
			}
		}
		return nullptr;
	}

	bool FloatingELF32::CanLoad(FILE *fMod) {
		ElfIdent ident;

		fseek(fMod, 0, SEEK_SET);

		if (1 != fread(&ident, sizeof(ident), 1, fMod)) {
			return false;
		}

		if (ELF_MAGIC != ident.e_magic) {
			return false;
		}

		if ((ELFCLASS32 != ident.e_class) || (ELFDATA2LSB != ident.e_data)) {
			return false;
		}

		// the file seems to be a falid ELF32 module
		return true;
	}

	bool FloatingELF32::LoadELF(FILE *fModule) {
		if (NULL == fModule) {
			dbg_log("File open error!\n");
			return false;
		}

		if (1 != fread(&ident, sizeof(ident), 1, fModule)) {
			dbg_log("Read error\n");
			return false;
		}

		if (ELF_MAGIC != ident.e_magic) {
			dbg_log("ident.e_magic not valid %08x != %08lx\n", ELF_MAGIC, ident.e_magic);
			return false;
		}

		if ((ELFCLASS32 != ident.e_class) || (ELFDATA2LSB != ident.e_data)) {
			dbg_log("ident.e_class or ident.e_data not supported\n");
			return false;
		}

		if (1 != fread(&header, sizeof(header), 1, fModule)) {
			dbg_log("Read error\n");
			return false;
		}

		if (EM_386 != header.e_machine) {
			dbg_log("header.e_machine not supported\n");
			return false;
		}

		//dbg_log("Base: %08lx\n", header.)

		moduleBase = 0xFFFFF000;
		pHeaders.resize(header.e_phnum);
		fseek(fModule, header.e_phoff, SEEK_SET);
		dbg_log("Type     Offset   V.Addr   P.Addr   F.Size   M.Size   Flags    Align\n");
		for (int i = 0; i < header.e_phnum; ++i) {
			if (1 != fread(&pHeaders[i].header, sizeof(pHeaders[i].header), 1, fModule)) {
				dbg_log("read error\n");
				return false;
			}

			DWORD tBA = 0xFFFFF000;
			switch (pHeaders[i].header.p_type) {
				case PT_LOAD:
					tBA = pHeaders[i].header.p_vaddr & ~(pHeaders[i].header.p_align - 1);
					break;
			};
			if (tBA < moduleBase) {
				moduleBase = tBA;
			}
		

			dbg_log("%08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
				pHeaders[i].header.p_type,
				pHeaders[i].header.p_offset,
				pHeaders[i].header.p_vaddr,
				pHeaders[i].header.p_paddr,
				pHeaders[i].header.p_filesz,
				pHeaders[i].header.p_memsz,
				pHeaders[i].header.p_flags,
				pHeaders[i].header.p_align
			);

		}

		sections.resize(header.e_shnum);
		fseek(fModule, header.e_shoff, SEEK_SET);
		for (int i = 0; i < header.e_shnum; ++i) {
			if (1 != fread(&sections[i].header, sizeof(sections[i].header), 1, fModule)) {
				dbg_log("read error\n");
				return false;
			}

			if (SHT_STRTAB == sections[i].header.sh_type) {
				names = &sections[i];
			}
		}

		for (int i = 0; i < header.e_shnum; ++i) {
			if (!sections[i].Load(fModule)) {
				return false;
			}

			switch (sections[i].header.sh_type) {
				case SHT_DYNAMIC :
					dbg_log("Dynamic section\n");
					ParseDynamic(sections[i]);
					break;
				case SHT_VERSYM :
					dbg_log("Versym section\n");
					sections[sections[i].header.sh_link].versions = &sections[i];
					break;
				case SHT_VERNEED :
					dbg_log("Verneed section\n");
					gnu_versions_r = &sections[i];
					ParseVerNeed(sections[i]);
					break;
				case SHT_VERDEF:
					dbg_log("Verdef section\n");
					gnu_versions_d = &sections[i];
					ParseVerDef(sections[i]);
					break;
				default:
					dbg_log("Section type %08lx - not recognized\n", sections[i].header.sh_type);
			}
		}

		/*dbg_log("\nName                     Type     Flags    Addr     Offset   Size     Link     Info     AddrAlgn EntSize\n");
		for (int i = 0; i < header.e_shnum; ++i) {
			dbg_log("%24s %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
				&sections[header.e_shstrndx].data[sections[i].header.sh_name],
				sections[i].header.sh_type,
				sections[i].header.sh_flags,
				sections[i].header.sh_addr,
				sections[i].header.sh_offset,
				sections[i].header.sh_size,
				sections[i].header.sh_link,
				sections[i].header.sh_info,
				sections[i].header.sh_addralign,
				sections[i].header.sh_entsize
			);
		}*/

		return true;
	}

	FloatingELF32::FloatingELF32(const char *moduleName) :start(0), init_array(0)
	{
		// TODO fix this fclose double free
		FILE *fModule = nullptr;
		
		if (0 != FOPEN(fModule, moduleName, "rb")) {
			isValid = false;
			return;
		}

		isValid = LoadELF(fModule);
		fclose(fModule);
	}

	FloatingELF32::FloatingELF32(const wchar_t *moduleName) :start(0), init_array(0)
	{
		FILE *fModule = nullptr;
		
		if (0 != W_FOPEN(fModule, moduleName, L"rb")) {
			isValid = false;
			return;
		}

		isValid = LoadELF(fModule);
		fclose(fModule);
	}

	FloatingELF32::~FloatingELF32() {
		for (auto i = sections.begin(); i != sections.end(); ++i) {
			i->Unload();
		}
	}

	DWORD FloatingELF32::GetEntryPoint() const {
		if (!isValid)
			return 0;
		return header.e_entry;
	}

	DWORD FloatingELF32::Import(AbstractImporter &impr, const char *name) {
		DWORD ret;
		const char *at = strchr(name, '@');

		if (at && at[1] == '@') {
			// retrieve version
			char funcname[100];
			strncpy(funcname, name, at - name);
			funcname[at - name] = '\0';

			if  (nullptr != gnu_versions_r) {
				for (auto v = gnu_versions_r->snVers.begin(); v != gnu_versions_r->snVers.end(); ++v) {
					if (!strcmp(v->version.c_str(), at + 2)) {
						return impr.FindImport(v->module.c_str(), funcname, at + 2);
					}
				}
			}

		}
		for (auto l = libraries.begin(); l != libraries.end(); ++l) {
			ret = impr.FindImport(l->c_str(), name);

			if (IMPORT_NOT_FOUND != ret) {
				return ret;
			}
		}

		return 0;
	}

	bool FloatingELF32::FixImports(AbstractImporter &impr, DWORD offset) {
		for (auto i = sections.begin(); i != sections.end(); ++i) {
			if ((SHT_DYNSYM == i->header.sh_type) || (SHT_SYMTAB == i->header.sh_type)) {
				ELF32Sym *symb = (ELF32Sym *)i->data;
				DWORD count = i->header.sh_size / sizeof(*symb);
				for (DWORD j = 0; j < count; ++j) {
					if ((STB_GLOBAL == ELF32STBIND(symb[j].st_info)) && (SHN_UNDEF == symb[j].st_shndx)) {
						dbg_log(
							"%s",
							(char *)&sections[i->header.sh_link].data[symb[j].st_name]
						);

						if (nullptr != i->versions && nullptr != gnu_versions_r) {
							ELFSymbolVersionNeeded *vers = gnu_versions_r->idxSnVers[((WORD *)i->versions->data)[j]];
							if (nullptr != vers) {
								dbg_log(
									"##%s",
									vers->version.c_str()
								);

								symb[j].st_value = impr.FindImport(vers->module.c_str(), (char *)&sections[i->header.sh_link].data[symb[j].st_name], vers->version.c_str());
							} else {
								symb[j].st_value = Import(impr, (char *)&sections[i->header.sh_link].data[symb[j].st_name]);
							}
						} else {
							symb[j].st_value = Import(impr, (char *)&sections[i->header.sh_link].data[symb[j].st_name]);
						}
						//symb[j].st_value -= offset;
						//hack because s.st_value is normally the offset of a symbol in ITS library
						dbg_log("\n");
					}
				}
			}
		}
		
		return false;
	}

	bool FloatingELF32::PrintSymbols() const {
		for (auto i = sections.begin(); i != sections.end(); ++i) {
			if ((SHT_DYNSYM == i->header.sh_type) || (SHT_SYMTAB == i->header.sh_type)) {
				ELF32Sym *symb = (ELF32Sym *)i->data;
				DWORD count = i->header.sh_size / sizeof(*symb);
				for (DWORD j = 0; j < count; ++j) {
					if ((STB_GLOBAL == ELF32STBIND(symb[j].st_info)) && (SHN_UNDEF == symb[j].st_shndx)) {
						dbg_log(
							"%s",
							(char *)&sections[i->header.sh_link].data[symb[j].st_name]
						);

						if (nullptr != i->versions && nullptr != gnu_versions_r) {
							ELFSymbolVersionNeeded *vers = gnu_versions_r->idxSnVers[((WORD *)i->versions->data)[j]];

							if (nullptr != vers) {
								dbg_log(
									"@@%s",
									vers->version.c_str()
								);
							}
						}
						dbg_log("\n");
					}
				}
			}
		}

		return true;
	}

	bool FloatingELF32::GetExport(const char *funcName, DWORD &funcRVA) const {
		for (auto i = sections.begin(); i != sections.end(); ++i) {
			if (SHT_DYNSYM == i->header.sh_type) {
				ELF32Sym *symb = (ELF32Sym *)i->data;
				DWORD count = i->header.sh_size / sizeof(*symb);
				for (DWORD j = 0; j < count; ++j) {
					if ((STB_GLOBAL == ELF32STBIND(symb[j].st_info)) &&
							(SHN_UNDEF != symb[j].st_shndx)) {
						if (!strcmp(funcName, (char *)&sections[i->header.sh_link].data[symb[j].st_name])) {
							//we found it
							funcRVA = symb[j].st_value;
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	bool FloatingELF32::RelocateSection(void *r, DWORD count, const ELFSection &symb, const ELFSection &names, DWORD offset) {
		ELF32Rel *rels = (ELF32Rel *)r;
			for (DWORD i = 0; i < count; ++i) {
			dbg_log("Off: 0x%08lx, Sym: 0x%06lx, Typ: 0x%02x ", rels[i].r_offset, ELF32RSYM(rels[i].r_info), ELF32RTYPE(rels[i].r_info));
			DWORD *addr = (DWORD *)RVA(rels[i].r_offset);
			DWORD oldAddr;
			WORD sectionIndex;
			ELF32Sym *s;

			switch (ELF32RTYPE(rels[i].r_info)) {
				case R_386_NONE :
					dbg_log("$ none");
					break;
				case R_386_32 :
					s = &((ELF32Sym *)symb.data)[ELF32RSYM(rels[i].r_info)];
					sectionIndex = s->st_shndx;
					oldAddr = *addr;
					*addr += s->st_value;
					if (sectionIndex != SHN_UNDEF)
						*addr += offset; //hack because the imports have absolute addrs and static calls have relative addrs
					dbg_log("$ %s st %lx off %lx res_addr %lx oldaddr %lx", (char *)&names.data[s->st_name], s->st_value, offset, *addr, oldAddr);
					//set *addr
					break;
				case R_386_PC32:
					s = &((ELF32Sym *)symb.data)[ELF32RSYM(rels[i].r_info)];
					sectionIndex = s->st_shndx;
					oldAddr = *addr;
					*addr += s->st_value - rels[i].r_offset;
					if (sectionIndex == SHN_UNDEF)
						*addr -= offset; //hack because we get the imports as absolute values and the static relocs have relative values X(X((X
					dbg_log("$ %s %lx %lx %lx %lx oldaddr %lx", (char *)&names.data[s->st_name], s->st_value, offset, rels[i].r_offset, *addr, oldAddr);
					//set *addr
					break;
				case R_386_RELATIVE :
					oldAddr = *addr;
					*addr += offset;
					dbg_log("$ 0x%08lx => 0x%08lx", oldAddr, *addr);
					break;
				case R_386_GLOB_DAT :
					// TODO : Probably bug source
					s = &((ELF32Sym *)symb.data)[ELF32RSYM(rels[i].r_info)];
					oldAddr = *addr;
					*addr = s->st_value;
					dbg_log("$ 0x%08lx => 0x%08lx; %s", oldAddr, *addr, (char *)&names.data[s->st_name]);
					//set *addr
					break;
				case R_386_JMP_SLOT :
					// TODO : Probably bug source
					s = &((ELF32Sym *)symb.data)[ELF32RSYM(rels[i].r_info)];
					oldAddr = *addr;
					*addr = s->st_value;
					dbg_log("$ 0x%08lx => 0x%08lx; %s", oldAddr, *addr, (char *)&names.data[s->st_name]);
					//set *addr
					break;
			};

			dbg_log("\n");
		}
		return true;
	}

	bool FloatingELF32::Relocate(DWORD newBase) {
		DWORD offset = newBase - moduleBase;

		for (auto i = sections.begin(); i != sections.end(); ++i) {
			if (SHT_REL == i->header.sh_type) {
				if (i->header.sh_addr == rd[REL_INDEX].address) {
					DWORD symbols = i->header.sh_link;
					DWORD symNames = sections[symbols].header.sh_link;
					RelocateSection((ELF32Rel *)rd[REL_INDEX].rValue,
							rd[REL_INDEX].size / relEnt,
							sections[symbols], sections[symNames], offset);
				}

				if (i->header.sh_addr == rd[PLTREL_INDEX].address) {
					DWORD symbols = i->header.sh_link;
					DWORD symNames = sections[symbols].header.sh_link;
					RelocateSection((ELF32Rel *)rd[PLTREL_INDEX].rValue,
							rd[PLTREL_INDEX].size / relEnt,
							sections[symbols], sections[symNames], offset);
				}
			}
		}

		// Do the same for rela sections

		for (auto i = sections.begin(); i != sections.end(); ++i) {
			if (SHF_ALLOC & i->header.sh_flags) {
				i->header.sh_addr += offset;
			}
		}

		for (auto i = pHeaders.begin(); i != pHeaders.end(); ++i) {
			if (PT_LOAD == i->header.p_type) {
				i->header.p_vaddr += offset;
			}
		}

		moduleBase += offset;
		return true;
	}

	void FloatingELF32::MapSections(AbstractMapper &mapr, DWORD startSeg, DWORD stopSeg) {
		for (auto i = sections.begin(); i != sections.end(); ++i) {
			if (SHF_ALLOC & i->header.sh_flags) {
				DWORD cpStart = i->header.sh_addr;
				if (cpStart < startSeg) {
					cpStart = startSeg;
				}

				DWORD cpStop = i->header.sh_addr + i->header.sh_size;
				if (cpStop > stopSeg) {
					cpStop = stopSeg;
				}

				if (cpStart < cpStop) { // we need to do some copying
					if (SHT_NOBITS == i->header.sh_type) {
						unsigned char *buff = new unsigned char[cpStop - cpStart];
						memset(buff, 0, cpStop - cpStart);
						mapr.WriteBytes((void *)(cpStart), buff, cpStop - cpStart);
						delete buff;
					}
					else {
						mapr.WriteBytes((void *)(cpStart), &i->data[cpStart - i->header.sh_addr], cpStop - cpStart);
					}
					dbg_log("%s[%lu:%lu] ", &sections[header.e_shstrndx].data[i->header.sh_name], cpStart - i->header.sh_addr, cpStop - i->header.sh_addr);
				}
			}
		}

		dbg_log("\n");
	}

	void FloatingELF32::ForAllExports(std::function<void(const char *, const DWORD, const char *, const DWORD, const unsigned char *)> verb) const {
		for (auto i = sections.begin(); i != sections.end(); ++i) {
			if (SHT_DYNSYM == i->header.sh_type) {
				ELF32Sym *symb = (ELF32Sym *)i->data;
				DWORD count = i->header.sh_size / sizeof(*symb);
				for (DWORD j = 0; j < count; ++j) {
					if ((STB_GLOBAL == ELF32STBIND(symb[j].st_info)) && (SHN_UNDEF != symb[j].st_shndx)) {
						if (nullptr != i->versions && nullptr != gnu_versions_d) {
							ELFSymbolVersionDefined *vers = gnu_versions_d->idxSdVers[((WORD *)i->versions->data)[j]];

							if (0 != symb[j].st_value) {
								if (nullptr != vers) {
									verb(
											(const char *)&sections[i->header.sh_link].data[symb[j].st_name],
											0,
											vers->version.c_str(),
											symb[j].st_value,
											(const unsigned char *)RVA(symb[j].st_value)
										);
								}
								else {
									verb(
											(const char *)&sections[i->header.sh_link].data[symb[j].st_name],
											0,
											"",
											symb[j].st_value,
											(const unsigned char *)RVA(symb[j].st_value)
										);
								}
							}
						}
					}
				}
			}
		}
	}



	bool FloatingELF32::ParseVerNeed(ELFSection &s) {
		DWORD maxVer = 0;
		s.snVers.clear();
		for (unsigned char *ptr = s.data; ptr < s.data + s.header.sh_size; ) {
			ELF32VerNeed *vn = (ELF32VerNeed *)ptr;
			ptr += sizeof(*vn);

			dbg_log("%s with %d entries\n", &sections[s.header.sh_link].data[vn->vn_file], vn->vn_cnt);

			for (DWORD j = 0; j < vn->vn_cnt; ++j) {
				ELF32VerNeedAux *va = (ELF32VerNeedAux *)ptr;
				ptr += sizeof(*va);

				dbg_log("\t %s -> %d\n", &sections[s.header.sh_link].data[va->vna_name], va->vna_other);
				if (maxVer < va->vna_other) {
					maxVer = va->vna_other;
				}
				s.snVers.push_back(ELFSymbolVersionNeeded(va->vna_other, (char *)&sections[s.header.sh_link].data[va->vna_name], (char *)&sections[s.header.sh_link].data[vn->vn_file]));
			}
		}

		s.idxSnVers.resize(maxVer + 1);
		for (DWORD i = 0; i <= maxVer; ++i) {
			s.idxSnVers[i] = nullptr;
		}

		for (auto i = s.snVers.begin(); i != s.snVers.end(); ++i) {
			s.idxSnVers[i->index] = &(*i);
		}
		return true;
	}

	bool FloatingELF32::ParseVerDef(ELFSection &s) {
		DWORD maxVer = 0;
		s.snVers.clear();
		for (unsigned char *ptr = s.data; ptr < s.data + s.header.sh_size; ) {
			ELF32VerDef *vd = (ELF32VerDef *)ptr;
			ptr += sizeof(*vd);

			dbg_log("%d with %d entries [%s %s]\n", 
				vd->vd_ndx, 
				vd->vd_cnt,
				(vd->vd_flags & VER_FLG_BASE) ? "VER_FLG_BASE" : "",
				(vd->vd_flags & VER_FLG_WEAK) ? "VER_FLG_WEAK" : ""
			);

			if (maxVer < vd->vd_ndx) {
				maxVer = vd->vd_ndx;
			}

			for (DWORD j = 0; j < vd->vd_cnt; ++j) {
				ELF32VerDefAux *va = (ELF32VerDefAux *)ptr;
				ptr += sizeof(*va);

				// I think that VER_FLG_BASE marks an unversioned symbol
				if (0 == (vd->vd_flags & VER_FLG_BASE)) {
					if (0 == j) {
						dbg_log("\t %s\n", &sections[s.header.sh_link].data[va->vda_name]);
						s.sdVers.push_back(ELFSymbolVersionDefined(vd->vd_ndx, (char *)&sections[s.header.sh_link].data[va->vda_name]));
					}
				}
 			}
 		}

		s.idxSdVers.resize(maxVer + 1);
 		for (DWORD i = 0; i <= maxVer; ++i) {
			s.idxSdVers[i] = nullptr;
 		}


		for (auto i = s.sdVers.begin(); i != s.sdVers.end(); ++i) {
			s.idxSdVers[i->index] = &(*i);
 		}
 		return true;
 	}


	bool FloatingELF32::ParseDynamic(const ELFSection &s) {
		ELF32Dyn *dyns = (ELF32Dyn *)s.data;
		DWORD cnt = s.header.sh_size / sizeof(dyns[0]);

		for (DWORD j = 0; j < cnt; ++j) {
			DWORD value = dyns[j].d_un;
			DWORD *rValue = nullptr;
			switch (dyns[j].d_tag) {
				case DT_INIT :
					start = dyns[j].d_un;
					break;
				case DT_INIT_ARRAY:
					init_array = dyns[j].d_un;
					break;
				case DT_INIT_ARRAYSZ:
					init_array_sz = dyns[j].d_un;
					break;
				case DT_PLTGOT :
				case DT_HASH :
				case DT_STRTAB :
				case DT_SYMTAB :
				case DT_RELA :
				case DT_FINI :
				case DT_REL :
				case DT_DEBUG :
				case DT_JMPREL :
					rValue = (DWORD *)RVA(value);
			}

			switch (dyns[j].d_tag) {
				case DT_NEEDED :
					dbg_log("Need library %s\n", &sections[s.header.sh_link].data[value]);
					libraries.push_back((char *)&sections[s.header.sh_link].data[value]);
					break;
				case DT_REL :
					rd[REL_INDEX].rValue = rValue;
					rd[REL_INDEX].address = value;
					break;
				case DT_RELA :
					rela = rValue;
					break;
				case DT_RELSZ :
					rd[REL_INDEX].size = value;
					break;
				case DT_RELASZ :
					relaSz = value;
					break;
				case DT_RELENT :
					relEnt = value;
					break;
				case DT_RELAENT :
					relaEnt = value;
					break;
				case DT_JMPREL:
					rd[PLTREL_INDEX].rValue = rValue;
					rd[PLTREL_INDEX].address = value;
					break;
				case DT_PLTRELSZ:
					rd[PLTREL_INDEX].size = value;
					break;
				case DT_SONAME :
					dbg_log("So name %s\n", &sections[s.header.sh_link].data[value]);
					break;
			}
		}
		
		return true;
	}

	bool FloatingELF32::Map(AbstractMapper &mapr, AbstractImporter &impr, DWORD &baseAddr, bool callConstructors) {
		DWORD oNA = 0;

		for (auto i = pHeaders.begin(); i != pHeaders.end(); ++i) {
			DWORD tNA = 0;
			switch (i->header.p_type) {
				case PT_LOAD :
					tNA = (i->header.p_vaddr + i->header.p_memsz + i->header.p_align - 1) & ~(i->header.p_align - 1);
					break;
			}

			if (tNA > oNA) {
				oNA = tNA;
			}
		}

		dbg_log("Base: 0x%08lx; Size: 0x%08lx\n", moduleBase, oNA - moduleBase);

		baseAddr = (DWORD)mapr.CreateSection((void *)(baseAddr), oNA - moduleBase, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
		if (0 == baseAddr) {
			return false;
		}

		FixImports(impr, baseAddr);

		Relocate(baseAddr);

		static const DWORD prot[] = {
			0, 1, 2, 3, 4, 5, 6, 7
		};

		for (auto i = pHeaders.begin(); i != pHeaders.end(); ++i) {
			DWORD startSegment, stopSegment;
			bool loaded = false;
			switch (i->header.p_type) {
				case PT_LOAD:
					startSegment = i->header.p_vaddr & ~(i->header.p_align - 1);
					stopSegment = (i->header.p_vaddr + i->header.p_memsz + i->header.p_align - 1) & ~(i->header.p_align - 1);
					MapSections(mapr, startSegment, stopSegment);
					loaded = true;
					break;
			}

			if (loaded) {
				void *address = (void*)(i->header.p_vaddr & ~(i->header.p_align - 1));
				mapr.ChangeProtect(address, stopSegment - startSegment, prot[i->header.p_flags]);
			}
		}

		typedef void (*start_handler) (void);
		typedef void (*init_handler) (void);

		if (!callConstructors)
			return true;

		if (start)
			((start_handler) (baseAddr + start)) ();

		// init array
		if (init_array) {
			unsigned int j, jm;
			jm = init_array_sz / 4;

			for (j = 0; j < jm; ++j) {
				(((init_handler*) (baseAddr + init_array))[j]) ();
			}
		}

		return true;
	}

	DWORD FloatingELF32::GetRequiredSize() const {
		DWORD maxAddr = 0x0;
		for (auto i = sections.begin(); i != sections.end(); ++i) {
			if (i->header.sh_addr == 0)
				continue;

			unsigned int alignment = std::max((DWORD)0x1000, i->header.sh_addralign);
			DWORD maxSec = ((i->header.sh_addr + i->header.sh_size) +
					(alignment - 1)) & (~(alignment - 1));
			if (maxSec > maxAddr)
				maxAddr = maxSec;
		}
		return maxAddr;
	}

}; //namespace ldr
