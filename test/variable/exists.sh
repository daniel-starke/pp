# @file exists.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-11-28
# @version 2016-11-28

to_var t_script <<"_END"
a = "."
b = ".."
c = "unknown.file"

@warn "{PP_SCRIPT:exists}"
@warn "{PP_SCRIPT:directory:exists}"
@warn "{a:exists}"
@warn "{b:exists}"
@warn "{c:exists}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:5:.*: Warn: true$
^process\\.parallel:6:.*: Warn: true$
^process\\.parallel:7:.*: Warn: true$
^process\\.parallel:8:.*: Warn: true$
^process\\.parallel:9:.*: Warn: false$
_END

t_exit="0"
