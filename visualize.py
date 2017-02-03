import re
import sys
sys.path.insert(0, 'script/')

import networkx as nx
import matplotlib.pyplot as plt
from collections import deque

import os

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

	def get_graph(self, out_file, title, plot_count=1, total_count=1, start_figure=False, figure_count=1):
		#edges = []
		color_values = []
		"""
		for n in self.node_list:
			self.graph.add_node(n)
			if self.node_list[n].nonterm == 'N2_1':
				color_values.append(1.0)
			else:
				color_values.append(0.5)

			for c in self.node_list[n].children:
				#edges.append(n + str(c.label))
				self.graph.add_edge(n, c.label)
		"""
		if start_figure:
			plt.figure(figure_count)

		#pos = nx.spectral_layout(self.graph)
		#pos = nx.circular_layout(self.graph)
		#pos = nx.shell_layout(self.graph)
		pos = nx.fruchterman_reingold_layout(self.graph)
		ax = plt.subplot(total_count, 1, plot_count)
		ax.set_title(title)
		nx.draw(self.graph, pos=pos, node_color=color_values)

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
					added_node_label = set()
					for rhs_node in rhs:
						edge_group = []
						for n in rhs_node[0]:
							if n not in added_node_label:
								self.node_list[n] = Node(n, set())
								self.open_node_label += 1
								added_node_label.add(n)
							edge_group.append(self.node_list[n])
						if rhs_node[1] != 'T':
							queue.append(edge_group)
						# add edges 
						for i, n_u in enumerate(edge_group):
							for n_v in edge_group[i+1:]:
								n_u.add_child(n_v)
								n_v.add_child(n_u)
					# start symbol is a clique
					if lhs != 'S':
						for ni in added_node_label:
							for nj in added_node_label:
								if ni != nj:
									self.node_list[ni].add_child(self.node_list[nj])
									self.node_list[nj].add_child(self.node_list[ni])
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

		for n in self.node_list:
			self.graph.add_node(n)
			for c in self.node_list[n].children:
				#edges.append(n + str(c.label))
				self.graph.add_edge(n, c.label)						

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


def visualize_dir_pairs(dir_name):
	#graph = Graph()
	#graph.build_graph("out_graphs/N5_1.txt")
	#graph.get_graph("vis")

	graph_files = [(dir_name + "/" + f) for f in os.listdir(dir_name) if os.path.isfile(os.path.join(dir_name, f))]
	count = 1
	total_count = len(graph_files)
	total_plots = 2
	figure_count = 0
	start_figure = True

	for f in graph_files:
		g = Graph()
		g.build_graph(f)
		if count % 2 == 1:
			figure_count += 1
			start_figure = True
			plot_count = 1
		else:
			start_figure = False
			plot_count = 2
		g.get_graph("vis", os.path.basename(f).split(".")[0], plot_count, total_plots, start_figure, figure_count)
		count += 1
	plt.show()	


def dir_node_count(dir_name):
	graph_files = [(dir_name + "/" + f) for f in os.listdir(dir_name) if os.path.isfile(os.path.join(dir_name, f))]
	count = 1
	total_count = len(graph_files)
	total_plots = 2
	figure_count = 0
	start_figure = True

	node_dic = {}
	for f in graph_files:
		g = Graph()
		g.build_graph(f)
		node_dic[os.path.basename(f).split(".")[0]] = len(g.node_list)
	return node_dic


if __name__ == "__main__":
	visualize_dir_pairs("out_graphs")
