# @file include6.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-22
# @version 2016-12-22

include6_cleanup() {
	rm -rf "include6.parallel"
	return 0
}

include6_init() {
	include6_cleanup
	cat > "include6.parallel" <<"_END"
@warn "include file"
_END
	return 0
}

to_var t_script <<"_END"
@warn "main file"
@include "include6.parallel"
@warn "main file"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:1:.*: Warn: main file$
^include6.parallel:1:.*: Warn: include file$
^process\\.parallel:3:.*: Warn: main file$
_END

t_exit="0"

t_before="include6_init"
if [ "${clean}" -eq 1 ]; then
	t_after="include6_cleanup"
fi
