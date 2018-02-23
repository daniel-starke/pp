# @file dependency2.sh
# @author Daniel Starke
# @copyright Copyright 2017-2018 Daniel Starke
# @date 2017-09-16
# @version 2017-09-16

dependency2_before() {
	rm -rf dependency2
	return 0
}

dependency2_after() {
	[ "${clean}" -eq 1 ] && rm -rf dependency2
	return 0
}

to_var t_create <<"_END"
d dependency2
f dependency2/in1.txt
f dependency2/in2.txt
f dependency2/in3.txt
f dependency2/dep.txt
_END

to_var t_script <<"_END"
@enable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "dependency2/in(?<num>[0-9]+)\\.txt"

process : A { foreach {
	destination = "{?:directory}/l1_{num}.txt"
	echo {?} > "{destination}"
} }

process : B {
	foreach ".*l1_(?<num>[0-9]).txt" {
		~destination = "{?:directory}/l2_(?<num>{num}).txt"
		echo {?} > "{destination}"
	}
}

process : C {
	foreach ".*l2_(?<num>[0-9]).txt" {
		destination = "{?:directory}/l3_(?<num>{num}).txt"
		dependency = "dependency2/dep.txt"
		echo {?} > "{destination}"
	}
}

execution : default {
	A("{input}") > B > C
}

_END

t_runs="2"

to_var t_touch1 <<"_END"
f dependency2/dep.txt
_END

to_var t_text <<"_END"
echo dependency2/l1_1.txt
echo dependency2/l1_2.txt
echo dependency2/l1_3.txt
^deleting "dependency2/l2_1\\.txt": ok$
^deleting "dependency2/l2_2\\.txt": ok$
^deleting "dependency2/l2_3\\.txt": ok$
_END

to_var t_ntext <<"_END"
echo dependency1/in1.txt
echo dependency1/in2.txt
echo dependency1/in3.txt
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f dependency2/dep.txt
f dependency2/l1_1.txt
f dependency2/l1_2.txt
f dependency2/l1_3.txt
f dependency2/l3_1.txt
f dependency2/l3_2.txt
f dependency2/l3_3.txt
_END

to_var t_ncheck <<"_END"
f dependency2/l2_1.txt
f dependency2/l2_2.txt
f dependency2/l2_3.txt
_END

t_exit="0"

t_before="dependency2_before"
t_after="dependency2_after"
