__version__="0.1.0"
from glob import glob
from script.graph_sampler import rwr_sample
import script.net_metrics_sal as metrics
import networkx as nx
import sys
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

if len(sys.argv) >= 4:
	train_name, gen_name, split = sys.argv[1], sys.argv[2], int(sys.argv[3])

	train_graph_base = "prepare_tree_rules/data/"
	gen_graph_base = "results_new/sampled_graphs/"
	metrics_list = ['gcd']

	g = nx.read_edgelist(train_graph_base+train_name+".txt")
	g_small = next(rwr_sample(g, 1, 5000))
	sampled_graphs = [nx.read_edgelist(f) for f in glob("%s%s/split_%d/*" % (gen_graph_base,gen_name, split) )]
	metrics.network_properties([g_small], metrics_list, sampled_graphs, name="%s_split_%d" % (train_name, split))
else:
	train_name, gen_name = sys.argv[1], sys.argv[2]
	split_list = [1, 2, 3, 4]
	train_graph_base = "prepare_tree_rules/data/"
	gen_graph_base = "results_new/sampled_graphs/"
	metrics_list = ['gcd']

	g = nx.read_edgelist(train_graph_base+train_name+".txt")
	g_small = next(rwr_sample(g, 1, 5000))
	
	for split in split_list:
		sampled_graphs = [nx.read_edgelist(f) for f in glob("%s%s/split_%d/*" % (gen_graph_base,gen_name, split) )]
		metrics.network_properties([g_small], metrics_list, sampled_graphs, name="%s_split_%d" % (train_name, split))

