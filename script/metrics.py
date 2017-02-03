"""
A new metrics file to calculate network properties
"""
import pandas as pd
import math
import networkx as nx
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from collections import Counter
from random import sample
import numpy as np

def get_degree_prob_distribution(graph_list):
	"""
	Get degree probability
	input: list of sampled graphs
	output: a dataframe of degree probability distribution numerical values
	"""
	if graph_list is not None:
		dorig = pd.DataFrame()
		for g in graph_list:
			d  = g.degree()
			df = pd.DataFrame.from_dict(d.items())
			gb = df.groupby(by=[1]).count()
			dorig = pd.concat([dorig, gb], axis=1)  # Appends to bottom new DFs
	return dorig

def hop_plot_multiples(graphs):
	def hops(all_succs, start, level=0, debug=False):
		#if debug: print("level:", level)
		succs = all_succs[start] if start in all_succs else []
		#if debug: print("succs:", succs)
		lensuccs = len(succs)
		#if debug: print("lensuccs:", lensuccs)
		#if debug: print()
		if not succs:
			yield level, 0
		else:
			yield level, lensuccs
		for succ in succs:
        # print("succ:", succ)
			for h in hops(all_succs, succ, level + 1):
				yield h

	def get_graph_hops(graph, num_samples):
		c = Counter()
		for i in range(0, num_samples):
			node = sample(graph.nodes(), 1)[0]
			b = nx.bfs_successors(graph, node)
			for l, h in hops(b, node):
				c[l] += h
		hopper = Counter()
		for l in c:
			hopper[l] = float(c[l]) / float(num_samples)
		return hopper

	if graphs is not None:
		m_hops_ar = []
		for g in graphs:
			c = get_graph_hops(g, 20)
			d = dict(c)
			m_hops_ar.append(d.values())

		hops_df = pd.DataFrame(m_hops_ar)

	return hops_df.transpose()

def clustering_coefficients_multiples(graphs):
	if graphs is not None:
		dorig = pd.DataFrame()
		for g in graphs:
			degdf = pd.DataFrame.from_dict(g.degree().items())
			ccldf = pd.DataFrame.from_dict(nx.clustering(g).items())
			dat = np.array([degdf[0], degdf[1], ccldf[1]])
			df = pd.DataFrame(np.transpose(dat))
			df = df.astype(float)
			df.columns = ['v', 'k', 'cc']
			dorig = pd.concat([dorig, df])  # Appends to bottom new DFs
	return dorig

def kcore_decomposition_multiples(graphs):
	dorig = pd.DataFrame()
	for g in graphs:
		g.remove_edges_from(g.selfloop_edges())
		d = nx.core_number(g)
		df = pd.DataFrame.from_dict(d.items())
		df[[0]] = df[[0]].astype(int)
		gb = df.groupby(by=[1])
		dorig = pd.concat([dorig, gb.count()], axis=1)  # Appends to bottom new DFs

	return dorig

def eigenvector_multiples(graphs):
	#
	#  dorig = pd.DataFrame()
	# for g in graphs:
	#     # d = nx.eigenvector_centrality(g)
	#     d = nx.eigenvector_centrality_numpy(g)
	#     df = pd.DataFrame.from_dict(d.items())
	#     gb = df.groupby(by=[1])
	#     dorig = pd.concat([dorig, gb.count()], axis=1)  # Appends to bottom new DFs
	# # print "orig"
	# # print dorig.mean(axis=1)
	eig_cents = [nx.eigenvector_centrality_numpy(g) for g in graphs]  # nodes with eigencentrality
	net_vals = []
	for cntr in eig_cents:
		net_vals.append(sorted(cntr.values(), reverse=True))
	df = pd.DataFrame(net_vals)
    
	return df

def network_properties(org_graph_list, syn_graph_list_base, syn_graph_list):
	gs = gridspec.GridSpec(3,3)
	ax0 = plt.subplot(gs[0, 1])
	ax1 = plt.subplot(gs[1, 0])
	ax2 = plt.subplot(gs[1, 1])

	print "Degree"
	orig_deg = get_degree_prob_distribution(org_graph_list)
	syn_deg_base = get_degree_prob_distribution(syn_graph_list_base)
	syn_deg = get_degree_prob_distribution(syn_graph_list)
	orig_deg.mean(axis=1).plot(ax=ax0,marker='.', ls="None", markeredgecolor="w", color='b')
	syn_deg_base.mean(axis=1).plot(ax=ax0,alpha=0.5, color='r')
	syn_deg.mean(axis=1).plot(ax=ax0,alpha=0.5, color='g')
	ax0.set_title('Degree distributuion', y=0.9)
	ax0.set_xscale('log')
	ax0.set_yscale('log')

	print "Hops"
	orig_hop = hop_plot_multiples(org_graph_list)
	syn_hop_base = hop_plot_multiples(syn_graph_list_base)
	syn_hop = hop_plot_multiples(syn_graph_list)
	orig_hop.mean(axis=1).plot(ax=ax1, marker='o', color='b')
	syn_hop_base.mean(axis=1).plot(ax=ax1, color='r')
	syn_hop.mean(axis=1).plot(ax=ax1, color='g')
	ax1.set_title('Hop Plot', y=0)

	print 'Clustering Coef'
	orig_clust_coef = clustering_coefficients_multiples(org_graph_list)
	synth_clust_coef_base = clustering_coefficients_multiples(syn_graph_list_base)
	synth_clust_coef = clustering_coefficients_multiples(syn_graph_list)
	gb = orig_clust_coef.groupby(['k'])
	gb['cc'].mean().plot(ax=ax2, marker='o', ls="None", markeredgecolor="w", color='b', alpha=0.8)
	#gb['cc'].mean().to_csv('Results/clust_orig_{}.tsv'.format(name),sep='\t'
	gb = synth_clust_coef_base.groupby(['k'])
	gb['cc'].mean().plot(ax=ax2, marker='o', ls="None", markeredgecolor="w", color='r',  alpha=0.8 ) 
	#gb['cc'].mean().to_csv('Results/clust_hrg_{}.tsv'.format(name),sep='\t')
	
	gb = synth_clust_coef.groupby(['k'])
	gb['cc'].mean().plot(ax=ax2, marker='o', ls="None", markeredgecolor="w", color='g',  alpha=0.8 )    
	#gb['cc'].mean().to_csv('Results/clust_hrg_{}.tsv'.format(name),sep='\	
	ax2.set_title('Avg Clustering Coefficient', y=0)
	plt.show()

