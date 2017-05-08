import networkx as nx 
import PHRG as phrg
#import matplotlib.pyplot as plt
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

def prepare_tree_decomp(G, left_derive_file_name, num_samples, subgraph_szie):
	prod_rules = {}
	p_rules = phrg.probabilistic_hrg_deriving_prod_rules(G, left_derive_file_name, num_samples, subgraph_szie)
	#print p_rules.items[0]

def non_partition_sample(graph_name):
	"""
	Sample tree decomposition from the same graph without graph partition
	"""
	samples = [4]
	subgraph_size = [25]
	#file_name = ["train", "test", "hold"]
	file_name = [str(i) for i in xrange(500)]
	for size in subgraph_size:
		for s in samples:
			dir_name = "%s/%d_sub/nonpartition/%d_sample/" % (graph_name, size, s)
			ensure_dir(dir_name)
			for f in file_name:
				name = "%s%s_%s.txt" % (dir_name, graph_name, f)
				prepare_tree_decomp(G, name, s, size)

def partition_sample(graph_name):
	"""
	Sample tree decomposition from the same graph with graph partition
	"""
	name_list = ["train", "test", "hold"]
	train_size_list = [4, 8, 12, 16]
	sample_size_list = [16, 4, 4]
	subgraph_size_list = [100, 200, 300, 400, 500]
	for subgraph_size in subgraph_size_list:
		for train_size in train_size_list:
			file_name_list = []
			sample_size_list[0] = train_size
			base_dir_name = "%s/%d_sub/partition/%d_train/" % (graph_name, subgraph_size, train_size)
			
			ensure_dir(base_dir_name)
			for name in name_list:
				file_name_list.append("%s%s_%s.txt" % (base_dir_name, graph_name, name))
			phrg.phrg_derive_prod_rules_partition(G, file_name_list, sample_size_list, subgraph_size)

if __name__ == "__main__":
	#G = nx.barabasi_albert_graph(30000, 15)
	G = nx.read_edgelist("data/ba-30000-3.txt", comments="#")
	#G = nx.karate_club_graph()
	G.remove_edges_from(G.selfloop_edges())
	#orig_size = len(G)

	# Get the largest connected components
	#G_big = max(nx.connected_component_subgraphs(G), key=len)
	#comp_size = len(G_big)
	#print "orig_size", orig_size
	#print "comp_size", comp_size
	#nx.draw(G)
	#plt.show()

	non_partition_sample("ba-test")
