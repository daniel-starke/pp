# @file filename.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
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
@warn "{a1:filename}"
@warn "{b1:filename}"
@warn "{c1:filename}"
@warn "{d1:filename}"
@warn "{e1:filename}"
@warn "{f1:filename}"
@warn "{g1:filename}"

@warn "{a2}"
@warn "{a2:native:filename:win}"
@warn "{b2:native:filename:win}"
@warn "{c2:native:filename:win}"
@warn "{d2:native:filename:win}"
@warn "{e2:native:filename:win}"
@warn "{f2:native:filename:win}"
@warn "{g2:native:filename:win}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:17:.*: Warn: /root/parent/child\\.ext$
^process\\.parallel:18:.*: Warn: child\\.ext$
^process\\.parallel:19:.*: Warn: \\.$
^process\\.parallel:20:.*: Warn: parent$
^process\\.parallel:21:.*: Warn: \\.$
^process\\.parallel:22:.*: Warn: root$
^process\\.parallel:23:.*: Warn: /$
^process\\.parallel:24:.*: Warn: $
^process\\.parallel:26:.*: Warn: C:\\\\root\\\\parent\\\\child\\.ext$
^process\\.parallel:27:.*: Warn: child\\.ext$
^process\\.parallel:28:.*: Warn: \\.$
^process\\.parallel:29:.*: Warn: parent$
^process\\.parallel:30:.*: Warn: \\.$
^process\\.parallel:31:.*: Warn: root$
^process\\.parallel:32:.*: Warn: \\(\\\\\\|\\.\\)$
^process\\.parallel:33:.*: Warn: $
_END

t_exit="0"
