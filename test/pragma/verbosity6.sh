# @file verbosity6.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-13
# @version 2016-12-13

to_var t_script <<"_END"
@warn "nok"
@info "nok"
@debug "nok"
@error "ok"

execution : default {}
_END

t_cmdline="-v ERROR"

to_var t_error <<"_END"
^process\\.parallel:4:.*: Error: ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*: .*: nok$
_END

to_var t_ntext <<"_END"
^process\\.parallel:.*:.*: .*: nok$
_END

t_exit="1"
