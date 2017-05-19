import matplotlib
matplotlib.use('pdf')
import os 
from glob import glob
from prepare_tree_rules.graph_sampler import rwr_sample
from treedecomps.load_edgelist_from_dataframe import Pandas_DataFrame_From_Edgelist
import treedecomps.net_metrics as metrics
import networkx as nx

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

fname = "./data/out.cit-HepTh"
G = load_edgelist(fname)
sg_gen = rwr_sample(G, 1, 5000)
for sg in sg_gen:
	sg.name = os.path.basename(fname)
	print nx.info(sg)

s_graphs = glob ("results/sampled_graphs/cit-HepTh/split_1/*")
#s_graphs = s_graphs[:10]
hStars = [nx.read_edgelist(f) for f in s_graphs]

## metrics
import datetime
ts = datetime.datetime.now().strftime("%d%b%y-%H%M")
metricx = ['degree', 'hops', 'clust', 'assort', 'kcore', 'eigen', 'gcd']
metricx = ['degree', 'hops', 'clust', 'gcd']
metrics.network_properties([sg], metricx, hStars, name=os.path.basename(fname)+"_5k"+ts, out_tsv=True)

