# @file file.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-11-28
# @version 2016-12-30

to_var t_script <<"_END"
a1 = "/root/parent/child.ext"
b1 = "/root/parent/"
c1 = "/root/parent"
d1 = "/root/"
e1 = "/root"
f1 = "/"
g1 = ""

a2 = "C:\\root\\parent\\child.ext"
b2 = "C:\\root\\parent\\"
c2 = "C:\\root\\parent"
d2 = "C:\\root\\"
e2 = "C:\\root"
f2 = "C:\\"
g2 = ""

@warn "{a1}"
@warn "{a1:file}"
@warn "{b1:file}"
@warn "{c1:file}"
@warn "{d1:file}"
@warn "{e1:file}"
@warn "{f1:file}"
@warn "{g1:file}"

@warn "{a2}"
@warn "{a2:native:file:win}"
@warn "{b2:native:file:win}"
@warn "{c2:native:file:win}"
@warn "{d2:native:file:win}"
@warn "{e2:native:file:win}"
@warn "{f2:native:file:win}"
@warn "{g2:native:file:win}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:17:.*: Warn: /root/parent/child\\.ext$
^process\\.parallel:18:.*: Warn: child$
^process\\.parallel:19:.*: Warn: \\.$
^process\\.parallel:20:.*: Warn: parent$
^process\\.parallel:21:.*: Warn: \\.$
^process\\.parallel:22:.*: Warn: root$
^process\\.parallel:23:.*: Warn: /$
^process\\.parallel:24:.*: Warn: $
^process\\.parallel:26:.*: Warn: C:\\\\root\\\\parent\\\\child\\.ext$
^process\\.parallel:27:.*: Warn: child$
^process\\.parallel:28:.*: Warn: \\.$
^process\\.parallel:29:.*: Warn: parent$
^process\\.parallel:30:.*: Warn: \\.$
^process\\.parallel:31:.*: Warn: root$
^process\\.parallel:32:.*: Warn: \\(\\\\\\|\\.\\)$
^process\\.parallel:33:.*: Warn: $
_END

t_exit="0"
