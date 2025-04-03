cmd_/workspace/ex_drivers/ex5/modules.order := {   echo /workspace/ex_drivers/ex5/mymodule.ko; :; } | awk '!x[$$0]++' - > /workspace/ex_drivers/ex5/modules.order
