import re
import sys
from collections import deque, OrderedDict
import numpy as np
import david as da
import PHRG.probabilistic_cfg as pcfg
import PHRG.salPHRG
import networkx as nx

import matplotlib.pyplot as plt

class Grammar(object):
	def __init__(self, rule_dict, k):
		self.alphabet = set(["t"])
		self.start = "S"
		self.rule_dict = rule_dict
		self.k = k
		self.getAllRules()

	def getAllRules(self):
		"""
		convert original rule_dict into rule_dict with k copies of each nonterm
		"""
		# get a combination of all rules
		def get_combs(nonterms, terms, rules_comb, idx, cur_rule):
			if idx == len(nonterms):
				r = " ".join(cur_rule) + " " + " ".join(terms)
				rules_comb.append(r.strip())
				return
			for i in xrange(1, self.k+1):
				cur_nonterm = nonterms[idx] + "_" + str(i)
				self.alphabet.add(cur_nonterm)
				cur_rule.append(cur_nonterm)
				get_combs(nonterms, terms, rules_comb, idx + 1, cur_rule)
				cur_rule.pop()

		new_rule_dict = {}
		for lhs in self.rule_dict:
			rules = self.rule_dict[lhs]
			# get all possible combs of rules
			rules_comb = []
			for r in rules:
				r_tok = r.split()
				nonterms = [t for t in r_tok if t != "t"]
				terms = [t for t in r_tok if t == "t"]
				# find all combs
				get_combs(nonterms, terms, rules_comb, 0, [])
			new_lhs = []
			if lhs == "S":
				new_lhs = ["S"]
				self.alphabet.add("S")
			else:
				for i in xrange(1, self.k+1):
					new_lhs.append(lhs + "_" + str(i))
					self.alphabet.add(lhs + "_" + str(i))
			for l in new_lhs:
				new_rule_dict[l] = self._get_rules(rules_comb)
		self.rule_dict = new_rule_dict

	def printAllRules(self):
		for s in self.rule_dict:
			for r in self.rule_dict[s]:
				print s + "->" + r + " " + str(self.rule_dict[s][r])

	def _get_rules(self, rules):
		"""
		input: list of all rules
		output: dictionary of rules and random probs
		"""
		n = len(rules)
		a = np.random.random(n)
		a /= a.sum()
		d = {}
		count = 0
		for i in rules:
			d[i] = a[count]
			count += 1
		return d

	def sample(self):
		tree = TreeNode(self.start)
		rules = []
		cur_nonterm = deque([self.start])
		cur_tree = deque([tree])
		term_count = 0
		while cur_nonterm:
			lhs = cur_nonterm.popleft()
			root_node = cur_tree.popleft()
			rhs_dic = self.rule_dict[lhs]
			rule = np.random.choice(rhs_dic.keys(), p = rhs_dic.values())
			rules.append((lhs, rule, rhs_dic[rule]))
			rule = rule.split()
			for r in rule:
				child_node = TreeNode(r)
				root_node.children.append(child_node)
				if r != "t":
					cur_nonterm.append(r)
					cur_tree.append(child_node)
				else:
					term_count += 1
		return rules, term_count, tree

	def get_valid_rules(self, cv):
		# return all rules with positive probability
		count = 0
		rules = []
		for lhs in self.rule_dict:
			for rhs, prob in self.rule_dict[lhs].items():
				hrg = cv.get_orig_cfg(lhs, rhs)
				if prob > 0:
					#rules.append(("r" + str(count), lhs, rhs.split(), prob))
					rules.append(("r" + str(count), hrg, prob))
					count += 1
		return rules

class TreeNode(object):
	def __init__(self, val, children=None):
		self.val = val
		self.children = []
		self.term_count = 0
		self.counted = False

	def print_tree(self):
		cur_level = deque([self])
		next_level = deque()
		while cur_level:
			cur = cur_level.popleft()
			sys.stdout.write(cur.val + " ")
			for child in cur.children:
				next_level.append(child)
			if not cur_level:
				sys.stdout.write("\n")
				cur_level = next_level
				next_level = deque()

	def get_term_count(self, term_count_dic, nonterm_count_dict, nonterm_size_dic):
		if self.val == 't':
			#count_dic[self.val] = count_dic.get(self.val, 0) + 1
			return 1
		nonterm_count_dict[self.val] = nonterm_count_dict.get(self.val, 0) + 1
		if self.counted:
			return self.term_count
		count = 0
		for c in self.children:
			count += c.get_term_count(term_count_dic, nonterm_count_dict, nonterm_size_dic)
		self.term_count = count
		self.counted = True

		term_count_dic[self.val] = term_count_dic.get(self.val, 0) + count

		if self.val not in nonterm_size_dic:
			nonterm_size_dic[self.val] = []
		nonterm_size_dic[self.val].append(count)

		return count

