# @file none-basic.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-27
# @version 2016-12-27

none_basic_cleanup() {
	rm -rf none-basic
	return 0
}

to_var t_create <<"_END"
d none-basic
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

process : A { none {
	destination = "none-basic/out.txt"
	echo output > "{destination}"
} }

execution : default {
	A
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f none-basic/out.txt
_END

t_exit="0"

t_before="none_basic_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="none_basic_cleanup"
fi
