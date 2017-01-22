import networkx as nx 
import PHRG as phrg

def prepare_tree_decomp(G, left_derive_file_name):
	prod_rules = {}
	p_rules = phrg.probabilistic_hrg_deriving_prod_rules(G, left_derive_file_name)
	#print p_rules.items[0]

if __name__ == "__main__":
	G = nx.read_edgelist("data/com-amazon.ungraph.txt", comments="#")
	prepare_tree_decomp(G, "amazon_left_derive.txt")