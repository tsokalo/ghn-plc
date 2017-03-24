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

os.system(LOCAL_DIR + "/waf build");

lib="LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/tsokalo/workspace/coin-Clp/lib/:/home/tsokalo/workspace/SimpleNetSim/build/utils/utils/:/home/tsokalo/workspace/SimpleNetSim/build/galois-field/galois-field/:/home/tsokalo/workspace/SimpleNetSim/build/ccack/ccack/:/home/tsokalo/workspace/SimpleNetSim/build/lp-solver/lp-solver/:/home/tsokalo/workspace/SimpleNetSim/build/network/network/:/home/tsokalo/workspace/SimpleNetSim/build/traffic/traffic/:/home/tsokalo/workspace/SimpleNetSim/build/routing-rules/routing-rules/:/home/tsokalo/workspace/new-kodo-rlnc/kodo-rlnc/kodo_build:./build"

exec_command=lib + " " + prog_path + prog_name

def DoWork(n_iter, uselessvar):

	c_iter = 0
	while c_iter < n_iter:
		os.system(exec_command);
		print(exec_command);							
		c_iter+=1	

def TestAlgorithm():

	n_iter = 2000
	n_proc = 1

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







