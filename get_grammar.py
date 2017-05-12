__version__="0.1.0"
import argparse
import os
import traceback
import sys
sys.path.insert(0, 'script/')

import new_em
import numpy as np
import david as da
import networkx as nx
import matplotlib.pyplot as plt
from treedecomps.load_edgelist_from_dataframe import Pandas_DataFrame_From_Edgelist
# //////////////////////////////////////////////////////////////////////////////
def get_parser ():
    parser = argparse.ArgumentParser(description='Get Grammar')
    parser.add_argument('--orig', required=True, nargs=1,
        help='Reference graph filename')
    parser.add_argument('--pr', required=False, nargs=1,
		help="Prod rules file-path")
    parser.add_argument('--version', action='version', version=__version__)
    return parser

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

def load_reference_graph(args):
    orig_file = args['orig'][0]

    datframes = Pandas_DataFrame_From_Edgelist(args['orig'])
    df = datframes[0]
    g_name = os.path.basename(orig_file).split('.')[-1]
    if df.shape[1] ==3:
      origG = nx.from_pandas_dataframe(df, 'src', 'trg', ['ts'])  # whole graph
    elif df.shape[1]==4:
        origG = nx.from_pandas_dataframe(df, 'src', 'trg', 'wt', ['ts'])
    else:
      origG = nx.from_pandas_dataframe(df, 'src', 'trg')
    origG.name = g_name
    print("  Graph Loaded")
    return origG

if __name__ == "__main__":
	parser = get_parser()
	args = vars(parser.parse_args())

	subgraph_size = 25
	subgraph_num = 500
	split = 2

	if args['pr'] is None:
		print parser.print_help()
		train_file = "prepare_tree_rules/cit-HepTh/%d_sub/nonpartition/%d_sample/cit-HepTh_train.txt" % (subgraph_size, subgraph_num)
	else:
		train_file = args['pr'][0]
	print "~"*80
	print "  Using the following set of prod rules:",train_file
	# print args['orig']
	G = load_reference_graph(args)
	target_nbr_of_nodes = G.number_of_nodes()


	cv_train = new_em.ConvertRule(train_file, tree_count=subgraph_num)
	gram = new_em.Grammar(cv_train.rule_dict, split)
	cur_str_result = []
	em = new_em.EM(gram, cv_train.Tree, cur_str_result)
	em.iterations(use_converge=True)

	train_loglikelihood = em.loglikelihood
	rules = em.gram.get_valid_rules(cv_train)
	print "len(rules)", len(rules)

	g = da.Grammar('S')
	for (id, hrg, prob) in rules:
		lhs, rhs = hrg
		#print lhs, rhs
		g.add_rule(da.RuleSplit(id, lhs, rhs, prob, True))

	#for lhs in g.by_lhs:
	#	print lhs
	#	print [(r.lhs, r.cfg_rhs) for r in g.by_lhs[lhs]]

	g.set_max_size(target_nbr_of_nodes)
	graph_rules = g.sample(target_nbr_of_nodes)

	# for rid in graph_rules:
	# 	print g.by_id[rid][0].lhs, g.by_id[rid][0].rhs


	nxg = rule_to_graph(graph_rules, g)
	print nx.info(nxg)

	# nx.draw(nxg)
	# plt.show()
