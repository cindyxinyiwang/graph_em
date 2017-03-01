import networkx as nx 
import PHRG as phrg
import matplotlib.pyplot as plt

def prepare_tree_decomp(G, left_derive_file_name, num_samples, subgraph_szie):
	prod_rules = {}
	p_rules = phrg.probabilistic_hrg_deriving_prod_rules(G, left_derive_file_name, num_samples, subgraph_szie)
	#print p_rules.items[0]

if __name__ == "__main__":
	#G = nx.hypercube_graph(9)
	G = nx.read_edgelist("data/rounters.txt", comments="#")
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
	prepare_tree_decomp(G, "routers/200_sub/routers_test.txt", 4, 200)
