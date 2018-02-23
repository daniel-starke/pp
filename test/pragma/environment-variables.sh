# @file environment-variables.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-03
# @version 2016-12-03

to_var t_script <<"_END"
@enable environment-variables

OS = "{PP_OS}"
@if OS is "unix"
	@if SHELL is set
		@warn "ok"
	@else
		@warn "nok"
	@end
@elseif OS is "windows"
	@if ComSpec is set or COMSPEC is set
		@warn "ok"
	@else
		@warn "nok"
	@end
@else
	@warn "nok"
@end

@disable environment-variables

@if OS is "unix"
	@if SHELL is not set
		@warn "ok"
	@else
		@warn "nok"
	@end
@elseif OS is "windows"
	@if ComSpec is not set and COMSPEC is not set
		@warn "ok"
	@else
		@warn "nok"
	@end
@else
	@warn "nok"
@end

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:\\(6\\|12\\):.*: Warn: ok$
^process\\.parallel:\\(24\\|30\\):.*: Warn: ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*: Warn: nok$
_END

t_exit="0"
