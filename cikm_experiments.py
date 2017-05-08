__version__="0.1.0"
import argparse
import os, sys
import networkx as nx
import treedecomps.load_edgelist_from_dataframe as ledf
import treedecomps.graph_utils as gutil
import traceback
from collections import defaultdict
import prepare_tree_rules.PHRG as phrg #import phrg_derive_prod_rules_partition
import prepare_tree_rules.graph_sampler as gs
import prepare_tree_rules.tree_decomposition as td



DEBUG = False
# //////////////////////////////////////////////////////////////////////////////
def get_parser ():
    parser = argparse.ArgumentParser(description='Experiemnts Workflow for CIKM')
    parser.add_argument('--orig', required=True, nargs=1, help='Filename edgelist graph')
    #parser.add_argument('-tw', action='store_true', default=False, \
    #     required=False, help="print xphrg (mcs) tw")
    parser.add_argument('--version', action='version', version=__version__)
    return parser

def LogInfo(str_arg):
  print "~"*len(str_arg)
  print str_arg
  print "~"*len(str_arg)


def get_25node_subgraphs_inrage(graph,start_size, end_size, step):
  G=graph
  G.remove_edges_from(G.selfloop_edges())
  giant_nodes = max(nx.connected_component_subgraphs(G), key=len)
  G = nx.subgraph(G, giant_nodes)
  num_nodes = G.number_of_nodes()

  gutil.graph_checks(G)

  g_obj_d = defaultdict(list) # dict with list of graph objects
  for nbr_sg in range(start_size, end_size+1, step):
    #g_obj_d[nbr_sg].append(sample_subgraphs_G_n_K(G,25,nbr_sg))
    print "\t",nbr_sg
    g_obj_d[nbr_sg].append(phrg.probabilistic_hrg(G, nbr_sg, 25))

  print np.shape(g_obj_d)
  return g_obj_d


def main():
  sgb_d = [get_25node_subgraphs_inrage(G, 50,500,step=50)]

def sample_refrence_graph(G, ofname, K_ls, n):
    if G is None: return

    G.remove_edges_from(G.selfloop_edges())
    giant_nodes = max(nx.connected_component_subgraphs(G), key=len)
    G = nx.subgraph(G, giant_nodes)

    if n is None:
        num_nodes = G.number_of_nodes()
    else:
        num_nodes = n

    gutil.graph_checks(G)

    subgraph_objs_lst = []
    if DEBUG: print
    if DEBUG: print "--------------------"
    if DEBUG: print "-Tree Decomposition-"
    if DEBUG: print "--------------------"
    prod_rules = {}
    left_deriv_prod_rules = []

    cntr = 1
    for K in K_ls:
      left_deriv_file_name = "ProdRules/subg_{}of{}_{}_rules.txt".format(cntr,K,ofname)
      print left_deriv_file_name
      cntr += 1

      for Gprime in gs.rwr_sample(G, K, n):
        subgraph_objs_lst.append( Gprime )
        T = td.quickbb(Gprime)
        root = list(T)[0]
        T = td.make_rooted(T, root)
        T = phrg.binarize(T)
        root = list(T)[0]
        root, children = T
        td.new_visit(T, G, prod_rules, left_deriv_prod_rules)

    left_derive_file = open(left_deriv_file_name, 'w')
    for r in left_deriv_prod_rules:
    	left_derive_file.write(r)
    	left_derive_file.write('\n')
    left_derive_file.close()

    return subgraph_objs_lst

if __name__ == '__main__':
  '''
  Assumptions:
  - edgelist (datasets) are in ./data
  - subgraphs can be saved in ./data
  - clique trees will be placed in ./data
  '''

  parser = get_parser()
  args = vars(parser.parse_args())

  orig_file = args['orig'][0]

  datframes = ledf.Pandas_DataFrame_From_Edgelist(args['orig'])
  df = datframes[0]
  g_name = os.path.basename(orig_file).split('.')[-1]
  LogInfo(g_name)
  if df.shape[1] >=3:
    G = nx.from_pandas_dataframe(df, 'src', 'trg', ['ts'])  # whole graph
  else:
    G = nx.from_pandas_dataframe(df, 'src', 'trg')
  G.name = g_name
  LogInfo("Graph Loaded")
  try:
    # sub_graphs_lst  = [get_25node_subgraphs_inrage(G, 50,500,step=50)]
    subgraphs_lst = sample_refrence_graph(G,g_name,range(4,8,4), 25)
    #print (rules_fname)
    #print [x.number_of_nodes() for x in subgraphs_lst]

  except  Exception, e:
    print str(e)
    traceback.print_exc()
    os._exit(1)
  sys.exit(0)
