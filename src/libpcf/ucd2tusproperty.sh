#!/bin/sh
# @file ucd2tusproperty.sh
# @author Daniel Starke
# @copyright Copyright 2014-2016 Daniel Starke
# @see typeus.h
# @date 2014-11-08
# @version 2014-11-08
# @see http://www.unicode.org/Public/UCD/latest/ucdxml/ucd.all.flat.zip

#cp="0000" age="1.1" na="" JSN="" gc="Cc" ccc="0" dt="none" dm="#" nt="None" nv="NaN" bc="BN" bpt="n" bpb="#" M="N" bmg="" suc="#" slc="#" stc="#" uc="#" lc="#" tc="#" scf="#" cf="#" jt="U" jg="No_Joining_Group" ea="N" lb="CM" sc="Zyyy" scx="Zyyy" Dash="N" WSpace="N" Hyphen="N" QMark="N" Radical="N" Ideo="N" UIdeo="N" IDSB="N" IDST="N" hst="NA" DI="N" ODI="N" Alpha="N" OAlpha="N" Upper="N" OUpper="N" Lower="N" OLower="N" Math="N" OMath="N" Hex="N" AHex="N" NChar="N" VS="N" C="N" C="N" Base="N" Ext="N" Ext="N" Link="N" STerm="N" Ext="N" Term="N" Dia="N" Dep="N" IDS="N" OIDS="N" XIDS="N" IDC="N" OIDC="N" XIDC="N" SD="N" LOE="N" WS="N" Syn="N" GCB="CN" WB="XX" SB="XX" CE="N" Ex="N" QC="Y" QC="Y" QC="Y" QC="Y" NFC="N" NFD="N" NFKC="N" NFKD="N" NFKC="#" CI="N" Cased="N" CWCF="N" CWCM="N" CWKCF="N" CWL="N" CWT="N" CWU="N" CF="#" InSC="Other" InMC="NA" blk="ASCII" isc="" na1="NULL" 

INPUT="ucd.all.flat.xml"

if [ ! -f "${INPUT}" ]; then
	echo "Could not find input file \"${INPUT}\"." >&2
	exit 1
fi

SCRIPT=`cat <<"_END"
function printSeparator(idx)
{
	if (idx == 0) {
		printf("\t")
		return
	}
	if ((idx % 16) == 0) printf(",\n\t")
	else printf(", ")
}

BEGIN {
	idx = 0
	TUST_DIGIT      = lshift(1, 0)
	TUST_NUMERIC    = lshift(1, 1)
	TUST_UPPER      = lshift(1, 2)
	TUST_LOWER      = lshift(1, 3)
	TUST_ALPHA      = lshift(1, 4)
	TUST_MATH       = lshift(1, 5)
	TUST_HEX        = lshift(1, 6)
	TUST_WHITESPACE = lshift(1, 7)
	printf("const uint8_t tus_property[0x10000] = {\n");
}
/<char cp/ {
	cp     = "" # code point
	nt     = "" # numeric type
	nv     = "" # numeric value
	lb     = "" # line break
	Upper  = ""
	Lower  = ""
	OUpper = ""
	OLower = ""
	suc    = "" # single upper case
	slc    = "" # single lower case
	stc    = "" # single title case
	uc     = "" # upper case
	lc     = "" # lower case
	tc     = "" # title case
	Alpha  = ""
	OAlpha = ""
	Math   = ""
	OMath  = ""
	Hex    = ""
	AHex   = ""
	WSpace = ""
	while (match($0, "([a-zA-Z0-9]+=\"[^\"]*\")")) {
		tagValue = substr($0, RSTART, RLENGTH)
		split(tagValue, arr, "=")
		value = substr(arr[2], 2, length(arr[2]) - 2)
		if (arr[1] == "cp")     cp = value
		if (arr[1] == "nt")     nt = value
		if (arr[1] == "nv")     nv = value
		if (arr[1] == "lb")     lb = value
		if (arr[1] == "Upper")  Upper = value
		if (arr[1] == "Lower")  Lower = value
		if (arr[1] == "OUpper") OUpper = value
		if (arr[1] == "OLower") OLower = value
		if (arr[1] == "suc")    suc = value
		if (arr[1] == "slc")    slc = value
		if (arr[1] == "stc")    stc = value
		if (arr[1] == "uc")     uc = value
		if (arr[1] == "lc")     lc = value
		if (arr[1] == "tc")     tc = value
		if (arr[1] == "Alpha")  Alpha = value
		if (arr[1] == "OAlpha") OAlpha = value
		if (arr[1] == "Math")   Math = value
		if (arr[1] == "OMath")  OMath = value
		if (arr[1] == "Hex")    Hex = value
		if (arr[1] == "AHex")   AHex = value
		if (arr[1] == "WSpace") WSpace = value
		$0 = substr($0, RSTART + RLENGTH)
	}
	charVal = strtonum("0x" cp)
	if (charVal <= 0xFFFF) {
		if (idx < charVal) {
			while (idx < charVal) {
				printSeparator(idx)
				printf("0x00")
				idx++
			}
			printSeparator(idx)
		} else {
			printSeparator(idx)
		}
		value = 0
		if (nt == "De"   || nt == "Di")    value = or(value, TUST_DIGIT     )
		if (tolower(nt) != "none")         value = or(value, TUST_NUMERIC   )
		if (Upper == "Y" || OUpper == "Y") value = or(value, TUST_UPPER     )
		if (Lower == "Y" || OLower == "Y") value = or(value, TUST_LOWER     )
		if (Alpha == "Y" || OAlpha == "Y") value = or(value, TUST_ALPHA     )
		if (Math  == "Y" || OMath  == "Y") value = or(value, TUST_MATH      )
		if (Hex   == "Y" || AHex   == "Y") value = or(value, TUST_HEX       )
		if (WSpace == "Y")                 value = or(value, TUST_WHITESPACE)
		printf("0x%02X", value)
		idx++
	} else {
		while (idx <= 0xFFFF) {
			printSeparator(idx)
			printf("0x00")
			idx++
		}
		exit 0
	}
}

END {
	printf("\n};\n")
}
_END`

awk "${SCRIPT}" <"${INPUT}" >tusproperty.i
exit $?
