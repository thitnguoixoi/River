#!/usr/bin/env bash

[ $# != 2 ] && { echo "Usage: ./build-all.sh <prefix> <prefix_payload> "; exit 1; }
DIRS="revtracer-wrapper revtracer BinLoader  Execution http-parser-payload libxml-payload"

for d in $DIRS; do
	make clean -C $d
	prefix="$1"
	if [ "$d" == "http-parser-payload" ] || [ "$d" == "libxml-payload" ]; then
		prefix="$2"
	fi
	make install prefix="$prefix" -C $d
done
