import sys
sys.path.insert(0, 'script/')

import re
import new_em
import numpy as np
import david as da
import networkx as nx
import os, errno

def ensure_dir(dirname):
	"""
	Ensure that a named directory exists; if it does not, attempt to create it.
	"""
	try:
		os.makedirs(dirname)
	except OSError, e:
		if e.errno != errno.EEXIST:
			raise

def rule_to_graph(graph_rules, gram):
	nxg = nx.Graph()	# networkx graph
	nonterm_stack = []
	next_node_id = 0
	for rid in graph_rules:
		lhs, rhs_list = gram.by_id[rid][0].lhs, gram.by_id[rid][0].rhs 
		lhs_map = {}
		#print lhs, rhs_list
		if "_" in lhs:
			lhs = lhs.split("_")
			lhs_letters, split = lhs[0].split(","), lhs[1]
			lhs_nids = nonterm_stack[-1]
			for i,t in enumerate(nonterm_stack[:-1]):
				if len(t) == len(lhs_letters):
					lhs_nids = t
					del nonterm_stack[i]
					break
			#print nonterm_stack
			#print lhs_letters, lhs_nids
			assert len(lhs_letters) == len(lhs_nids)
			for letter, id in zip(lhs_letters, lhs_nids):
				lhs_map[letter] = id 
		add_node_set = set()
		rhs_list.reverse()
		for rhs in rhs_list:
			rhs = rhs.split(":")
			r_nodes, sym = rhs[0].split(","), rhs[1]
			r_nid = []
			for rid in r_nodes:
				if rid.isalpha():
					r_nid.append(lhs_map[rid])
				else:
					new_nid = int(rid) + next_node_id
					add_node_set.add(new_nid)
					r_nid.append(new_nid)
			for id in add_node_set:
				nxg.add_node(id)
			if sym == 'T':
				assert len(r_nid) <= 2
				if len(r_nid) == 2:
					nxg.add_edge(r_nid[0], r_nid[1])
			else:
				nonterm_stack.append(r_nid)
		next_node_id += len(add_node_set)
	return nxg

if __name__ == "__main__":
	subgraph_size = 25
	subgraph_num = 500
	gram_dir_name = sys.argv[1]
	graph_sample_max_size = int(sys.argv[2]) 
	graph_sample_num = 50
	split = int(sys.argv[3])

	graph_file_dir = "results_new/sampled_graphs/%s/split_%d/" % (gram_dir_name, split)	
	ensure_dir(graph_file_dir)

	g = da.Grammar('S')
	# parse grammar file
	gram_file = "results_new/%s/split_%d.gram" % (gram_dir_name, split)
	id = 0
	with open(gram_file) as myfile:
		for line in myfile:
			toks = line.split()
			lhs, rhs_str, prob = toks[0], toks[1:-1], float(toks[-1])
			print rhs_str
			rhs_re = re.compile(".*\'(.*)\'.*")
			rhs = []
			for str in rhs_str:
				rhs.append(rhs_re.match(str).group(1))
			g.add_rule(da.RuleSplit(id, lhs, rhs, prob, True))
			id += 1
	#for lhs in g.by_lhs:
	#	print lhs
	#	print [(r.lhs, r.cfg_rhs) for r in g.by_lhs[lhs]]
	
	g.set_max_size(graph_sample_max_size)
	for i in xrange(graph_sample_num):
		graph_rules = g.sample(graph_sample_max_size)
		
		nxg = rule_to_graph(graph_rules, g)
		graph_file = graph_file_dir + "%d.txt" % (i)
		#print graph_file
		nx.write_edgelist(nxg, graph_file)
	
