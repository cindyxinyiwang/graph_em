__version__="0.1.0"
import argparse
import os, sys
from os import listdir
from os.path import isfile, join
import csv
import networkx as nx
import pandas as pd
import numpy	as np
import treedecomps.load_edgelist_from_dataframe as ledf
import treedecomps.graph_utils as gutil
import treedecomps.experiments as exps # graph stats
from	 prepare_tree_rules.xphrg_graph_gen import xphrg_graph_gen
import traceback
from	 collections import defaultdict
import prepare_tree_rules.PHRG as phrg #import phrg_derive_prod_rules_partition
import prepare_tree_rules.graph_sampler as gs
import prepare_tree_rules.tree_decomposition as td


DGB = False
DEBUG = False
# //////////////////////////////////////////////////////////////////////////////
def get_parser ():
		parser = argparse.ArgumentParser(description='Experiemnts Workflow for CIKM')
		parser.add_argument('--orig', required=True, nargs=1,
				help='Filename edgelist graph')
		parser.add_argument('-cate', required=False, nargs=1, help="Subg category")
		parser.add_argument('-w', action='store_true',default=False,required=False,
				help="Gen whole graph")
		parser.add_argument('--version', action='version', version=__version__)
		return parser

def LogInfo(str_arg):
	print "~"*len(str_arg)
	print str_arg
	print "~"*len(str_arg)


def get_25node_subgraphs_inrage(graph,start_size, end_size, step):
	G=graph
	G.remove_edges_from(G.selfloop_edges())
	giant_nodes = max(nx.connected_component_subgraphs(G), key=len)
	G = nx.subgraph(G, giant_nodes)
	num_nodes = G.number_of_nodes()

	gutil.graph_checks(G)

	g_obj_d = defaultdict(list) # dict with list of graph objects
	for nbr_sg in range(start_size, end_size+1, step):
		#g_obj_d[nbr_sg].append(sample_subgraphs_G_n_K(G,25,nbr_sg))
		print "\t",nbr_sg
		g_obj_d[nbr_sg].append(phrg.probabilistic_hrg(G, nbr_sg, 25))

	print np.shape(g_obj_d)
	return g_obj_d




def sample_refrence_graph(G, ofname, scate, K_lst, n):
		if G is None: return

		G.remove_edges_from(G.selfloop_edges())
		giant_nodes = max(nx.connected_component_subgraphs(G), key=len)
		G = nx.subgraph(G, giant_nodes)

		if n is None:
				num_nodes = G.number_of_nodes()
		else:
				num_nodes = n

		gutil.graph_checks(G)

		if DEBUG: print
		if DEBUG: print "--------------------"
		if DEBUG: print "-Tree Decomposition-"
		if DEBUG: print "--------------------"
		prod_rules = {}
		left_deriv_prod_rules = []

		cntr = 1
		subgraph_objs_dict = defaultdict(list)

		for K in K_lst:
				# print "	", K
				# subgraphs 50 to
				left_deriv_file_name = "ProdRules/{}_{}_{}_prodrules.txt".format(K,scate,ofname)
				out_edgelist_fname = "data/{}_{}_{}_el.tsv".format(ofname,scate,K)
				print "	", out_edgelist_fname #  we will write a subgraph to disk w/ this name
				cntr += 1
				gprime_group =[]
				for Gprime in gs.rwr_sample(G, K, n):
						gprime_group.append(Gprime)
						nx.write_edgelist(Gprime, out_edgelist_fname, delimiter='\t')
						exit()
						T = td.quickbb(Gprime)
						root = list(T)[0]
						T = td.make_rooted(T, root)
						T = phrg.binarize(T)
						root = list(T)[0]
						root, children = T
						td.new_visit(T, G, prod_rules, left_deriv_prod_rules)
				subgraph_objs_dict[(scate,K)].append(gprime_group)

				# left_derive_file = open(left_deriv_file_name, 'w')
				# for r in left_deriv_prod_rules:
				# 	left_derive_file.write(r)
				# 	left_derive_file.write('\n')
				# left_derive_file.close()


		return subgraph_objs_dict



