import networkx as nx 
import PHRG as phrg
import matplotlib.pyplot as plt

def prepare_tree_decomp(G, left_derive_file_name):
	prod_rules = {}
	p_rules = phrg.probabilistic_hrg_deriving_prod_rules(G, left_derive_file_name)
	#print p_rules.items[0]

if __name__ == "__main__":
	G = nx.read_edgelist("data/com-dblp.ungraph.txt", comments="#")
	nx.draw(G)
	plt.show()
	prepare_tree_decomp(G, "dblp_left_derive.txt")