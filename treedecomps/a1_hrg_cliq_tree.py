# script: a1_hrg_cliq_tree.py

import networkx as nx
import traceback
import os
import sys
import tree_decomposition as td
import PHRG as phrg

def unfold_2wide_tuple(tup, ind="", i=0):
  if not tup:
    return
  # print i,type(tup),tup[0],
  # print i,ind, tup[0]
  if isinstance(tup[1], (list)):
    for x in tup[1]:
      ind += "  "
      unfold_2wide_tuple(x, ind, i+1)
  else:
    print 'not list', tup[1]


def get_clique_tree(g):
  g.remove_edges_from(g.selfloop_edges())
  giant_nodes = max(nx.connected_component_subgraphs(g), key=len)
  g = nx.subgraph(g, giant_nodes)
  prod_rules = {}
  T = td.quickbb(G)

  root = list(T)[0]
  T = td.make_rooted(T, root)

  T = phrg.binarize(T)
  unfold_2wide_tuple(T)
  return

def load_edgelist(gfname):
  import pandas as pd
  try:
    edglst = pd.read_csv(gfname, comment='%', delimiter='\s+')
    # print edglst.shape
    if edglst.shape[1]==1: edglst = pd.read_csv(gfname, comment='%', delimiter="\s+")

  except Exception, e:
    print "EXCEPTION:",str(e)
    traceback.print_exc()
    sys.exit(1)

  if edglst.shape[1] == 3:
    edglst.columns = ['src', 'trg', 'wt']
  elif edglst.shape[1] == 4:
    edglst.columns = ['src', 'trg', 'wt','ts']
  else:
    edglst.columns = ['src', 'trg']
  g = nx.from_pandas_dataframe(edglst,source='src',target='trg')
  g.name = os.path.basename(gfname)
  return g

if __name__ == '__main__':
  print 'done'
  # fname = "/Users/saguinag/Theory/DataSets/out.ucidata-gama"
  # fname = "/Users/saguinag/Theory/DataSets/out.brunson_southern-women_southern-women"
  # G = load_edgelist(fname)
  # T = get_clique_tree(G)
  # print 'Done'
