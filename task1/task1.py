from pulp import *
import pandas as pd
import sys
import os

data = pd.read_csv(sys.argv[1], delimiter=';')

t = data["TimeCost"]
d = data["DueTo"]
p = data["Reward"]

numTasks = len(t)
Tasks = range(numTasks)
problem = LpProblem("TheSingleMachineProblem", LpMaximize)
A = LpVariable.dicts("Position", (Tasks, Tasks), cat="Binary")

#the task can be found not more than once
for i in range(numTasks):
    problem += lpSum([A[i][j] for j in range(numTasks)]) <= 1

#the position can hold not more than 1 task    
for j in range(numTasks):
    problem += lpSum([A[i][j] for i in range(numTasks)]) <= 1

#constraint for time
for k in range(numTasks):
    problem += lpSum([lpSum([A[i][j] * t[i] for i in range(numTasks)]) for j in range(k + 1)]) <= lpSum([d[z] * A[z][k] for z in range(numTasks)])

#Add objective function
problem += lpSum([p[i] * lpSum([A[i][j] for j in range(numTasks)]) for i in range(numTasks)])

problem.writeLP("output/SingleMachineProblem.lp")
problem.solve()
print("Status: ", LpStatus[problem.status])

pos = []
for j in range(numTasks):
    for i in range(numTasks):
        if (value(A[i][j]) == 1):
            pos.append(i + 1)

print("Total reward: {} | Order of Tasks: {}".format(value(problem.objective), pos))