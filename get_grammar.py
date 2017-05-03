import sys
sys.path.insert(0, 'script/')

import new_em
import numpy as np

subgraph_size = 25
subgraph_num = 500
split = 1

train_file = "prepare_tree_rules/cit-HepTh/%d_sub/nonpartition/%d_sample/cit-HepTh_train.txt" % (subgraph_size, subgraph_num)

cv_train = new_em.ConvertRule(train_file, tree_count=subgraph_num)
gram = new_em.Grammar(cv_train.rule_dict, split)
cur_str_result = []
em = new_em.EM(gram, cv_train.Tree, cur_str_result)
em.iterations(use_converge=True)

train_loglikelihood = em.loglikelihood
rules = em.gram.get_valid_rules(cv_train)
for r in rules:
	print r
