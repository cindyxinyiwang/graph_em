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

def assortativity_coefficients_multiples(graphs):
	dorig = pd.DataFrame()
	for g in graphs:
		kcdf = pd.DataFrame.from_dict(nx.average_neighbor_degree(g).items())
		kcdf['k'] = g.degree().values()
		dorig = pd.concat([dorig, kcdf])    
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


def external_rage(G,netname):
	import subprocess
	import networkx as nx
	from pandas import DataFrame
	from os.path import expanduser

	giant_nodes = sorted(nx.connected_component_subgraphs(G), key=len, reverse=True)

	G = nx.subgraph(G, giant_nodes[0])
	tmp_file = "tmp_{}.txt".format(netname)
	with open(tmp_file, 'w') as tmp:
		for e in G.edges():
			tmp.write(str(int(e[0])+1) + ' ' + str(int(e[1])+1) + '\n')

	if "Users" in expanduser('~').split('/'):
		args = ("/Users/saguinag/Research/rage/Source_RAGE_unix/RAGE", tmp_file)
	else:
		args = ("/home/saguinag/Software/rage/Source_RAGE_unix/RAGE", tmp_file)
	popen = subprocess.Popen(args, stdout=subprocess.PIPE)
	popen.wait()
	output = popen.stdout.read()

    # Results are hardcoded in the exe
	df = DataFrame.from_csv("./Results/UNDIR_RESULTS_tmp_{}.csv".format(netname), header=0, sep=',', index_col=0)
	df = df.drop('ASType', 1)
	return df

def tijana_eval_compute_gcm(G_df):
	import scipy.stats

	l = len(G_df.columns)
	gcm = np.zeros((l, l))
	i = 0
	for column_G in G_df:
		j = 0
		for column_H in G_df:
			gcm[i, j] = scipy.stats.spearmanr(G_df[column_G].tolist(), G_df[column_H].tolist())[0]
			if scipy.isnan(gcm[i, j]):
				gcm[i, j] = 1.0
			j += 1
		i += 1
	return gcm

def tijana_eval_compute_gcd(gcm_g, gcm_h):
	import math

	if len(gcm_h) != len(gcm_g):
		raise "Graphs must be same size"
	s = 0
	for i in range(0, len(gcm_g)):
		for j in range(i, len(gcm_h)):
			s += math.pow((gcm_g[i, j] - gcm_h[i, j]), 2)

	gcd = math.sqrt(s)
	return gcd

def network_properties_plot(orig_graph_list, graph_list_list, graph_names, prop):
	"""
	input orig_graph_list: list of original graph
	input graph_list_list: list of graph lists (right now only support 4 graphs)
							[[[g1_list], [g2], [g3], [g4]], []]
	"""
	gs = gridspec.GridSpec(2, 2)
	ax0 = plt.subplot(gs[0, 0])
	ax1 = plt.subplot(gs[0, 1])
	ax2 = plt.subplot(gs[1, 0])
	ax3 = plt.subplot(gs[1, 1])
	property_dict = {"degree": get_degree_prob_distribution,
		"hops": hop_plot_multiples, "clustering": clustering_coefficients_multiples,
		"assort": assortativity_coefficients_multiples, "kcore": kcore_decomposition_multiples,
		"eigenvec": eigenvector_multiples}

	property_func = property_dict[prop]
	orig_values = property_func(orig_graph_list)

	axes = [ax0, ax1, ax2, ax3]
	base_graph_list, graph_list = graph_list_list[0], graph_list_list[1]
	for i in xrange(4):
		base_graph, graph = base_graph_list[i], graph_list[i]
		syn_base_values = property_func(base_graph)

		syn_values = property_func(graph)
		if prop in ['degree', 'hops']:
			orig_values.mean(axis=1).plot(ax=axes[i], marker='.', color='b')
			syn_base_values.mean(axis=1).plot(ax=axes[i], color='r')
			syn_values.mean(axis=1).plot(ax=axes[i], color='g')
			axes[i].set_title(graph_names[i])
		elif pop == 'clustering':
			gb = orig_values.groupby(["k"])
			gb['cc'].mean().plot()
			axes[i].set_title(graph_names[i])
	plt.show()



