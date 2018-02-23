# @file esc.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-11-28
# @version 2016-11-28

to_var t_script <<"_END"
a = "abc"
b = '"abc"'
c = "C:\\abc.txt"

@warn "{a:esc}"
@warn "{b:esc}"
@warn "{c:esc}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:5:.*: Warn: abc$
^process\\.parallel:6:.*: Warn: \\\\"abc\\\\"$
^process\\.parallel:7:.*: Warn: C:\\\\\\\\abc\\.txt$
_END

t_exit="0"
