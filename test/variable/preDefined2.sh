# @file preDefined2.sh
# @author Daniel Starke
# @copyright Copyright 2017-2018 Daniel Starke
# @date 2017-01-10
# @version 2017-01-10

preDefined2_before() {
	rm -rf preDefined2
	return 0
}

preDefined2_after() {
	local aPattern
	to_var aPattern <<"_END"
^[0-9a-fA-F]\\+
_END
	sub_check_pattern "final output file" "preDefined2/out.txt" 0 "${aPattern}"
	[ "${clean}" -eq 1 ] && rm -rf preDefined2
	return 0
}

to_var t_create <<"_END"
d preDefined2
f preDefined2/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

base = "preDefined2"
input = "{base}/in\\.txt"

process : A { foreach {
	destination = "{?:directory}/out.txt"
	echo {PP_THREAD} > "{destination}"
} }

execution : default {
	A("{input}")
}
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f preDefined2/out.txt
_END

t_exit="0"

t_before="preDefined2_before"
t_after="preDefined2_after"
