# @file remove-temporaries1.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-21
# @version 2016-12-21

remove_temporaries1_cleanup() {
	rm -rf remove-temporaries1
	return 0
}

to_var t_create <<"_END"
d remove-temporaries1
f remove-temporaries1/in.txt
_END

to_var t_script <<"_END"
@enable remove-temporaries

input = "remove-temporaries1/in\\.txt"

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

to_var t_text <<"_END"
^deleting "remove-temporaries1/tmp.txt": ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f remove-temporaries1/out.txt
_END

to_var t_ncheck <<"_END"
f remove-temporaries1/tmp.txt
_END

t_exit="0"

t_before="remove_temporaries1_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="remove_temporaries1_cleanup"
fi
