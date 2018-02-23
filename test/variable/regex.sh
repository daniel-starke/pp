# @file regex.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-11-28
# @version 2016-11-28

to_var t_script <<"_END"
a = "abcdefghijklmnopqrstuvwxyz"
b = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
c = "a0b1c2d3e4f5_äÖé_日本語"

@warn "{a:$,[h-o]{5},,}"
@warn "{b:$|^..(.*)..$|\1|}"
@warn "{c:$/[^_]*_..._(.*)/\1/}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:5:.*: Warn: abcdefgmnopqrstuvwxyz$
^process\\.parallel:6:.*: Warn: CDEFGHIJKLMNOPQRSTUVWX$
^process\\.parallel:7:.*: Warn: 日本語$
_END

t_exit="0"
