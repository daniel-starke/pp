# @file foreach-capture8.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-27
# @version 2016-12-27

foreach_capture8_cleanup() {
	rm -rf foreach-capture8
	return 0
}

to_var t_create <<"_END"
d foreach-capture8
f foreach-capture8/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./(?<path>foreach-capture8)/in(?<ext>\\.txt)"

process : A { foreach {
	destination = "(?<path>{path})/intermediate(?<ext>{ext})"
	echo intermediate > "{destination}"
} }

noext = ""
unset noext

process : B { foreach {
	destination = "{path}/out{noext}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}") > B
}

_END

to_var t_error <<"_END"
^process\\.parallel:16:.*: Error: Trying to access unknown variable "noext"\\.$
_END

to_var t_ncheck <<"_END"
f foreach-capture8/intermediate.txt
f foreach-capture8/out.txt
_END

t_exit="1"

t_before="foreach_capture8_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_capture8_cleanup"
fi
