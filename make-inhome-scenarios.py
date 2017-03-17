import os
import math
from multiprocessing import Process, Queue
import time

LOCAL_DIR = os.path.dirname(os.path.abspath(__file__))

print(LOCAL_DIR)



def DoWork(el_devs, uselessvar):
	os.system(LOCAL_DIR + "/run_scratch.sh \"ghn-plc-in-home-snr " + str(el_devs) + "\"");
	print(LOCAL_DIR + "/run_scratch.sh \"ghn-plc-in-home-snr " + str(el_devs) + "\"");

def TestAlgorithm():

	el_devs = 3;

	while el_devs <= 21:

		nm_iter = 0;
		while nm_iter < 10:

			print("Start new batch")
			n_iter = 0;
			procList = []
			while n_iter < 16:

				p = Process(target=DoWork, args=(el_devs, 0))
				p.start()
				procList.append(p)				
				time.sleep (1);			
				n_iter+=1

#			<-while
			nm_iter+=1

			for p in procList:
				p.join()
	
#		<-while
		el_devs+=2		

#	<-while
	print "FINAL"
	
if __name__ == '__main__':
	TestAlgorithm()







