import os
import math
from multiprocessing import Process, Queue
import time
from random import randint

LOCAL_DIR = os.path.dirname(os.path.abspath(__file__))

print(LOCAL_DIR)

prog_path = "/home/tsokalo/workspace/ns-allinone-3.25/ns-3.25/build/scratch/"
main_prog = prog_path + "ghn-nc-plc-example"
prog_name = "ghn-nc-plc-example_" + str(randint(0,1000000))
curr_prog = prog_path + prog_name
os.system("cp " + main_prog + " " + curr_prog)


def DoWork(n_iter, uselessvar):

	c_iter = 0
	while c_iter < n_iter:
		os.system(LOCAL_DIR + "/run_scratch.sh " + prog_name);
		print(LOCAL_DIR + "/run_scratch.sh " + prog_name);							
		c_iter+=1	

def TestAlgorithm():

	n_iter = 300
	n_proc = 8

	c_proc = 0
	procList = []
	while c_proc < n_proc:

		p = Process(target=DoWork, args=(n_iter / n_proc, 0))
		p.start()
		procList.append(p)				
		time.sleep (1);			
		c_proc+=1


	for p in procList:
		p.join()

	print "FINAL"

	
if __name__ == '__main__':
	TestAlgorithm()







