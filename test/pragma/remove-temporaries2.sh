# @file remove-temporaries2.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-21
# @version 2016-12-21

remove_temporaries2_cleanup() {
	rm -rf remove-temporaries2
	return 0
}

to_var t_create <<"_END"
d remove-temporaries2
f remove-temporaries2/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries

input = "remove-temporaries2/in\\.txt"

process : A { foreach {
	~destination = "{?:directory}/tmp.txt"
	echo temporary > "{destination}"
} }

process : B { foreach {
	destination = "{?:directory}/out.txt"
	echo output > "{destination}"
} }

execution : default { A("{input}") > B }

_END

to_var t_ntext <<"_END"
^deleting "remove-temporaries2/tmp.txt": ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f remove-temporaries2/tmp.txt
f remove-temporaries2/out.txt
_END

t_exit="0"

t_before="remove_temporaries2_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="remove_temporaries2_cleanup"
fi
