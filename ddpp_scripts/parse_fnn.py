#!/usr/bin/env python3
import sys
import os.path

ln = 0
file_path = "FNN/move_distance_finish.fnn"
jumps = 0
actual_jumps = 0
last_jump = 0
hooks = 0
actual_hooks = 0
last_hook = 0
directions = [0,0,0]
targetXs = []
targetYs = []
if ((len(sys.argv) > 1) and (sys.argv[1])):
  file_path = sys.argv[1]
  print("loaded path: " + file_path)
else:
  print("default path: " + file_path)
if not os.path.isfile(file_path):
  print("error loading file.")
  exit(1)

with open(file_path, "r") as f:
  for line in f:
    line = line[0:-1] # chop of newline char
    if ln == 0:
      print("header: " + str(line))
    elif ln == 1:
      print("move ticks: " + str(line))
    elif ln == 2:
      print("distance: " + str(line))
    elif ln == 3:
      print("fitness: " + str(line))
    elif ln == 4:
      print("distance finish: " + str(line))
    else:
      if (ln) % 5 == 0:
        # print("-----------")
        # print("dir: " + str(line))
        di = int(line) + 1
        directions[di] += 1
      elif (ln + 1) % 5 == 0:
        # print("targetY: " + str(line))
        targetYs.append(int(line))
      elif (ln + 2) % 5 == 0:
        # print("targetX: " + str(line))
        targetXs.append(int(line))
      elif (ln + 3) % 5 == 0:
        # print("hook: " + str(line))
        hooks += int(line)
        if last_hook == 0 and int(line) == 1:
          actual_hooks += 1
        last_hook = int(line)
      elif (ln + 4) % 5 == 0:
        # print("jump: " + str(line))
        jumps += int(line)
        if last_jump == 0 and int(line) == 1:
          actual_jumps += 1
        last_jump = int(line)
      # print("line[" + str(ln) + "] " + str(line))
    # print("line[" + str(ln) + "] " + str(line))
    ln += 1

print("=== total imputs ===")
print("jumps: " + str(actual_jumps) + " (ticks " + str(jumps) + ")")
print("hooks: " + str(actual_hooks) + " (ticks " + str(hooks) + ")")
print("avg(targX): " + str(round((sum(targetXs) / float(len(targetXs))), 2)))
print("avg(targY): " + str(round((sum(targetYs) / float(len(targetYs))), 2)))
print("left: " + str(directions[0]) + " stand: " + str(directions[1]) + " right: " + str(directions[2]))
