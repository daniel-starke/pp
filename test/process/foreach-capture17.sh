# @file foreach-capture17.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

foreach_capture17_cleanup() {
	rm -rf foreach-capture17
	return 0
}

to_var t_create <<"_END"
d foreach-capture17
f foreach-capture17/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes
@enable full-recursive-match

input = "(?<path>foreach-capture17)/in(?<ext>\\.txt)"

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
f foreach-capture17/intermediate.txt
f foreach-capture17/out.txt
_END

t_exit="0"

t_before="foreach_capture17_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_capture17_cleanup"
fi
