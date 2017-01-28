# @file reference1.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-11-29
# @version 2016-11-29

to_var t_script <<"_END"
a = "a"
b = "{a:upper}"
c = "{b:lower}"
d = "{a}{b}{c}"

@warn "{a}"
@warn "{b}"
@warn "{c}"
@warn "{d}"
@warn "{e}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:6:.*: Warn: a$
^process\\.parallel:7:.*: Warn: A$
^process\\.parallel:8:.*: Warn: a$
^process\\.parallel:9:.*: Warn: aAa$
^process\\.parallel:10:.*: Error: Trying to access unknown variable "e"\\.$
_END

t_exit="1"
