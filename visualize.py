import re
import networkx as nx
import matplotlib.pyplot as plt
from collections import deque

class Graph(object):
	paren_re = re.compile(r'\((.*?)\)')

	def __init__(self):
		self.node_list = {}
		self.open_node_label = 0
		self.graph = nx.Graph()

	def __str__(self):
		ret = []
		for n in self.node_list:
			ret.append(str(self.node_list[n]))
		return " ".join(ret)

	def get_graph(self, out_file):
		#edges = []
		color_values = []
		for n in self.node_list:
			self.graph.add_node(n)
			if self.node_list[n].nonterm == 'N2_1':
				color_values.append(1.0)
			else:
				color_values.append(0.5)

			for c in self.node_list[n].children:
				#edges.append(n + str(c.label))
				self.graph.add_edge(n, c.label)
		
		nx.draw(self.graph, node_color=color_values)
		plt.show()

	def tokenize_graph_file(self, line):
		tokens = line.split()
		lhs, rhs = tokens[0], tokens[2]
		underscore = lhs.split("_")
		if len(underscore) > 1:
			underscore = underscore[1]
		lhs = Graph.paren_re.findall(lhs)[0]
		rhs = Graph.paren_re.findall(rhs)
		for i, r in enumerate(rhs):
			r_toks = r.split(":")
			rhs[i] = (r_toks[0].split(","), r_toks[1])
		return lhs, rhs, underscore

	def build_graph(self, file):
		with open(file) as myfile:
			queue = deque()
			for line in myfile:
				# left most derivation stack
				# pop the last item, then push the reverse of current nonterms on stack
				lhs, rhs, rule_underscore = self.tokenize_graph_file(line)

				if not queue:
					# start of tree file
					for rhs_node in rhs:
						edge_group = []
						for n in rhs_node[0]:
							if int(n) >= self.open_node_label:
								self.node_list[n] = Node(n, set())
								self.open_node_label += 1
							edge_group.append(self.node_list[n])
						if rhs_node[1] != 'T':
							queue.append(edge_group)
						# add edges 
						for i, n_u in enumerate(edge_group):
							for n_v in edge_group[i+1:]:
								n_u.add_child(n_v)
								n_v.add_child(n_u)
				else:
					#print(stack)
					start_clique = queue.popleft()
					lhs = lhs.split(",")
					lhs_map = {}

					for key, node in zip(lhs, start_clique):
						lhs_map[key] = node

					new_node_map = {}
					nonterms = []
					new_nodes = []
					for rhs_node in rhs:
						edge_group = []
						for n in rhs_node[0]:
							if n in lhs_map:
								edge_group.append(lhs_map[n])
							else:
								if int(n) not in new_node_map:
									self.node_list[str(self.open_node_label)] = Node(str(self.open_node_label), set())
									new_node_map[int(n)] = self.node_list[str(self.open_node_label)]
									new_nodes.append(self.node_list[str(self.open_node_label)])
									self.open_node_label += 1
								edge_group.append(new_node_map[int(n)])
						if rhs_node[1] != 'T':
							queue.append(edge_group)
						nonterm_symbol = 'N' + str(len(edge_group)) + "_" + rule_underscore
						for n in new_nodes:
							n.add_nonterm(nonterm_symbol)
						# add edges
						for i, n_u in enumerate(edge_group):
							for n_v in edge_group[i+1:]:
								n_u.add_child(n_v)
								n_v.add_child(n_u)						

class Node(object):
	def __init__(self, label, children):
		self.label = label
		self.children = children
		self.nonterm = None

	def __str__(self):
		ret = self.label + ": "
		for c in self.children:
			ret += c.label
			ret += ","
		return ret

	def add_child(self, child):
		self.children.add(child)
		pass

	def add_nonterm(self, s):
		self.nonterm = s


if __name__ == "__main__":
	graph = Graph()
	graph.build_graph("graph.txt")
	graph.get_graph("vis")


