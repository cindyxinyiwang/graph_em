import re
import sys
from collections import deque, OrderedDict
import numpy as np
import david as da

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
		rules = []
		cur_nonterm = deque([self.start])
		term_count = 0
		while cur_nonterm:
			lhs = cur_nonterm.popleft()
			rhs_dic = self.rule_dict[lhs]
			rule = np.random.choice(rhs_dic.keys(), p = rhs_dic.values())
			rules.append((lhs, rule, rhs_dic[rule]))
			rule = rule.split()
			for r in rule:
				if r != "t":
					cur_nonterm.append(r)
				else:
					term_count += 1
		return rules, term_count

	def get_valid_rules(self):
		# return all rules with positive probability
		count = 0
		rules = []
		for lhs in self.rule_dict:
			for rhs, prob in self.rule_dict[lhs].items():
				if prob > 0:
					rules.append(("r" + str(count), lhs, rhs.split(), prob))
					count += 1
		return rules

class TreeNode(object):
	def __init__(self, val, children=None):
		self.val = val
		self.children = []
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
				#print lhs, rhs
				lhs, rhs = self.hrg_to_cfg(lhs[0], rhs)
				# add to grammar
				if lhs not in self.rule_dict:
					self.rule_dict[lhs] = {}
				self.rule_dict[lhs][rhs] = 1

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

if __name__ == "__main__":
	cv = ConvertRule("decomp_new.txt")
	#for tree in cv.tree_list:
	#	tree.print_tree()
	gram = Grammar(cv.rule_dict, 3)
	#gram.printAllRules()
	
	#cv.Tree.print_tree()
	em = EM(gram, cv.Tree)
	em.iterations(50)

	graph_size_counts = []
	rules = []
	for i in xrange(100):
		rules, term_count = em.gram.sample()
		#print "graph ", i
		#print len(rules)
		graph_size_counts.append(term_count)

	graph_size_counts.sort()
	fig = plt.figure()
	ax = fig.add_subplot(111)
	ax.set_xlabel('size of tree')
	ax.set_ylabel('count')
	ax.hist(graph_size_counts, bins=[i for i in xrange(100)])
	
	#for r in rules:
	#	print r

	grammar = em.gram.get_valid_rules()
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

