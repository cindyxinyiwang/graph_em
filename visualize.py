import re
from graphviz import Digraph

class Graph(object):
	paren_re = re.compile(r'\((.*?)\)')

	def __init__(self):
		self.node_list = {}
		self.open_node_label = 0
		self.dot = Digraph(comment="graph from HRG")

	def __str__(self):
		ret = []
		for n in self.node_list:
			ret.append(str(self.node_list[n]))
		return " ".join(ret)

	def get_graph(self, out_file):
		edges = []
		for n in self.node_list:
			self.dot.node(n, n)
			for c in self.node_list[n].children:
				edges.append(n + str(c.label))
		self.dot.edges(edges)
		self.dot.render(out_file, view=True)

	def tokenize_graph_file(self, line):
		tokens = line.split()
		lhs, rhs = tokens[0], tokens[2]
		lhs = Graph.paren_re.findall(lhs)[0]
		rhs = Graph.paren_re.findall(rhs)
		for i, r in enumerate(rhs):
			r_toks = r.split(":")
			rhs[i] = (r_toks[0].split(","), r_toks[1])
		return lhs, rhs

	def build_graph(self, file):
		with open(file) as myfile:
			stack = []
			for line in myfile:
				# left most derivation stack
				# pop the last item, then push the reverse of current nonterms on stack
				lhs, rhs = self.tokenize_graph_file(line)

				if not stack:
					# start of tree file
					nonterms = []
					for rhs_node in rhs:
						edge_group = []
						for n in rhs_node[0]:
							if int(n) >= self.open_node_label:
								self.node_list[n] = Node(n, set())
								self.open_node_label += 1
							edge_group.append(self.node_list[n])
						if rhs_node[1] == 'N':
							nonterms.append(edge_group)
						# add edges 
						for i, n_u in enumerate(edge_group):
							for n_v in edge_group[i+1:]:
								n_u.add_child(n_v)
								n_v.add_child(n_u)
					nonterms.reverse()
					stack.extend(nonterms)
				else:
					#print(stack)
					start_clique = stack.pop()
					lhs, rhs = self.tokenize_graph_file(line)
					lhs = lhs.split(",")
					lhs_map = {}
					for key, node in zip(lhs, start_clique):
						lhs_map[key] = node

					new_node_map = {}
					nonterms = []
					for rhs_node in rhs:
						edge_group = []
						for n in rhs_node[0]:
							if n in lhs_map:
								edge_group.append(lhs_map[n])
							else:
								if int(n) not in new_node_map:
									self.node_list[str(self.open_node_label)] = Node(str(self.open_node_label), set())
									new_node_map[int(n)] = self.node_list[str(self.open_node_label)]
									self.open_node_label += 1
								edge_group.append(new_node_map[int(n)])
						if rhs_node[1] == 'N':
							nonterms.append(edge_group)
						# add edges
						for i, n_u in enumerate(edge_group):
							for n_v in edge_group[i+1:]:
								n_u.add_child(n_v)
								n_v.add_child(n_u)	
					nonterms.reverse()
					stack.extend(nonterms)					

class Node(object):
	def __init__(self, label, children):
		self.label = label
		self.children = children

	def __str__(self):
		ret = self.label + ": "
		for c in self.children:
			ret += c.label
			ret += ","
		return ret

	def add_child(self, child):
		self.children.add(child)
		pass

if __name__ == "__main__":
	graph = Graph()
	graph.build_graph("graph.txt")
	graph.get_graph("vis")


