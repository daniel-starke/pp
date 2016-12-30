# @file foreach-capture12.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-27
# @version 2016-12-27

foreach_capture12_cleanup() {
	rm -rf foreach-capture12
	return 0
}

to_var t_create <<"_END"
d foreach-capture12
f foreach-capture12/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./foreach-capture12/in\\.txt"
filter = ".*/(?<path>foreach-capture12)/in(?<ext>\\.txt)"

process : A { foreach "{filter}" {
	destination = "{path}/out{ext}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}")
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f foreach-capture12/out.txt
_END

t_exit="0"

t_before="foreach_capture12_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_capture12_cleanup"
fi
