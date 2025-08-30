import gurobipy as gp

model = gp.read("esc12.sop.ajn.txt.lp")
reduced_model = model.presolve()
reduced_model.write("esc12.sop.ajn.txt-reduced.lp")