#!/bin/bash
# @file create-hex-include.sh
# @author Daniel Starke
# @copyright Copyright 2015-2016 Daniel Starke
# @date 2015-12-03
# @version 2015-12-10

INPUT="${1}"
OUTPUT="${2}"
VARNAME="${3}"

export LANG="C"
export LANGUAGE="C"
export LC_ALL="C"

if [ ! -e "${INPUT}" ]; then
	echo "Error: Input file not found \"${INPUT}\"." >&2
	exit 1
fi

if [ ! -d "`dirname ${OUTPUT}`" ]; then
	echo "Error: Output directory not found \"`dirname ${OUTPUT}`\"." >&2
	exit 1
fi

if [ "!${VARNAME}" = "!" ]; then
	echo "Error: Variable name not defined." >&2
	exit 1
fi

if ! touch "${OUTPUT}"; then
	echo "Error: Failed to create output file \"${OUTPUT}\"." >&2
	exit 1
fi

oldIFS="${IFS}"
IFS=""
{
	cat << _FILE_HEAD
/**
 * @file `basename ${OUTPUT}`
 * @author ${pp_author}
 * @copyright Copyright 2015-2016 Daniel Starke
 * @copyright Copyright `date +"%Y"` ${pp_author}
 * @date `date +"%Y-%m-%d"`
 */

#include <boost/cstdint.hpp>


/**
 * Automatically converted license text.
 */
static const boost::uint8_t ${VARNAME}[] = {
_FILE_HEAD
	index=0
	printf "\t"
	while read -n 1 byte
	do
		ubyte=$(printf "%i" "'${byte}")
		# signed to unsigned
		if [ "${ubyte}" -lt 0 ]; then
			let "ubyte=ubyte + 256"
		fi
		# wrong interpreted line-feed correction
		if [ "${ubyte}" = 0 ]; then
			ubyte="10"
		fi
		printf "0x%02X," "${ubyte}"
		let "index++"
		if [ "${index}" -ge 32 ]; then
			printf "\n\t"
			index=0
		else
			printf " "
		fi
	done << _FILE_CONTENT
`cat "${INPUT}"`
_FILE_CONTENT
	printf "0x00\n};\n"
} > "${OUTPUT}"
IFS="${oldIFS}"