class ConvertRule(object):
	def __init__(self, tree_file):
		self.treefile = tree_file
		self.cfg_to_hrg_map = {}
		self.Tree = None

		self.nonterm = set(["S"])
		self.rule_dict = {}

		self.build_rules()
		#self.Tree.print_tree()
		#print self.nonterm
		#print self.level_tree_nodes

	def hrg_to_cfg(self, lhs, rhs):
		if lhs != 'S':
			size = lhs.count(",") + 1
			lhs = 'N' + str(size)
			self.nonterm.add(lhs)

		t_symb = set()
		n_symb = []
		for r in rhs:
			if r.endswith(":N"):
				size = r.count(",") + 1
				n_symb.append('N' + str(size))
				self.nonterm.add('N' + str(size))
			for x in r.split(":")[0].split(","):
				if x.isdigit(): 
					t_symb.add(x)
		rhs = " ".join(n_symb) + " " + " ".join(["t" for i in xrange(len(t_symb))])
		#rhs = n_symb + ["t" for i in xrange(len(t_symb))]
		#print lhs, rhs
		return lhs, rhs.strip()

	def build_rules(self):
		tree = TreeNode("S")
		stack = [tree]
		with open(self.treefile) as myfile:
			for line in myfile:
				rules = line.split()
				lhs, rhs = rules[0], rules[2]
				lhs = re.findall(r'\((.*?)\)', lhs)
				rhs = re.findall(r'\((.*?)\)', rhs)
				hrg = (lhs[0], rhs)
				#print lhs, rhs
				lhs, rhs = self.hrg_to_cfg(lhs[0], rhs)
				# add to grammar
				if lhs not in self.rule_dict:
					self.rule_dict[lhs] = {}
				self.rule_dict[lhs][rhs] = 1

				self.cfg_to_hrg_map[lhs + "->" + rhs] = hrg
				#self.cfg_to_hrg_map[] = line
				"""
				if not stack:
					self.tree_list.append(tree)
					tree = TreeNode("S")
					stack = [tree]
				"""
				parent = stack.pop()
				
				add_stack = []
				for r in rhs.split():
					n = TreeNode(r)
					if r != "t":
						add_stack.append(n)
					parent.children.append(n)
				add_stack.reverse()
				stack.extend(add_stack)
		self.Tree = tree

	def get_orig_cfg(self, lhs, rhs):
		if lhs == "S":
			base_lhs, c_lhs = "S", -1
		else:
			base_lhs, c_lhs = lhs.split("_")[0], lhs.split("_")[1]
		base_rhs, c_rhs = [], []
		rhs = rhs.split()
		for t in rhs:
			if t != "t":
				t = t.split("_")
				base_rhs.append(t[0])
				c_rhs.append(t[1])
			else:
				base_rhs.append("t")
		orig_hrg = self.cfg_to_hrg_map[base_lhs + "->" + " ".join(base_rhs)]
		#print (orig_hrg, lhs, rhs)
		orig_hrg_lhs, orig_hrg_rhs = orig_hrg[0], orig_hrg[1]
		if c_lhs >= 0:
			hrg_lhs = "(" + orig_hrg_lhs + ")_" + c_lhs
		else:
			hrg_lhs = "(" + orig_hrg_lhs + ")"
		hrg_rhs = []
		i = 0
		for t in orig_hrg_rhs:
			if t.endswith("N"):
				t = t + "_" + c_rhs[i]
				i += 1
			hrg_rhs.append(t)
		return (hrg_lhs, hrg_rhs)

def get_level_nodes(tree):
	i = 0
	level_tree_nodes = {}
	level_tree_nodes[i] = [tree]
	cur_level = deque([tree])
	next_level = deque()
	while cur_level:
		cur = cur_level.popleft()
		for child in cur.children:
			if child.val != "t":
				next_level.append(child)
		if not cur_level:
			i += 1
			level_tree_nodes[i] = [n for n in next_level]
			cur_level = next_level
			next_level = deque()
	return level_tree_nodes

