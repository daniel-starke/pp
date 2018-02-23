# @file verbosity3.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-13
# @version 2016-12-13

to_var t_script <<"_END"
@warn "ok"
@info "ok"
@debug "ok"
@error "ok"

execution : default {}
_END

t_cmdline="-v DEBUG"

to_var t_error <<"_END"
^process\\.parallel:1:.*: Warn: ok$
^process\\.parallel:3:.*: Debug: ok$
^process\\.parallel:4:.*: Error: ok$
_END

to_var t_text <<"_END"
^process\\.parallel:2:.*: Info: ok$
_END

t_exit="1"
