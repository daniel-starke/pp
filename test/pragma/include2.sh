# @file include2.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-22
# @version 2016-12-22

include2_cleanup() {
	rm -rf include2
	return 0
}

to_var t_create <<"_END"
d include2
f include2/in.txt
_END

to_var t_script <<"_END"
input = "include2/in\\.txt"

process : A { none {
	destination = "include2/out1.txt"
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
	@include CE
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f include2/out1.txt
f include2/out2.txt
f include2/out3.txt
_END

t_exit="0"

t_before="include2_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="include2_cleanup"
fi
