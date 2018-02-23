# @file foreach-capture7.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-27
# @version 2016-12-27

foreach_capture7_cleanup() {
	rm -rf foreach-capture7
	return 0
}

to_var t_create <<"_END"
d foreach-capture7
f foreach-capture7/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./(foreach-capture7)/in(\\.txt)"

process : A { foreach {
	destination = "{1}/out{3}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}")
}

_END

to_var t_error <<"_END"
^process\\.parallel:8:.*: Error: Trying to access unknown variable "3"\\.$
_END

to_var t_ncheck <<"_END"
f foreach-capture7/out.txt
_END

t_exit="1"

t_before="foreach_capture7_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_capture7_cleanup"
fi
