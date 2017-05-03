"""
Plot the data from Results
"""

import matplotlib.pyplot as plt
import numpy as np
from collections import OrderedDict

def test_likelihood():
	markers = ['D', 'o', 'v', 's']
	data = [
		('4 train graph(10 smooth rules, 33%)', [-29.02, -13.52, -40.17, -44.1, -41.07, -60.78, -53.98]),
		('8 train graph(9 smooth rules, 30%)', [-35.06, -17.96, -10.51, -14.71, -41.17, -34.72, -47.95]),
		('12 train graph(8 smooth rules, 27%)', [-41.11, -27.99, -35.74, -67.18, -69.21, -128.75, -146.39]),
		('16 train graph(6 smooth rules, 20%)', [-47.40, -42.81, -83.30, -155.40, -158.22, -308.34, -341.42])
	]
	order_data = OrderedDict(data)
	legend = []
	i = 0
	for name, data_list in order_data.items():
		line, = plt.plot([1,2,3,4,5], data_list[:5], label=name, marker=markers[i])
		legend.append(line)
		i += 1
	plt.legend(handles=legend, loc=0)
	plt.xlabel('Split')
	plt.ylabel('Log Likelihood')
	plt.title('Test Log Likelihood for 25 Node Graph Samples')
	plt.xticks(np.arange(1, 6, 1))
	plt.show()
	
	data_50 = [
		('4 train graph(35 smooth rules, 56%)', [-47.20, -9.13, 15.19, 30.24, 21.62, 19.62, 5.76]),
		('8 train graph(24 smooth rules, 39%)', [-70.32, -34.17, -14.49, 0.94, -64.85, -36.11, -20.20]),
		('12 train graph(24 smooth rules, 39%)', [-72.15, -35.92, -14.71, -0.71, 5.37, 11.36, 20.09]),
		('16 train graph(22 smooth rules, 35%)', [-79.6, -75.61, -139.1, -35.01, -172.05, -455.17, -73.92])
	]
	order_data_50 = OrderedDict(data_50)
	legend = []
	i = 0
	for name, data_list in order_data_50.items():
		line, = plt.plot([1,2,3,4,5], data_list[:5], label=name, marker=markers[i])
		legend.append(line)
		i += 1
	plt.legend(handles=legend, loc=0)
	plt.xlabel('Split')
	plt.ylabel('Log Likelihood')
	plt.title('Test Log Likelihood for 50 Node Graph Samples')
	plt.xticks(np.arange(1, 6, 1))
	plt.show()
	
	data_75 = [
		('4 train graph(48 smooth rules, 58%)', [-68.6, -6.56, 28.09, 47.36, 66.08, 80.03, 92.93]),
		('8 train graph(33 smooth rules, 40%)', [-79.51, -28.66, -83.65, -195.17, -439.07, -699.64]),
		('12 train graph(30 smooth rules, 36%)', [-90.35, -44.47, -127.8, -392.52, -254.51, -608.91, -239.48]),
		('16 train graph(22 smooth rules, 35%)', [-95.64, -51.08, -279.95, -411.33, -610.38, -706.08, -919.11])
	]
	order_data_75 = OrderedDict(data_75)
	legend = []
	i = 0
	for name, data_list in order_data_75.items():
		line, = plt.plot([1,2,3,4,5], data_list[:5], label=name, marker=markers[i])
		legend.append(line)
		i += 1
	plt.legend(handles=legend, loc=0)
	plt.xlabel('Split')
	plt.ylabel('Log Likelihood')
	plt.title('Test Log Likelihood for 75 Node Graph Samples')
	plt.xticks(np.arange(1, 6, 1))

	plt.show()

test_likelihood()