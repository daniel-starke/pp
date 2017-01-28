# @file foreach-parallel1.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-29
# @version 2016-12-29

foreach_parallel1_cleanup() {
	rm -rf foreach-parallel1
	return 0
}

to_var t_create <<"_END"
d foreach-parallel1
f foreach-parallel1/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "foreach-parallel1/in\\.txt"

process : A { foreach {
	destination = "{?:directory}/intermediate{num}.txt"
	echo intermediate{num} > "{destination}"
} }

process : B {
	@include A
}

process : C { foreach {
	destination = "{?:directory}/intermediate4.txt"
	echo intermediate4 > "{destination}"
} }

process : D { foreach {
	destination = "{?:directory}/out.txt"
	echo output > "{destination}"
} }

execution : default {
	A("{input}", num = "1") | A("{input}", num = "2") | B("{input}", num = "3")
	C("{input}") | D("{input}")
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f foreach-parallel1/intermediate1.txt
f foreach-parallel1/intermediate2.txt
f foreach-parallel1/intermediate3.txt
f foreach-parallel1/intermediate4.txt
f foreach-parallel1/out.txt
_END

t_exit="0"

t_before="foreach_parallel1_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_parallel1_cleanup"
fi
