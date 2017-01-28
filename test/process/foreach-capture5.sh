# @file foreach-capture5.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-27
# @version 2016-12-27

foreach_capture5_cleanup() {
	rm -rf foreach-capture5
	return 0
}

to_var t_create <<"_END"
d foreach-capture5
f foreach-capture5/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./(?<path1>foreach-capture5)/in(?<ext1>\\.txt)"

process : A { foreach {
	destination = "(?<path2>{path1})/intermediate(?<ext2>{ext1})"
	echo intermediate > "{destination}"
} }

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
f foreach-capture5/intermediate.txt
f foreach-capture5/out.txt
_END

t_exit="0"

t_before="foreach_capture5_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_capture5_cleanup"
fi
