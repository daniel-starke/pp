# @file foreach-capture6.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-27
# @version 2016-12-27

foreach_capture6_cleanup() {
	rm -rf foreach-capture6
	return 0
}

to_var t_create <<"_END"
d foreach-capture6
f foreach-capture6/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./(?<path>foreach-capture6)/in(?<ext>\\.txt)"

noext = ""
unset noext

process : A { foreach {
	destination = "{path}/out{noext}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}")
}

_END

to_var t_error <<"_END"
^process\\.parallel:11:.*: Error: Trying to access unknown variable "noext"\\.$
_END

to_var t_ncheck <<"_END"
f foreach-capture6/out.txt
_END

t_exit="1"

t_before="foreach_capture6_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_capture6_cleanup"
fi
