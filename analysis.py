import re
import sys
sys.path.insert(0, 'script/')

from random import random
from bisect import bisect

import new_em
import numpy as np
import networkx as nx
from collections import deque

#import visualize
#import net_metrics as nm
#import net_metrics_final as metrics
#import metrics as new_metrics
import graph_sampler as gs

#import matplotlib.pyplot as plt



def plot_nonterm_stats(nonterm_size_dic):
	nonterm_groups = {}
	for n in nonterm_size_dic.keys():
		base = n.split("_")[0]
		if not base in nonterm_groups:
			nonterm_groups[base] = []
		nonterm_groups[base].append(n)

	for base in nonterm_groups:
		nonterms = nonterm_groups[base]
		for n in nonterms:
			print (n, "mean: ", np.mean(nonterm_size_dic[n]), "std: ", np.std(nonterm_size_dic[n]))
			#print nonterm_size_dic[n]
	"""
	fig = plt.figure()
	ax = fig.add_subplot(111)
	ax.set_xlabel('size of subtree for N2')
	ax.set_ylabel('count')
	ax.hist(nonterm_size_dic["N5_1"], bins=[i for i in xrange(100)], alpha=0.5)
	ax.hist(nonterm_size_dic["N5_2"], bins=[i for i in xrange(100)], alpha=0.5)
	ax.hist(nonterm_size_dic["N5_3"], bins=[i for i in xrange(100)], alpha=0.5)
	"""

def choice(values, p):
	total = 0
	cum_weights = []
	for w in p:
		total += w
		cum_weights.append(total)
	x = random() * total
	i = bisect(cum_weights, x)
	return values[i]

def grow_graph_with_root(grammar_dict, root_symbol, out_file):
	graph_rules = []
	graph_file = open(out_file, "w")

	queue = deque([root_symbol])
	while queue:
		lhs = queue.popleft()
		rhs = choice(grammar_dict[lhs].keys(), grammar_dict[lhs].values())
		#print (rhs)
		for t in rhs[:-1]:
			if not t.endswith("T"):
				toks = t.split("_")
				queue.append("N" + str(len(toks[0].split(","))) + "_" + toks[1])
		graph_rules.append((lhs, rhs, grammar_dict[lhs][rhs]))

	k = 0
	for lhs, rhs, prob in graph_rules:
		if k == 0:
			new_lhs = lhs
			graph_file.write("(%s) -> " % (new_lhs))
			k += 1
		else:
			node_count = int(lhs.split("_")[0][1:])
			nonterm_idx = lhs.split("_")[1]
			new_lhs = []
			for i in range(node_count):
				new_lhs.append(chr(ord('a') + i))
			new_lhs = ",".join(new_lhs)
			graph_file.write("(%s)_%s -> " % (new_lhs, nonterm_idx))
		for t in rhs[:-1]:
			graph_file.write("(%s)" % t)
			#if t.endswith("T"):
			#	graph_file.write("(%s)" % t)
			#else:
			#	t = t.split(":")
			#	graph_file.write("(%s:%s)" % (t[0], "N"))
		graph_file.write(" " + rhs[-1])
		graph_file.write("\n")

	graph_file.close()

def grow_nonterminal_graphs(grammar, out_dir):
	# convert grammar format
	grammar_dict = {}
	nonterms = set()
	versions = set()
	for id, rules, prob in grammar:
		lhs, rhs = rules[0], rules[1]
		rhs.append(id)
		if lhs == '(S)':
			new_lhs = 'S'
		else:
			toks = lhs.split("_")
			n_type, v_type = str(len(toks[0].split(","))), toks[1]
			new_lhs = "N" + n_type + "_" + v_type
			nonterms.add(n_type)
			versions.add(v_type)
		if new_lhs not in grammar_dict:
			grammar_dict[new_lhs] = {}
		grammar_dict[new_lhs][tuple(rhs)] = prob
	for i in range(1, len(versions) + 1):
		for j in range(1, len(nonterms) + 1):
			grow_graph_with_root(grammar_dict, 'N%d_%d' % (j, i), '%s/N%d_%d.txt' % (out_dir, j, i))

