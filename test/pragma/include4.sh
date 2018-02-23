# @file include4.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-22
# @version 2016-12-22

include4_cleanup() {
	rm -rf include4
	return 0
}

to_var t_create <<"_END"
d include4
f include4/in.txt
_END

to_var t_script <<"_END"
input = "include4/in\\.txt"

process : A { none {
	destination = "include4/out1.txt"
	echo output - A > {destination}
} }

process : B { foreach {
	destination = "{?:directory}/out2.txt"
	echo output - B > {destination}
} }

process : C { all {
	destination = "{?:directory}/out3.txt"
	echo output - C > {destination}
} }

execution : AE { A("{input}") }
execution : BE { B("{input}") }
execution : CE { C("{input}") }

execution : default {
	@include AE
	@include BE
	@include C
}

_END

to_var t_error <<"_END"
^process\\.parallel:25:.*:$
expected 'valid execution ID' here$
_END

to_var t_ncheck <<"_END"
f include4/out1.txt
f include4/out2.txt
f include4/out3.txt
_END

t_exit="1"

t_before="include4_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="include4_cleanup"
fi
