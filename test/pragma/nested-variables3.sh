# @file nested-variables3.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-19
# @version 2016-12-21

nested_variables3_cleanup() {
	rm -rf nested-variables3
	return 0
}

to_var t_create <<"_END"
d nested-variables3
f nested-variables3/in.txt
_END

to_var t_script <<"_END"
@enable nested-variables

input = "nested-variables3/in\\.txt"

@shell : test {
	path = "shell"
	commandLine = "shell -c {?}"
}
@shell test

test = "input"
process : A { foreach {
	destination = "{?:directory}/out.txt"
	text = "output"
	echo {text} > {destination}
} }

execution : default { A("{input}") }

_END

t_cmdline="-n"

to_var t_text <<"_END"
shell -c echo output > nested-variables3/out\.txt$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

t_exit="0"

t_before="nested_variables3_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="nested_variables3_cleanup"
fi
