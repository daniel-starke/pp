# @file remove-remains1.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-21
# @version 2016-12-21

remove_remains1_cleanup() {
	rm -rf remove-remains1
	return 0
}

to_var t_create <<"_END"
d remove-remains1
f remove-remains1/in.txt
_END

to_var t_script <<"_END"
@enable environment-variables
@enable remove-remains

input = "remove-remains1/in\\.txt"

process : A { foreach {
	destination = "{?:directory}/out1.txt"
	echo output1 > "{destination}"
} }

process : B { foreach {
	destination = "{?:directory}/out2.txt"
	echo output2 > "{destination}"
} }

execution : default {
	@if TEST_RUN is "1"
		A("{input}")
	@else
		B("{input}")
	@end
}

_END

to_var t_text <<"_END"
^deleting "remove-remains1/out1.txt": ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f remove-remains1/out2.txt
_END

to_var t_ncheck <<"_END"
f remove-remains1/out1.txt
_END

t_runs="2"

t_exit="0"

t_before="remove_remains1_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="remove_remains1_cleanup"
fi
