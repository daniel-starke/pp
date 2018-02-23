# @file include8.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-22
# @version 2016-12-22

include8_cleanup() {
	rm -rf "include8.parallel"
	return 0
}

include8_init() {
	include8_cleanup
	cat > "include8.parallel" <<"_END"
@warn "include file {a}"
a = "2"
_END
	return 0
}

to_var t_script <<"_END"
@warn "main file"
a = "1"
@include "include8.parallel"
@include "include8.parallel"
@warn "main file"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:1:.*: Warn: main file$
^include8.parallel:1:.*: Warn: include file 1$
^include8.parallel:1:.*: Warn: include file 2$
^process\\.parallel:5:.*: Warn: main file$
_END

t_exit="0"

t_before="include8_init"
if [ "${clean}" -eq 1 ]; then
	t_after="include8_cleanup"
fi
