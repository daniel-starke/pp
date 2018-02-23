# @file rexists.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-11-28
# @version 2016-11-28

to_var t_script <<"_END"
a = "rexists.*"
b = ".*\.sh"
c = "unknown.file"
d = "../.*\.sh" # relative paths are not supported

@warn "{PP_SCRIPT:unix:rexists}"
@warn "{PP_SCRIPT:unix:directory:rexists}"
@warn "{a:rexists}"
@warn "{b:rexists}"
@warn "{c:rexists}"
@warn "{d:rexists}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:6:.*: Warn: true$
^process\\.parallel:7:.*: Warn: true$
^process\\.parallel:8:.*: Warn: true$
^process\\.parallel:9:.*: Warn: true$
^process\\.parallel:10:.*: Warn: false$
^process\\.parallel:11:.*: Warn: false$
_END

t_exit="0"
