# @file variable-checking2.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-08
# @version 2016-12-08

to_var t_script <<"_END"
variable = "content"
nothing = ""
unset nothing

@disable variable-checking

ok = "{variable}"
nok = "{nothing}"

@warn "{ok}"
@warn "{nok}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:8:.*: Warning: Trying to access unknown variable "nothing".$
^process\\.parallel:10:.*: Warn: content$
^process\\.parallel:11:.*: Warn: $
_END

t_exit="0"
