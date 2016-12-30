# @file all-basic.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-27
# @version 2016-12-27

all_basic_cleanup() {
	rm -rf all-basic
	return 0
}

to_var t_create <<"_END"
d all-basic
f all-basic/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "all-basic/in\\.txt"

process : A { all {
	destination = "{?:directory}/out.txt"
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
f all-basic/out.txt
_END

t_exit="0"

t_before="all_basic_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="all_basic_cleanup"
fi
