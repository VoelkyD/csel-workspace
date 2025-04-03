cmd_/workspace/ex_drivers/ex2/modules.order := {   echo /workspace/ex_drivers/ex2/mymodule.ko; :; } | awk '!x[$$0]++' - > /workspace/ex_drivers/ex2/modules.order