def cikm17_graph_stats(gObjs_A, gObjs_B, glists_info_str, gname):
		'''
		gObjs_A = list of graphs (ie. test set)
		gObjs_A = list of graphs (ie. synth set)
		glists_info_str = list info for each list
		'''
		print '... Degree sample KS Test'
		results = exps.SNDegree_CDF_ks_2samp(gObjs_A, gObjs_B, glists_info_str)
		cols = ["{}_D".format("_".join(glists_info_str)),
						"{}_p".format("_".join(glists_info_str))]
		df = pd.DataFrame(results, columns = cols)
		# print df.to_string()
		outfname = 'Results/degree_ks_{}_{}_{}.csv'.format(gname,glists_info_str[0], glists_info_str[1])
		df.to_csv(outfname, index=False)
		if os.path.exists(outfname):
				print 'Saved to disk', outfname

		print '... GCD Test-I'
		results = exps.gcd(gObjs_A, gObjs_B, glists_info_str)
		outfname = 'Results/gcd_{}_{}_{}.csv'.format(gname,glists_info_str[0], glists_info_str[1])
		results.to_csv(outfname, index=False)
		if os.path.exists(outfname):
				print 'Saved to disk', outfname

		print '... Hop-plot Test-I'
		results = exps.hops_CDF_test(gObjs_A, gObjs_B, glists_info_str)
		outfname = 'Results/hopplot_{}_{}_{}.csv'.format(gname,glists_info_str[0], glists_info_str[1])
		results.to_csv(outfname, index=False)
		if os.path.exists(outfname):
				print 'Saved to disk', outfname

		print "Done"

# def main3 (origG):
#		 subgraph_categories = ["trn","tst","hld"]
#		 moa_results_dict = {}
#		 for scat in subgraph_categories:
#			 print ">", scat
#			 subgraphs_grp_dict =sample_refrence_graph(origG, g_name, scat, range(4,9,4),25)
#			 cntr = 0
#			 for k,v in subgraphs_grp_dict.iteritems():
#				 # print k, [g.number_of_nodes() for g in v[0]]
#				 key= "{}_{}".format(k[0],k[1])
#				 moa_results_dict[key] = exps.SNDegree_CDF_ks_2samp(v[0], origG, 'xpHRG')
#		 # end for
#		 mdf = pd.DataFrame()
#		 for k,v in moa_results_dict.items():
#			 cols= ["{}_D".format(k), "{}_p".format(k)]
#			 df = pd.DataFrame(v, columns=cols)
#			 mdf = pd.concat([df, mdf], axis=1)
#		 print mdf.head()
def cikm17_samp_subgs_derive_rules_gen_graph(origG,hstar_V, sbcate, nbr_sg_lst):
		'''
		Args:
		origG	 reference graph as nx graph object
		hstar_V nbr of nodes --generate a synth graph of target hstar_V
		'''
		for nbr_sg in nbr_sg_lst:
				print '	Sampling',nbr_sg,'subgraphs'
				outfname = "ProdRules/{}_{}_{}_prodrule.txt".format(sbcate,origG.name,nbr_sg)
				set_prod_rules = phrg.probabilistic_hrg_deriving_prod_rules(origG,
																left_deriv_file_name=outfname,
																num_samples=nbr_sg, subgraph_size=25)
				if 0: print "	",len(set_prod_rules)
				if os.path.exists(outfname): print "	Saved to disk:", outfname


