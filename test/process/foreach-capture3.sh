# @file foreach-capture3.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-27
# @version 2016-12-27

foreach_capture3_cleanup() {
	rm -rf foreach-capture3
	return 0
}

to_var t_create <<"_END"
d foreach-capture3
f foreach-capture3/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./(?<path>foreach-capture3)/in(?<ext>\\.txt)"

process : A { foreach {
	destination = "(?<path>{path})/intermediate(?<ext>{ext})"
	echo intermediate > "{destination}"
} }

process : B { foreach {
	destination = "{path}/out{ext}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}") > B
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f foreach-capture3/intermediate.txt
f foreach-capture3/out.txt
_END

t_exit="0"

t_before="foreach_capture3_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_capture3_cleanup"
fi
