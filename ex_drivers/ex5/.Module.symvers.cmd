cmd_/workspace/ex_drivers/ex5/Module.symvers := sed 's/\.ko$$/\.o/' /workspace/ex_drivers/ex5/modules.order | scripts/mod/modpost    -o /workspace/ex_drivers/ex5/Module.symvers -e -i Module.symvers   -T -