def sample_input_graph_into_sets(origG, args):
		subgraph_categories = ["trn","tst","hld"]
		cate_subgraph_groups =	{}
		subgraph_nbr_lst = range(10,11,10)
		for scat in subgraph_categories:
			if args['w']:
					print ">", scat,'-w'
					cikm17_samp_subgs_derive_rules_gen_graph(
							origG,
							origG.number_of_nodes(),
							scat,
							subgraph_nbr_lst
							)
			else:
					print ">>", scat
					scat_results_d= sample_refrence_graph(origG, origG.name, scat,subgraph_nbr_lst,25)
					for sg_nbr in subgraph_nbr_lst:
							prodrules= phrg.probabilistic_hrg(origG, sg_nbr, 25)
							outfname = "ProdRules/{}_{}_{}_{}_prodrules.tsv".format(scat, sg_nbr, 25, origG.name)
							with open(outfname, 'wb') as f:
									writer = csv.writer(f,delimiter="\t")
									writer.writerows(prodrules)
							if os.path.exists(outfname): print '	 Saved to disk:', outfname
					for k,v in scat_results_d.iteritems():
						ky= "{}_{}".format(k[0],k[1])
						cate_subgraph_groups[ky] = v[0]
		if 1:
				# print '.. Graphs sampled:', cate_subgraph_groups.keys()
				# print '.. files:', [f for f in listdir("./ProdRules") if isfile(join("./ProdRules", f))]
				# Using these prod rules, generate the same number of synth graphs and compare with the tst set
				trn_files = [f for f in listdir("./ProdRules") if isfile(join("./ProdRules", f)) and ("trn_10" in f)]
				trn_graphs = cate_subgraph_groups['trn_10']
				syn_graphs = xphrg_graph_gen(trn_files, subgraph_nbr_lst[-1], 25)

		if 0:
				# choosing to compare two groups of graphObjs
				# print subgraph_nbr_lst[-1]
				trn_set = [x for x in cate_subgraph_groups.keys() if 'trn' in x][0]
				tst_set = [x for x in cate_subgraph_groups.keys() if 'tst' in x][0]
				print trn_set, tst_set
				cikm17_graph_stats(cate_subgraph_groups[trn_set],
													 cate_subgraph_groups[tst_set],
													 glists_info_str=[trn_set, tst_set],
													 gname=origG.name)

def load_reference_graph(args):
		orig_file = args['orig'][0]
		print orig_file
		datframes = ledf.Pandas_DataFrame_From_Edgelist(args['orig'])
		df = datframes[0]
		g_name = os.path.basename(orig_file).split('.')[-1]
		LogInfo(g_name)
		if df.shape[1] ==3:
			origG = nx.from_pandas_dataframe(df, 'src', 'trg', ['ts'])	# whole graph
		elif df.shape[1]==4:
				origG = nx.from_pandas_dataframe(df, 'src', 'trg', 'wt', ['ts'])
		else:
			origG = nx.from_pandas_dataframe(df, 'src', 'trg')
		origG.name = g_name
		LogInfo("Graph Loaded")
		return origG


def main():
	parser = get_parser()
	args	 = vars(parser.parse_args())
	
	try:
		origG = nx.read_edgelist(args['orig'][0], data=(('wt',int),))
	except Exception, e:
		try:
			origG = nx.read_edgelist(args['orig'][0], comments="%")
		except Exception, e:
			print str(e)

	# sample_input_graph_into_sets(origG, args)
	# write training set 1 sg of 5K nodes
	fname = [x for x in os.path.basename(args['orig'][0]).split('.') if len(x)>3][0]
	print fname
	cate = args['cate'][0]
	subg = sample_refrence_graph(origG, fname, cate, [1], 5000)


if __name__ == '__main__':
	'''
	Assumptions:
	- edgelist (datasets) are in ./data
	- subgraphs can be saved in ./data
	- clique trees will be placed in ./data
	
	- sample_refrence_graph: samples a large graph and saves a subgraph to disk
	- '''

	
	try:
		main()
	except	Exception, e:
		print str(e)
		traceback.print_exc()
		os._exit(1)
	sys.exit(0)
