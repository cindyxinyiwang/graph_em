"""
Calculate stats of a list of numbers
"""
import sys
import numpy as np


file_name = sys.argv[1]
data = [float(line) for line in open(file_name)]

print "average: ", np.mean(data)
print "standard deviation: ", np.std(data)
