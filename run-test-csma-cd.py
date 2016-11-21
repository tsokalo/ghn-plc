#!/usr/bin/python

import os
import math
from multiprocessing import Process, Queue
import time

LOCAL_DIR = os.path.dirname(os.path.abspath(__file__))

print(LOCAL_DIR)



def DoWork(nodes, maxCw, uselessarg):
	os.system(LOCAL_DIR + "/waf --run ghn-plc-example --command-template=\"%s " + str(nodes) + " " + str(maxCw) + " 4\"");
	print(LOCAL_DIR + "/waf --run ghn-plc-example --command-template=\"%s " + str(nodes) + " " + str(maxCw) + " 4\"");

def TestAlgorithm():
	nodes = 3;
	procList = []
	while nodes <= 7:
		maxCw = 2;
		while maxCw <= 200:
			p = Process(target=DoWork, args=(nodes, maxCw, 0))
			p.start()
			procList.append(p)				
			time.sleep (1);			
			maxCw+=15
		nodes+=2
	for p in procList:
		p.join()			
	 	 
	print "FINAL"
	
if __name__ == '__main__':
	TestAlgorithm()








