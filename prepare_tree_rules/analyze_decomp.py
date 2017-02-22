"""
analyze the tree decomposition rules
"""

file = "rounters_left_derive.txt"

grammar_dict = {}
with open(file) as tree_file:
	for line in tree_file:
		toks = line.split()
		lhs, rhs = toks[0], toks[2]
		if lhs not in grammar_dict:
			grammar_dict[lhs] = {}
		grammar_dict[lhs][rhs] = grammar_dict[lhs].get(rhs, 0) + 1

c = 0
count_rule = 0
nonterm_set = set()
for lhs in grammar_dict:
	for rhs, count in grammar_dict[lhs].items():
		#print "(a,b,c)", rhs, count
		c += count
		count_rule += 1
print "total number of rules with (a,b,c) as LHS", len(grammar_dict["(a,b,c)"].items())
print "total count of rules with (a,b,c) as LHS", c

print "num rules", c, count_rule, len(grammar_dict.items())

for lhs in grammar_dict:
	total_count = sum(grammar_dict[lhs].values())
	for rhs in grammar_dict[lhs]:
		grammar_dict[lhs][rhs] = float(grammar_dict[lhs][rhs]) / total_count

"""
for rhs, prob in grammar_dict["(a,b,c)"].items():
	print "(a,b,c)", rhs, prob

for lhs in grammar_dict:
	for rhs in grammar_dict[lhs]:
		print lhs, rhs, grammar_dict[lhs][rhs]

"""