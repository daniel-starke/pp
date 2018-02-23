# @file scope1.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

scope1_cleanup() {
	rm -rf scope1
	return 0
}

to_var t_create <<"_END"
d scope1
f scope1/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./(?<path>scope1)/in(?<ext>\\.txt)"

process : A { foreach {
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
f scope1/out.txt
_END

t_exit="0"

t_before="scope1_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="scope1_cleanup"
fi
