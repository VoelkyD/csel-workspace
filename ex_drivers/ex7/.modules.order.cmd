cmd_/workspace/ex_drivers/ex7/modules.order := {   echo /workspace/ex_drivers/ex7/mymodule.ko; :; } | awk '!x[$$0]++' - > /workspace/ex_drivers/ex7/modules.order