class EM(object):
	def __init__(self, gram, tree):
		self.gram = gram
		self.tree = tree

	def get_nonterm_num(self, string):
		# get how many numbers the nonterminal contains
		# N4_1 returns 4
		toks = string.split("_")
		
		return int(toks[0][1])

	def expect(self):
		self.f_rules = {}
		for s in self.gram.rule_dict:
			self.f_rules[s] = {}
			for r in self.gram.rule_dict[s]:
				self.f_rules[s][r] = 0.0

		self.loglikelihood = 0
		self.inside = {}
		self.outside = {}

		level_tree_nodes = get_level_nodes(self.tree)
		level_tree_nodes = OrderedDict(sorted(level_tree_nodes.items(), reverse=True))
		for level in level_tree_nodes:
			for n in level_tree_nodes[level]:
				if n not in self.inside:
					self.inside[n] = {}
					#print n.val
				node_nonterm = set()
				if n.val == "S":
					node_nonterm.add("S")
				else:
					for i in xrange(1, self.gram.k+1):
						node_nonterm.add(n.val + "_" + str(i))
				for x in self.gram.alphabet:
					self.inside[n][x] = 0.0
					if x not in node_nonterm:
						continue
					l_children = len(n.children)
					for rhs, prob in self.gram.rule_dict[x].items():
						rhs = rhs.split()
						if len(rhs) != l_children:
							continue
						tmp = prob
						for ni, xi in zip(n.children, rhs):
							if ni.val == "t":
								if xi == "t":
									continue
								else:
									tmp = 0
									break
							tmp = tmp * self.inside[ni][xi]
						self.inside[n][x] += tmp
		"""
		for l, probs in self.inside.items():
			for r, p in probs.items():
				if p > 0:
					self.loglikelihood += np.log(p) 
		"""
		#print self.inside
		level_tree_nodes = OrderedDict(sorted(level_tree_nodes.items()))
		self.outside[level_tree_nodes[0][0]] = {}
		for x in self.gram.alphabet:
			self.outside[level_tree_nodes[0][0]][x] = 0.0
		self.outside[level_tree_nodes[0][0]][self.gram.start] = 1.0

		self.loglikelihood = np.log(self.inside[level_tree_nodes[0][0]][self.gram.start])

		for level in level_tree_nodes:
			for n in level_tree_nodes[level]:
				l_children = len(n.children)
				node_nonterm = set()
				if n.val == "S":
					node_nonterm.add("S")
				else:
					for i in xrange(1, self.gram.k+1):
						node_nonterm.add(n.val + "_" + str(i))
				for x in self.gram.alphabet:
					if x not in node_nonterm:
						continue
					for rhs, prob in self.gram.rule_dict[x].items():
						rhs = rhs.split()
						#print product
						if len(rhs) != l_children:
							continue
						for ni, xi in zip(n.children, rhs):
							if ni not in self.outside:
								self.outside[ni] = {}
								for x_p in self.gram.alphabet:
									self.outside[ni][x_p] = 0.0
							#print self.inside[ni][xi], product, prob
							product = 1.0
							#print [c.val for c in n.children]
							for ni_p, xi_p in zip(n.children, rhs):
								if ni_p.val == "t":
									if xi_p == "t":
										continue
									else:
										product = 0
										break
								if ni != ni_p:
									product = self.inside[ni_p][xi_p] * product
							#self.outside[ni][xi] += self.outside[n][x] * prob * product / self.inside[ni][xi]
							self.outside[ni][xi] += self.outside[n][x] * prob * product
		
		#print self.outside
		# get expected counts
		for level in level_tree_nodes:
			for n in level_tree_nodes[level]:
				node_nonterm = set()
				if n.val == "S":
					node_nonterm.add("S")
				else:
					for i in xrange(1, self.gram.k+1):
						node_nonterm.add(n.val + "_" + str(i))

				l_children = len(n.children)
				for x in self.gram.alphabet:
					if x not in node_nonterm:
						continue
					for rhs, prob in self.gram.rule_dict[x].items():
						rhs = rhs.split()
						if len(rhs) != l_children:
							continue
						tmp = prob * self.outside[n][x]
						for ni, xi in zip(n.children, rhs):
							if ni.val == "t":
								if xi == "t":
									continue
								else:
									tmp = 0
									break
							tmp *= self.inside[ni][xi]
						#print x, rhs
						#print self.gram.rule_dict[x]
						self.f_rules[x][" ".join(rhs)] += tmp
		#print self.f_rules

	def maximize(self):
		for x in self.gram.rule_dict:
			r_rules = self.gram.rule_dict[x]
			r_expects = self.f_rules[x]
			sum_x = 0
			for s in r_expects:
				sum_x += r_expects[s]

			for r in r_rules:
				self.gram.rule_dict[x][r] = r_expects[r]/sum_x

	def iterations(self, iteration):
		for i in xrange(iteration):
			self.expect()
			self.maximize()
			print self.loglikelihood

