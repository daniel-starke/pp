# @file include7.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-22
# @version 2016-12-22

include7_cleanup() {
	rm -rf "include7.parallel"
	return 0
}

include7_init() {
	include7_cleanup
	cat > "include7.parallel" <<"_END"
@warn "include file"
_END
	return 0
}

to_var t_script <<"_END"
@warn "main file"
@include "include7_invalid.parallel"
@warn "main file"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:1:.*: Warn: main file$
^process\\.parallel:2:.*: Error: Invalid file path "include7_invalid.parallel"\\.$
_END

to_var t_nerror <<"_END"
^include7.parallel:1:.*: Warn: include file$
^process\\.parallel:3:.*: Warn: main file$
_END

t_exit="1"

t_before="include7_init"
if [ "${clean}" -eq 1 ]; then
	t_after="include7_cleanup"
fi