def grow_graph_to_file(grammar_dict):
	graph_rules = []
	graph_file = open("graph.txt", "w")

	queue = deque("S")
	while queue:
		lhs = queue.popleft()
		rhs = choice(grammar_dict[lhs].keys(), grammar_dict[lhs].values())
		for t in rhs[:-1]:
			if not t.endswith("T"):
				toks = t.split("_")
				queue.append("N" + str(len(toks[0].split(","))) + "_" + toks[1])
		graph_rules.append((lhs, rhs, grammar_dict[lhs][rhs]))
	#print graph_rules
	for lhs, rhs, prob in graph_rules:
		if lhs == 'S':
			new_lhs = lhs
			graph_file.write("(%s) -> " % (new_lhs))
		else:
			node_count = int(lhs.split("_")[0][1:])
			nonterm_idx = lhs.split("_")[1]
			new_lhs = []
			for i in range(node_count):
				new_lhs.append(chr(ord('a') + i))
			new_lhs = ",".join(new_lhs)
			graph_file.write("(%s)_%s -> " % (new_lhs, nonterm_idx))
		for t in rhs[:-1]:
			graph_file.write("(%s)" % t)
			#if t.endswith("T"):
			#	graph_file.write("(%s)" % t)
			#else:
			#	t = t.split(":")
			#	graph_file.write("(%s:%s)" % (t[0], "N"))
		graph_file.write(" " + rhs[-1])
		graph_file.write("\n")

	graph_file.close()

def grow_graph(grammar_dict):
	grow_graph_to_file(grammar_dict)
	g = visualize.Graph()
	g.build_graph("graph.txt")
	return g.graph


def sample_stats(grammar):
	# sample graph size
	graph_size_counts = []
	rules = []
	
	nonterm_ave_size_dic = {}
	nonterm_size_dic = {}
	for i in xrange(1000):
		term_count_dict = {}
		nonterm_count_dict = {}
		rules, term_count, tree = grammar.sample()
		tree.get_term_count(term_count_dict, nonterm_count_dict, nonterm_size_dic)
		for t in term_count_dict:
			if t not in nonterm_ave_size_dic:
				nonterm_ave_size_dic[t] = []
			nonterm_ave_size_dic[t].append(term_count_dict[t] / float(nonterm_count_dict[t]))
		graph_size_counts.append(term_count)

	plot_nonterm_stats(nonterm_ave_size_dic)
	#plot_nonterm_stats(nonterm_size_dic)

	graph_size_counts.sort()
	print "graph size mean: ", np.mean(graph_size_counts), "graph size std: ", np.std(graph_size_counts)

  	fig = plt.figure()
	ax = fig.add_subplot(111)
	ax.set_xlabel('size of tree')
	ax.set_ylabel('count')
	ax.hist(graph_size_counts, bins=[i for i in xrange(100)])


def get_sample_graphs(grammar):
	"""
	input: grammar object
	output: list of sampled networkx graphs
	"""
	grammar_dict = {}
	for id, rules, prob in grammar:
		lhs, rhs = rules[0], rules[1]
		rhs.append(id)
		if lhs == '(S)':
			new_lhs = 'S'
		else:
			toks = lhs.split("_")
			new_lhs = "N" + str(len(toks[0].split(","))) + "_" + toks[1]
		if new_lhs not in grammar_dict:
			grammar_dict[new_lhs] = {}
		grammar_dict[new_lhs][tuple(rhs)] = prob	
	Gstar = []
	Dstar = []
	Gstargl = []
	Ggl = []
	for run in xrange(20):
		nG = grow_graph(grammar_dict)
		Gstar.append(nG)
	return Gstar


