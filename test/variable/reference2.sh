# @file reference2.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-11-29
# @version 2016-11-29

to_var t_script <<"_END"
unset a, b, c, d
@warn "{a}{b}{c}{d}"

execution : default {}
_END

# unknown variable references are reported in reverse order
to_var t_error <<"_END"
^process\\.parallel:2:.*: Error: Trying to access unknown variable "d"\\.$
_END

t_exit="1"
