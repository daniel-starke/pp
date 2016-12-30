# @file include3.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-22
# @version 2016-12-22

include3_cleanup() {
	rm -rf include3
	return 0
}

to_var t_create <<"_END"
d include3
f include3/in.txt
_END

to_var t_script <<"_END"
input = "include3/in\\.txt"

process : A { none {
	destination = "include3/out1.txt"
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

process : D {
	@include A
	@include B
	@include D
}

execution : default { D("{input}") }

_END

to_var t_error <<"_END"
^process\\.parallel:21:.*:$
expected 'valid process ID' here$
_END

to_var t_ncheck <<"_END"
f include3/out1.txt
f include3/out2.txt
f include3/out3.txt
_END

t_exit="1"

t_before="include3_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="include3_cleanup"
fi
