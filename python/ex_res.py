#extract result from result.txt 

import numpy as np
import re

Tstrangers=0
Tuser=0
Tfriends=0
Tserver=0

Times=0

with open("result.txt") as f:
    for line in f:        
        m = re.search(r"user\_id:[0-9]+, item\_id:[0-9]+", line) #user_id:1, item_id:1255
        if m is not None:
			print line
			Times=Times+1
        
        m = re.search(r"Time for stage1 Strangers:([-+]?[0-9]*\.?[0-9]+)", line) #match stranger time cost, a float number
        if m is not None: #match one result
        	print m.group(1)
        	Tstrangers=Tstrangers+float(m.group(1))

        m = re.search(r"Time for stage[0-9]* user:([-+]?[0-9]*\.?[0-9]+)", line)
        if m is not None: #match one result
        	print m.group(1)
        	Tuser=Tuser+float(m.group(1))

        m = re.search(r"Time for stage2 friends:([-+]?[0-9]*\.?[0-9]+)", line) 
        if m is not None: #match one result
        	print m.group(1)
        	Tfriends=Tfriends+float(m.group(1))

        m = re.search(r"Time for stage3 server:([-+]?[0-9]*\.?[0-9]+)", line) 
        if m is not None: #match one result
        	print m.group(1)
        	Tserver=Tserver+float(m.group(1))

        m = re.search(r"EX:([-+]?[0-9]*\.?[0-9]+)", line) #print out non-zero EX
        if m is not None and float(m.group(1))!=0: #match one result
        	print line

print "total time cost:"
print "Times:" +str(Times)
print "Tuser:" +str(Tuser)
print "Tserver:" +str(Tserver)
print "Tfriends:" +str(Tfriends)
print "stranger:" +str(Tstrangers)