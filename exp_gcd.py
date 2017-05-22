__version__="0.1.0"

import matplotlib
matplotlib.use('pdf')
import os 
from glob import glob
from prepare_tree_rules.graph_sampler import rwr_sample
from treedecomps.load_edgelist_from_dataframe import Pandas_DataFrame_From_Edgelist
import treedecomps.net_metrics as metrics
import networkx as nx
import traceback
import argparse
import sys

# //////////////////////////////////////////////////////////////////////////////
def get_parser ():
  parser = argparse.ArgumentParser(description='Experiemnts Workflow for CIKM')
  parser.add_argument('--orig', required=True, nargs=1,help="Edgelist filename")
  parser.add_argument('--cgs',  required=True, nargs=1,help="Gen graph")
  parser.add_argument('--xnbr',  required=False, nargs=1,help="Run nbr (ignore)")
  parser.add_argument('--version', action='version', version=__version__)
  return parser

def load_edgelist(gfname):
	import pandas as pd
	try:
		edglst = pd.read_csv(gfname, comment='%', delimiter='\s+')
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
  parser = get_parser()
  args = vars(parser.parse_args())
  
  cindy_graphs = args['cgs'][0]
  fname = args['orig'][0]
  
  print 'Args:'
  print fname
  print cindy_graphs
  
  
  G = load_edgelist(fname)
  sg_gen = rwr_sample(G, 1, 5000)
  for sg in sg_gen:
    sg.name = os.path.basename(fname)
    print nx.info(sg)
  #~~#
  #~~# K HRG graphs compared against (<|>) a 5K node subgraph sampled from the reference graph
  #~~#
  s_graphs = glob ("results/sampled_graphs/"+ cindy_graphs +"/*")
  hStars = [nx.read_edgelist(f) for f in s_graphs]

  ## metrics
  import datetime
  ts = datetime.datetime.now().strftime("%d%b%y-%H%M")
  metricx = ['degree', 'hops', 'clust', 'assort', 'kcore', 'eigen', 'gcd']
  metricx = ['degree', 'hops', 'clust', 'gcd']
  metrics.network_properties([sg], metricx, hStars, name=os.path.basename(fname)+"_5k"+ts, out_tsv=True)

