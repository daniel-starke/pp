# @file scope8.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

scope8_cleanup() {
	rm -rf scope8
	return 0
}

to_var t_create <<"_END"
d scope8
f scope8/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./scope8/(?<ext>i)n\\.txt"

ext = ".ext"

process : A { foreach "\\./(?<path>scope8)/(?<ext>in)\\.txt" {
	ext = ".txt"
	destination = "{path}/out{ext}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}", ext = ".dat")
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f scope8/out.txt
_END

t_exit="0"

t_before="scope8_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="scope8_cleanup"
fi
