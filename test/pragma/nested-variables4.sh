# @file nested-variables4.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-19
# @version 2016-12-21

nested_variables4_cleanup() {
	rm -rf nested-variables4
	return 0
}

to_var t_create <<"_END"
d nested-variables4
f nested-variables4/in.txt
_END

to_var t_script <<"_END"
@disable variable-checking
@disable nested-variables

input = "nested-variables4/in\\.txt"

@shell : test {
	path = "shell"
	commandLine = "shell -c {?}"
}
@shell test

text = "input"
process : A { foreach {
	destination = "{?:directory}/out.txt"
	text = "output"
	echo {text} > {destination}
} }

execution : default { A("{input}") }

_END

t_cmdline="-n"

to_var t_text <<"_END"
shell -c text = "output"$
shell -c echo input > nested-variables4/out\\.txt$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

t_exit="0"

t_before="nested_variables4_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="nested_variables4_cleanup"
fi
