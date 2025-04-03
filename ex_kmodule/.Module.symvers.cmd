cmd_/workspace/ex_kmodule/Module.symvers := sed 's/\.ko$$/\.o/' /workspace/ex_kmodule/modules.order | scripts/mod/modpost    -o /workspace/ex_kmodule/Module.symvers -e -i Module.symvers   -T -