def plot_nonterm_stats(nonterm_size_dic):
	nonterm_groups = {}
	for n in nonterm_ave_size_dic.keys():
		base = n.split("_")[0]
		if not base in nonterm_groups:
			nonterm_groups[base] = []
		nonterm_groups[base].append(n)

	for base in nonterm_groups:
		nonterms = nonterm_groups[base]
		for n in nonterms:
			print (n, "mean: ", np.mean(nonterm_size_dic[n]), "std: ", np.std(nonterm_size_dic[n]))
			print nonterm_size_dic[n]
	"""
	fig = plt.figure()
	ax = fig.add_subplot(111)
	ax.set_xlabel('size of subtree for N2')
	ax.set_ylabel('count')
	ax.hist(nonterm_size_dic["N5_1"], bins=[i for i in xrange(100)], alpha=0.5)
	ax.hist(nonterm_size_dic["N5_2"], bins=[i for i in xrange(100)], alpha=0.5)
	ax.hist(nonterm_size_dic["N5_3"], bins=[i for i in xrange(100)], alpha=0.5)
	"""

if __name__ == "__main__":
	cv = ConvertRule("decomp_new.txt")
	#for tree in cv.tree_list:
	#	tree.print_tree()
	gram = Grammar(cv.rule_dict, 2)
	#gram.printAllRules()
	
	#cv.Tree.print_tree()
	em = EM(gram, cv.Tree)
	em.iterations(50)
	"""
	rules, term_count, tree = em.gram.sample()
	count_dict = {}
	tree.get_term_count(count_dict)
	print count_dict

	"""
	graph_size_counts = []
	rules = []
	
	nonterm_ave_size_dic = {}
	nonterm_size_dic = {}
	for i in xrange(100):
		term_count_dict = {}
		nonterm_count_dict = {}
		rules, term_count, tree = em.gram.sample()
		tree.get_term_count(term_count_dict, nonterm_count_dict, nonterm_size_dic)
		for t in term_count_dict:
			if t not in nonterm_ave_size_dic:
				nonterm_ave_size_dic[t] = []
			nonterm_ave_size_dic[t].append(term_count_dict[t] / float(nonterm_count_dict[t]))
		graph_size_counts.append(term_count)

	plot_nonterm_stats(nonterm_ave_size_dic)
	plot_nonterm_stats(nonterm_size_dic)

	graph_size_counts.sort()

  	fig = plt.figure()
	ax = fig.add_subplot(111)
	ax.set_xlabel('size of tree')
	ax.set_ylabel('count')
	ax.hist(graph_size_counts, bins=[i for i in xrange(100)])
	
	grammar = em.gram.get_valid_rules(cv)

	#for r in rules:
	#	print r

	for r in grammar:
		print r 
	
	plt.show()

	"""
	rules_dict = {}
	i = 0
	for lhs, rule_dic in em.gram.rule_dict.items():
		for rhs, prob in rule_dic.items():
			i += 1
			#rules.append(("r"+str(i), lhs, rhs.split(), prob))
			rules_dict["r"+str(i)] = ("r"+str(i), lhs, rhs.split(), prob)

	sample_gram = da.Grammar('S')
	for (id, lhs, rhs, prob) in rules_dict.values():
		sample_gram.add_rule(da.Rule(id, lhs, rhs, prob, False))

	sample_gram.set_max_size(20)
	samp = sample_gram.sample(20)
	for id in samp:
		print rules_dict[id]
	"""
	rules_dict = {}
	i = 0
	for lhs, rule_dic in em.gram.rule_dict.items():
		for rhs, prob in rule_dic.items():
			i += 1
			#rules.append(("r"+str(i), lhs, rhs.split(), prob))
			rules_dict["r"+str(i)] = ("r"+str(i), lhs, rhs.split(), prob)

	sample_gram = pcfg.Grammar('S')
	for (id, lhs, rhs, prob) in rules_dict.values():
		# sample_gram.add_rule(pcfg.Rule(id, lhs, rhs, prob, False))
		sample_gram.add_rule(pcfg.Rule(id, lhs, rhs, prob))

	sample_gram.set_max_size(34)
	samp = sample_gram.sample(34)
	# for id in samp:
	#	 print rules_dict[id]
	# # """
	hstar = salPHRG.grow(samp, sample_gram)[0]

	print nx.info(hstar) 
