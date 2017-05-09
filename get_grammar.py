import sys
sys.path.insert(0, 'script/')

import new_em
import numpy as np
import david as da
import networkx as nx
import matplotlib.pyplot as plt

def rule_to_graph(graph_rules, gram):
	nxg = nx.Graph()	# networkx graph
	nonterm_stack = []
	next_node_id = 0
	for rid in graph_rules:
		lhs, rhs_list = gram.by_id[rid][0].lhs, gram.by_id[rid][0].rhs
		lhs_map = {}

		if "_" in lhs:
			lhs = lhs.split("_")
			lhs_letters, split = lhs[0].split(","), lhs[1]
			lhs_nids = nonterm_stack[-1]
			nonterm_stack.pop()
			assert len(lhs_letters) == len(lhs_nids)
			for letter, id in zip(lhs_letters, lhs_nids):
				lhs_map[letter] = id
		rhs_list.reverse() # left-most derivation
		add_node_set = set()
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
					print new_nid
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
	split = 2

	if len(sys.argv)<2:
		train_file = "prepare_tree_rules/cit-HepTh/%d_sub/nonpartition/%d_sample/cit-HepTh_train.txt" % (subgraph_size, subgraph_num)
		print "Using the following set of prod rules:",train_file
	else:
		train_file = sys.argv[1]
	print "~"*80
	print "Using the following set of prod rules:",train_file

	cv_train = new_em.ConvertRule(train_file, tree_count=subgraph_num)
	gram = new_em.Grammar(cv_train.rule_dict, split)
	cur_str_result = []
	em = new_em.EM(gram, cv_train.Tree, cur_str_result)
	em.iterations(use_converge=True)

	train_loglikelihood = em.loglikelihood
	rules = em.gram.get_valid_rules(cv_train)

	g = da.Grammar('S')
	for (id, hrg, prob) in rules:
		lhs, rhs = hrg
		#print lhs, rhs
		g.add_rule(da.RuleSplit(id, lhs, rhs, prob, True))

	#for lhs in g.by_lhs:
	#	print lhs
	#	print [(r.lhs, r.cfg_rhs) for r in g.by_lhs[lhs]]

	g.set_max_size(50)
	graph_rules = g.sample(10)

	for rid in graph_rules:
		print g.by_id[rid][0].lhs, g.by_id[rid][0].rhs


	nxg = rule_to_graph(graph_rules, g)
	nx.draw(nxg)
	plt.show()
