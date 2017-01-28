# @file import1.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-22
# @version 2016-12-22

import1_cleanup() {
	rm -rf "import1.parallel"
	return 0
}

import1_init() {
	import1_cleanup
	cat > "import1.parallel" <<"_END"
@warn "include file"
_END
	return 0
}

to_var t_script <<"_END"
@warn "main file"
@import "import1.parallel"
@warn "main file"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:1:.*: Warn: main file$
^import1.parallel:1:.*: Warn: include file$
^process\\.parallel:3:.*: Warn: main file$
_END

t_exit="0"

t_before="import1_init"
if [ "${clean}" -eq 1 ]; then
	t_after="import1_cleanup"
fi
