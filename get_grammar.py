import sys
sys.path.insert(0, 'script/')

import new_em
import numpy as np
import david as da

subgraph_size = 25
subgraph_num = 4
split = 1

train_file = "prepare_tree_rules/cit-HepTh/%d_sub/nonpartition/%d_sample/cit-HepTh_train.txt" % (subgraph_size, subgraph_num)

cv_train = new_em.ConvertRule(train_file, tree_count=subgraph_num)
gram = new_em.Grammar(cv_train.rule_dict, split)
cur_str_result = []
em = new_em.EM(gram, cv_train.Tree, cur_str_result)
em.iterations(use_converge=True)

train_loglikelihood = em.loglikelihood
rules = em.gram.get_valid_rules(cv_train)

g = da.Grammar('S')
for (id, hrg, prob) in rules:
	lhs, rhs = hrg
	#print lhs, rhs
	g.add_rule(da.RuleSplit(id, lhs, rhs, prob, True))

#for lhs in g.by_lhs:
#	print lhs
#	print [(r.lhs, r.cfg_rhs) for r in g.by_lhs[lhs]]

g.set_max_size(50)
graph_rules = g.sample(25)

for r in graph_rules:
	print r