def network_properties(org_graph_list, syn_graph_list_base, syn_graph_list):
	gs = gridspec.GridSpec(3,3)
	ax0 = plt.subplot(gs[0, 1])
	ax1 = plt.subplot(gs[1, 0])
	ax2 = plt.subplot(gs[1, 1])

	print "Degree"
	orig_deg = get_degree_prob_distribution(org_graph_list)
	syn_deg_base = get_degree_prob_distribution(syn_graph_list_base)
	syn_deg = get_degree_prob_distribution(syn_graph_list)
	#orig_deg.mean(axis=1).plot(ax=ax0,marker='.', ls="None", markeredgecolor="w", color='b')
	syn_deg_base.mean(axis=1).plot(ax=ax0,alpha=0.5, color='r')
	syn_deg.mean(axis=1).plot(ax=ax0,alpha=0.5, color='g')
	ax0.set_title('Degree distributuion', y=0.9)
	ax0.set_xscale('log')
	ax0.set_yscale('log')

	print "Hops"
	orig_hop = hop_plot_multiples(org_graph_list)
	syn_hop_base = hop_plot_multiples(syn_graph_list_base)
	syn_hop = hop_plot_multiples(syn_graph_list)
	#orig_hop.mean(axis=1).plot(ax=ax1, marker='o', color='b')
	syn_hop_base.mean(axis=1).plot(ax=ax1, color='r')
	syn_hop.mean(axis=1).plot(ax=ax1, color='g')
	ax1.set_title('Hop Plot', y=0)

	print 'Clustering Coef'
	orig_clust_coef = clustering_coefficients_multiples(org_graph_list)
	synth_clust_coef_base = clustering_coefficients_multiples(syn_graph_list_base)
	synth_clust_coef = clustering_coefficients_multiples(syn_graph_list)
	gb = orig_clust_coef.groupby(['k'])
	#gb['cc'].mean().plot(ax=ax2, marker='o', ls="None", markeredgecolor="w", color='b', alpha=0.8)
	#gb['cc'].mean().to_csv('Results/clust_orig_{}.tsv'.format(name),sep='\t'
	gb = synth_clust_coef_base.groupby(['k'])
	gb['cc'].mean().plot(ax=ax2, marker='o', ls="None", markeredgecolor="w", color='r',  alpha=0.8 ) 
	#gb['cc'].mean().to_csv('Results/clust_hrg_{}.tsv'.format(name),sep='\t')
	
	gb = synth_clust_coef.groupby(['k'])
	gb['cc'].mean().plot(ax=ax2, marker='o', ls="None", markeredgecolor="w", color='g',  alpha=0.8 )    
	#gb['cc'].mean().to_csv('Results/clust_hrg_{}.tsv'.format(name),sep='\	
	ax2.set_title('Avg Clustering Coefficient', y=0)
	plt.show()

	"""
	print "assortativity"
	orig_assort = assortativity_coefficients_multiples(org_graph_list)
	synth_assort_base = assortativity_coefficients_multiples(syn_graph_list_base)
	synth_assort = assortativity_coefficients_multiples(syn_graph_list)
	gb = orig_assort.groupby(['k'])
	gb[1].mean().plot(ax=ax3, marker='o', ls="None", markeredgecolor="w", color='b',  alpha=0.8)
	gb = synth_assort_base.groupby(['k'])
	gb[1].mean().plot(ax=ax3, marker='o', ls="None", markeredgecolor="w", color='r',  alpha=0.8)
	gb = synth_assort.groupby(['k'])
	gb[1].mean().plot(ax=ax3, marker='o', ls="None", markeredgecolor="w", color='g',  alpha=0.8)
	ax3.set_title('Assortativity', y=0.9)

	print "kcore decomposition"
	orig_core = kcore_decomposition_multiples(org_graph_list)
	synth_core_base = kcore_decomposition_multiples(syn_graph_list_base)
	synth_core = kcore_decomposition_multiples(syn_graph_list)
	orig_core.plot(ax=ax4, marker='o', ls="None", markeredgecolor="w", color='b',  alpha=0.8 )
	synth_core_base.mean(axis=1).plot(ax=ax4, marker='o', ls="None", markeredgecolor="w", color='r',  alpha=0.8 )
	synth_core.mean(axis=1).plot(ax=ax4, marker='o', ls="None", markeredgecolor="w", color='g',  alpha=0.8 )
	ax4.set_title('K-Core', y=0.9)

	print "eigenvec"
	orig_eigenvec = eigenvector_multiples(org_graph_list)
	synth_eigenvec_base = eigenvector_multiples(syn_graph_list_base)
	synth_eigenvec = eigenvector_multiples(syn_graph_list)
	orig_eigenvec= orig_eigenvec.transpose()
	orig_eigenvec.plot(ax=ax5, marker='o', ls="None", markeredgecolor="w", color='b',  alpha=0.8)
	synth_eigenvec_base= synth_eigenvec_base.transpose()
	synth_eigenvec_base.plot(ax=ax5, marker='o', ls="None", markeredgecolor="w", color='r',  alpha=0.8)
	synth_eigenvec= synth_eigenvec.transpose()
	synth_eigenvec.plot(ax=ax5, marker='o', ls="None", markeredgecolor="w", color='g',  alpha=0.8)
	ax5.set_title('eigenvector', y=0.9)

	print "GCD"
	"""

