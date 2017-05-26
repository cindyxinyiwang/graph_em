import networkx as nx
import math
from collections import defaultdict
import sys
import numpy as np
import scipy.stats

def mean_confidence_interval(data, confidence=0.95):
	a = 1.0*np.array(data)
	n = len(a)
	m, se = np.mean(a), scipy.stats.sem(a)
	h = se*scipy.stats.t._ppf((1+confidence)/2, n-1)
	return m, m-h, m+h

def degree_distribution_distance(train_graph, generated_graph_dict):
	def degree_distribution(G):
		degree_sequence=sorted(nx.degree(G).values(), reverse=True)
		dmax=max(degree_sequence)
		degree_count = defaultdict(lambda:0)
		for i in degree_sequence:
			degree_count[i]+=1
		s_k={}
		for k, count in degree_count.items():
			s_k[k] = count / (k+0.0)
		t_g=sum(s_k.values())
		n_k=defaultdict(lambda:0)
		for k, sk in s_k.items():
			n_k[k] = sk/t_g 
		return n_k, dmax
	train_nk, train_dmax = degree_distribution(train_graph)
	result_dict={}	# key is number of split, value is distance data
	for split in generated_graph_dict:
		dist_list = []
		generated_graph_lists=generated_graph_dict[split]
		for g in generated_graph_lists:
			gen_nk, gen_dmax = degree_distribution(g)
			dist=0
			for k in xrange(max(gen_dmax, train_dmax)+1):
				dist+=pow(train_nk[k]-gen_nk[k], 2)
			dist= math.sqrt(dist)/math.sqrt(2)
			dist_list.append(dist)
		result_dict[split]=dist_list
	return result_dict

#graph_names = ['ws-train', 'cit-HepTh', 'as-train-topo']
#gen_train_name_map = {'cit': 'cit-HepTh', 'as': 'as-train-topo', 'ba':'ba-train', 'ws': 'ba-train', 'twba': "ws-train", 'twws': 'ws-train'}
gen_train_name_map = {'cit': 'cit-HepTh', 'as': 'as-train-topo', 'ws': 'ba-train'}
splits = [1,2,3,4]
graph_num = 50
graph_list_dict = {}
properties = {"degree":degree_distribution_distance}

for graph_name in gen_train_name_map.keys():
	graph_list_dict[graph_name] = {}
	for split in splits:
		graph_lists = []
		for i in xrange(graph_num):
			graph_file = "%s/split_%d/%d.txt" % (graph_name, split, i)
			g = nx.read_edgelist(graph_file)
			graph_lists.append(g)
		graph_list_dict[graph_name][split] = graph_lists

for pname, p in properties.items():
	for gen_graph_name, train_graph_name in gen_train_name_map.items():
		train_graph = nx.read_edgelist("../../prepare_tree_rules/data/%s.txt" % train_graph_name,comments="#")
		result=p(train_graph, graph_list_dict[graph_name])
		result_file=open("%s_%s.txt" % (pname, gen_graph_name), 'w')
		for split in result:
			result_file.write("split %d\n"%(split))
			mean, interval_l, interval_r = mean_confidence_interval(result[split])
			result_file.write("mean: %s, int_left: %s, int_right: %s\n" % (str(mean), str(interval_l), str(interval_r)) )
			result_file.write("std: %s\n" % np.std(result[split]))
			for v in result[split]:
				result_file.write("%s\n" % str(v))
		result_file.close()

