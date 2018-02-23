# @file include1.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-22
# @version 2016-12-22

include1_cleanup() {
	rm -rf include1
	return 0
}

to_var t_create <<"_END"
d include1
f include1/in.txt
_END

to_var t_script <<"_END"
input = "include1/in\\.txt"

process : A { none {
	destination = "include1/out1.txt"
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
	@include C
}

execution : default { D("{input}") }

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f include1/out1.txt
f include1/out2.txt
f include1/out3.txt
_END

t_exit="0"

t_before="include1_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="include1_cleanup"
fi
