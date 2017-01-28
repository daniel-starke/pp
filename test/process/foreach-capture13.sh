# @file foreach-capture13.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-27
# @version 2016-12-27

foreach_capture13_cleanup() {
	rm -rf foreach-capture13
	return 0
}

to_var t_create <<"_END"
d foreach-capture13
f foreach-capture13/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./foreach-capture13/in\\.txt"
filter = "./(foreach-capture13)/in(\\.txt)"

process : A { foreach "{filter}" {
	destination = "{1}/out{2}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}")
}

_END

to_var t_error <<"_END"
^process\\.parallel:9:.*: Error: Trying to access unknown variable "2"\\.$
_END

to_var t_ncheck <<"_END"
f foreach-capture13/out.txt
_END

t_exit="1"

t_before="foreach_capture13_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_capture13_cleanup"
fi