def train_test(train_file, test_file, smooth=True, use_converge=True, converge=1, split=1, train_iterations=20):
	"""
	extract and train a grammar on training file, get log likelihood on test file
	train_file: training tree left derivation
	test_file: test tree left derivation
	smooth: whether to add smoothing on grammar
	split: number of split for the grammar
	train_iterations: number of training iterations
	"""
	cv_train = new_em.ConvertRule(train_file)
	cv_test = new_em.ConvertRule(test_file)

	gram = new_em.Grammar(cv_train.rule_dict, split)

	em = new_em.EM(gram, cv_train.Tree)
	em.iterations(use_converge, converge=converge)

	# smooth grammar probabilities
	epsilon = float("1e-323")
	added_nonterms = set()
	test_gram_rules = new_em.Grammar(cv_test.rule_dict, split).rule_dict
	train_gram_rules = em.gram.rule_dict

	if smooth:
		#find mininum probability and use that to set epsilon
		#min_prob = float("inf")
		#for lhs in train_gram_rules:
		#	for rhs, prob in train_gram_rules[lhs].items():
		#		if prob < min_prob and prob > 0:
		#			min_prob = prob 
		#if min_prob < epsilon:
		#	epsilon = min_prob
		print epsilon
		
		added_gram_count = 0
		original_gram_count = sum(map(len, train_gram_rules.values()))
		for lhs in test_gram_rules:
			if lhs not in train_gram_rules:
				train_gram_rules[lhs] = {}
				added_nonterms.add(lhs)
			for rhs, prob in test_gram_rules[lhs].items():
				if rhs not in train_gram_rules[lhs]:
					train_gram_rules[lhs][rhs] = epsilon
					added_gram_count += 1
					added_nonterms.add(rhs)
			# renormalize
			total_prob = sum(train_gram_rules[lhs].values())
			for rhs, prob in train_gram_rules[lhs].items():
				train_gram_rules[lhs][rhs] = prob / total_prob
		em.gram.rule_dict = train_gram_rules
		em.gram.alphabet = em.gram.alphabet.union(added_nonterms)

		print "original grammar count: ", original_gram_count
		print "added grammar count: ", added_gram_count
	# get test likelihood
	em_test = new_em.EM(em.gram, cv_test.Tree)
	em_test.get_loglikelihood()

	return em_test.loglikelihood / len(em_test.tree)


if __name__ == "__main__":
	subgraph_size_list = [100]
	train_sample_size_list = [4]
	for subgraph_size in subgraph_size_list:
		for train_sample_size in train_sample_size_list:
			train_file = "prepare_tree_rules/routers/%d_sub/nonpartition/%d_sample/routers_train.txt" % (subgraph_size, train_sample_size)
			test_file = "prepare_tree_rules/routers/%d_sub/nonpartition/%d_sample/routers_hold.txt" % (subgraph_size, 4)
			print "subgraph size: %d, train sample size: %d" % (subgraph_size, train_sample_size)
			for split in xrange(1, 8, 1):
				print
				print "split: ", split
				for i in xrange(5):
					test_loglikelihood = train_test(train_file, test_file, 
						smooth=True, use_converge=True, converge=1, split=split)
					print "test loglikelihood:", test_loglikelihood
	"""
	#G = nx.hypercube_graph(9)
	G = nx.read_edgelist("prepare_tree_rules/data/rounters.txt", comments="#")
	#G.remove_edges_from(G.selfloop_edges())
	#G = nx.karate_club_graph()
	G.remove_edges_from(G.selfloop_edges())

	cv = new_em.ConvertRule("prepare_tree_rules/routers_left_derive_big.txt")

	gram = new_em.Grammar(cv.rule_dict, 2)

	em = new_em.EM(gram, cv.Tree)
	em.iterations(20)

	grammar = em.gram.get_valid_rules(cv)
	for r in grammar:
		print r
	#grow_nonterminal_graphs(grammar, "out_graphs")
	#visualize.dir_node_count("out_graphs")
	#plt.show()
	#sample_stats(em.gram)
	#get_sample_graphs(grammar)


	Gstar2 = get_sample_graphs(grammar)


	gram = new_em.Grammar(cv.rule_dict, 1)
	em = new_em.EM(gram, cv.Tree)
	em.iterations(20)
	Gstar1 = get_sample_graphs(em.gram.get_valid_rules(cv))

	new_metrics.network_properties_plot([G], 
		[[Gstar1, Gstar1, Gstar1, Gstar1], [Gstar2, Gstar2, Gstar2, Gstar2]], 
		['Karate', 'Karate', 'Karate', 'Karate'],
		'degree')
	"""



