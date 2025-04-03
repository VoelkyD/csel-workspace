cmd_/workspace/ex_kmodule/modules.order := {   echo /workspace/ex_kmodule/mymodule.ko; :; } | awk '!x[$$0]++' - > /workspace/ex_kmodule/modules.order
