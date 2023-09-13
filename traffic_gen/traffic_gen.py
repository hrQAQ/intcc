import sys
import random
import math
import heapq
from optparse import OptionParser
from custom_rand import CustomRand

##################################
# Usage: 
#	python traffic_gen.py -c FbHdp_distribution.txt --ndc 128 --nwan 128 -l 0.2 -b 10G -t 0.1 --pdc 1 --pwan 1 -o hdp_mix_0.2_10G_0.1_1:1.txt
##################################


class Flow:
	def __init__(self, src, dst, size, t):
		self.src, self.dst, self.size, self.t = src, dst, size, t
	def __str__(self):
		return "%d %d 3 100 %d %.9f"%(self.src, self.dst, self.size, self.t)

def translate_bandwidth(b):
	if b == None:
		return None
	if type(b)!=str:
		return None
	if b[-1] == 'G':
		return float(b[:-1])*1e9
	if b[-1] == 'M':
		return float(b[:-1])*1e6
	if b[-1] == 'K':
		return float(b[:-1])*1e3
	return float(b)

def poisson(lam):
	return -math.log(1-random.random())*lam

if __name__ == "__main__":
	parser = OptionParser()
	parser.add_option("-c", "--cdf", dest = "cdf_file", help = "the file of the traffic size cdf", default = "uniform_distribution.txt")
	parser.add_option("--ndc", dest = "ndchost", help = "number of dc hosts")
	parser.add_option("--nwan", dest = "nwanhost", help = "number of wan hosts")
	parser.add_option("-l", "--load", dest = "load", help = "the percentage of the traffic load to the network capacity, by default 0.3", default = "0.3")
	parser.add_option("-b", "--bandwidth", dest = "bandwidth", help = "the bandwidth of host link (G/M/K), by default 10G", default = "10G")
	parser.add_option("--pdc", dest = "proportionOfDc", help = "number of copies of the traffic in data center, by default 1", default = "1")
	parser.add_option("--pwan", dest = "proportionOfWan", help = "number of copies of the traffic in wan, by default 1", default = "1")
	parser.add_option("-t", "--time", dest = "time", help = "the total run time (s), by default 10", default = "10")
	parser.add_option("-o", "--output", dest = "output", help = "the output file", default = "tmp_traffic.txt")
	options,args = parser.parse_args()

	base_t = 2 * 1e9 # 2s

	if not options.ndchost or not options.nwanhost:
		print "please use --ndc to enter number of dc hosts and --nwan to enter number of wan hosts"
		sys.exit(0)
	ndchost = int(options.ndchost)
	nwanhost = int(options.nwanhost)
	nhost = ndchost + nwanhost
	load = float(options.load)
	proportionOfDc = int(options.proportionOfDc)
	proportionOfWan = int(options.proportionOfWan)
	bandwidth = translate_bandwidth(options.bandwidth)
	time = float(options.time)*1e9 # translates to ns
	output = options.output
	if bandwidth == None:
		print "bandwidth format incorrect"
		sys.exit(0)

	fileName = options.cdf_file
	file = open(fileName,"r")
	lines = file.readlines()
	# read the cdf, save in cdf as [[x_i, cdf_i] ...]
	cdf = []
	for line in lines:
		x,y = map(float, line.strip().split(' '))
		cdf.append([x,y])

	# create a custom random generator, which takes a cdf, and generate number according to the cdf
	customRand = CustomRand()
	if not customRand.setCdf(cdf):
		print "Error: Not valid cdf"
		sys.exit(0)

	ofile = open(output, "w")

	# generate flows
	avg = customRand.getAvg()
	avg_inter_arrival = 1/(bandwidth*load/8./avg)*1000000000
	n_flow_estimate = int(time / avg_inter_arrival * nhost)
	n_flow = 0
	ofile.write("%d \n"%n_flow_estimate)
	# flow must src from dc host
	host_list = [(base_t + int(poisson(avg_inter_arrival)), i) for i in range(ndchost)]
	heapq.heapify(host_list)
	idx = 0
	proportionAll = proportionOfDc + proportionOfWan
	dc_host = [i for i in range(nhost) if i < ndchost]
	wan_host = [i for i in range(nhost) if i >= ndchost]
	while len(host_list) > 0:
		t,src = host_list[0]
		inter_t = int(poisson(avg_inter_arrival))
		if (idx % proportionAll < proportionOfDc):
			dst = random.choice(dc_host)
			while (dst == src):
				dst = random.choice(dc_host)
		else:
			dst = random.choice(wan_host)
		idx = (idx + 1) % proportionAll
		if (t + inter_t > time + base_t):
			heapq.heappop(host_list)
		else:
			size = int(customRand.rand())
			if size <= 0:
				size = 1
			n_flow += 1;
			ofile.write("%d %d 3 100 %d %.9f\n"%(src, dst, size, t * 1e-9))
			heapq.heapreplace(host_list, (t + inter_t, src))
	ofile.seek(0)
	ofile.write("%d"%n_flow)
	ofile.close()

'''
	f_list = []
	avg = customRand.getAvg()
	avg_inter_arrival = 1/(bandwidth*load/8./avg)*1000000000
	# print avg_inter_arrival
	for i in range(nhost):
		t = base_t
		while True:
			inter_t = int(poisson(avg_inter_arrival))
			t += inter_t
			dst = random.randint(0, nhost-1)
			while (dst == i):
				dst = random.randint(0, nhost-1)
			if (t > time + base_t):
				break
			size = int(customRand.rand())
			if size <= 0:
				size = 1
			f_list.append(Flow(i, dst, size, t * 1e-9))

	f_list.sort(key = lambda x: x.t)

	print len(f_list)
	for f in f_list:
		print f
'''
