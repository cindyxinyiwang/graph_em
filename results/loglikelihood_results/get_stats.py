"""
Calculate stats of a list of numbers
"""
import sys
import numpy as np
import scipy.stats

def mean_confidence_interval(data, confidence=0.95):
	a = 1.0*np.array(data)
	n = len(a)
	m, se = np.mean(a), scipy.stats.sem(a)
	h = se*scipy.stats.t._ppf((1+confidence)/2, n-1)
	return m, m-h, m+h
graphs = ['twws', 'twba', 'ws', 'ba', 'cit', 'as']
splits = 4
out_file = open("result.txt", "w")
for graph in graphs:
	out_file.write("%s\n" % graph)
	for split in xrange(1, splits+1):
		out_file.write("split: %d\n" % split)
		#file_name = sys.argv[1]
		file_name = "%s/%s_split_%d.txt" % (graph, graph, split)
		data = [float(line) for line in open(file_name)]

		out_file.write("average: %s\n" % str(np.mean(data)))
		out_file.write("standard deviation: %s\n" % str(np.std(data)))
		mean, left, right = mean_confidence_interval(data)
		out_file.write("confidence interval: %s %s\n" % (str(left), str(right)))
out_file.close()
