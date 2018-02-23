# @file preDefined1.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-07
# @version 2016-12-08

to_var t_script <<"_END"
@if PP_PATH and PP_PATH is file and "{PP_PATH:directory}" is directory
	@warn "ok"
@else
	@warn "nok"
@end

@if PP_VERSION and PP_VERSION is like "[0-9]+\\.[0-9]+\\.[0-9]+.*"
	@warn "ok"
@else
	@warn "nok"
@end

@if PP_OS and PP_OS is "windows" or PP_OS is "unix"
	@warn "ok"
@else
	@warn "nok"
@end

@if PP_TIME and PP_TIME is like "[0-2][0-9]:[0-5][0-9]:[0-5][0-9]"
	@warn "ok"
@else
	@warn "nok"
@end

@if PP_DATE and PP_DATE is like "[0-9][0-9][0-9][0-9]-[0-1][0-9]-[0-3][0-9]"
	@warn "ok"
@else
	@warn "nok"
@end

@if PP_SCRIPT and PP_SCRIPT is file and "{PP_SCRIPT:filename}" is "process.parallel"
	@warn "ok"
@else
	@warn "nok"
@end

@if PP_TARGETS and PP_TARGETS is "default"
	@warn "ok"
@else
	@warn "nok"
@end

@if PP_THREADS and PP_THREADS is like "[1-9][0-9]*"
	@warn "ok"
@else
	@warn "nok"
@end

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:2:.*: Warn: ok$
^process\\.parallel:8:.*: Warn: ok$
^process\\.parallel:14:.*: Warn: ok$
^process\\.parallel:20:.*: Warn: ok$
^process\\.parallel:26:.*: Warn: ok$
^process\\.parallel:32:.*: Warn: ok$
^process\\.parallel:38:.*: Warn: ok$
^process\\.parallel:44:.*: Warn: ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*: Warn: nok$
_END

t_exit="0"
