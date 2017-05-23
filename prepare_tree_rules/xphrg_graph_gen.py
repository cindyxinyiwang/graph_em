__version__="0.1.0"
__author__ = ['Salvador Aguinaga', 'Rodrigo Palacios', 'David Chaing', 'Tim Weninger']

import os
import pprint as pp
import re
import pandas as pd

import networkx as nx
import sys
# sys.path.insert(0, '../prepare_tree_rules/')
import david as pcfg
from PHRG import grow
# import graph_sampler as gs
# import tree_decomposition as td

def xphrg_graph_gen(prodrules_fname, nbr_sg, n):
    '''
    returns a list of Hstars
    '''
    # trn_files, subgraph_nbr_lst[-1], 25
    # print prodrules_fname, nbr_sg, n
    # rules = np.loadtxt(prodrules_fname, delimiter="\t")
    rules = []
    df = pd.read_csv("ProdRules/"+prodrules_fname[0], delimiter="\t",header=None)
    rules = df.values
    print len(rules), 'rules read'

    g = pcfg.Grammar('S')
    for (id, lhs, rhs, prob) in rules:
      print (id), (lhs), (rhs), (prob)
      g.add_rule(pcfg.Rule(id, lhs, rhs, prob))

    num_nodes = n
    num_samples = 1

    g.set_max_size(6)

    if 1: print "Done with max size"

    Hstars = []

    for i in range(0, num_samples):
      rule_list = g.sample(6)
      # print rule_list
      hstar = grow(rule_list, g)[0]
      Hstars.append(hstar)
    print len(Hstars)
    return Hstars
