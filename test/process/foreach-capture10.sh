# @file foreach-capture10.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-27
# @version 2016-12-28

foreach_capture10_cleanup() {
	rm -rf foreach-capture10
	return 0
}

to_var t_create <<"_END"
d foreach-capture10
f foreach-capture10/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./(?<path1>foreach-capture10)/in(?<ext1>\\.txt)"

process : A { foreach {
	destination = "(?<path2>{path1})/intermediate(?<ext2>{ext1})"
	echo intermediate > "{destination}"
} }

ext2 = ".dat"

process : B { foreach {
	destination = "{path2}/out{ext2}"
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
f foreach-capture10/intermediate.txt
f foreach-capture10/out.txt
_END

t_exit="0"

t_before="foreach_capture10_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_capture10_cleanup"
fi
