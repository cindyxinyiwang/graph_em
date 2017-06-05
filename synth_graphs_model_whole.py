__author__ = 'tweninge,saguinag'
__version__="0.1.0"
import subprocess, shlex
from threading import Timer
import math
import os
import networkx as nx
import numpy as np
import pandas as pd
import argparse
import treedecomps.net_metrics as metrics
import treedecomps.graph_utils as gutil
from	 collections import defaultdict
import prepare_tree_rules.graph_sampler as gs
import pprint as pp
#from resource import setrlimit
import resource 

resource.setrlimit(resource.RLIMIT_STACK, (resource.RLIM_INFINITY, resource.RLIM_INFINITY))

# from treedecomps.salPHRG import grow_graphs_using_krongen
DEBUG = False

def get_parser ():
	parser = argparse.ArgumentParser(description='Synth Graphs')
	parser.add_argument('--train', required=True, nargs=1,help='Filename edgelist graph')
	parser.add_argument('--test', required=True, nargs=1,help='Filename edgelist graph')
	parser.add_argument('--version', action='version', version=__version__)
	return parser


def run(cmd, timeout_sec):
	proc = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE,
							stderr=subprocess.PIPE)
	kill_proc = lambda p: p.kill()
	timer = Timer(timeout_sec, kill_proc, [proc])
	try:
		timer.start()
		stdout, stderr = proc.communicate()
	finally:
		timer.cancel()

def run(cmd, timeout_sec):
	proc = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE,
							stderr=subprocess.PIPE)
	kill_proc = lambda p: p.kill()
	timer = Timer(timeout_sec, kill_proc, [proc])
	try:
		timer.start()
		stdout, stderr = proc.communicate()
	finally:
		timer.cancel()

def scale_norm_dist(gb):
    # Scaled
    gb['Sg'] = gb.apply(lambda x: x[0]/float(x.name), axis=1)
    # gb['Sg'] = gb.apply(lambda x: x[1]/float(x[0]), axis=1)
    # Total
    Tg = gb['Sg'].sum()
    # Normalized
    gb['Ng'] = gb['Sg']/float(Tg)
    #orig_sn_deg_dist= gb['Ng'].values
    results = gb['Ng'].values
    return results

def ecdf(x):
  xs = np.sort(x)
  ys = np.arange(1, len(xs)+1)/float(len(xs))
  return xs, ys

def SNDegree_CDF_ks_2samp(glist, origG, label):
    from scipy.stats import ks_2samp
    # print
    # print 'Scaled-Normalized Degree Dist CDF', label
    kdeg = nx.degree_histogram(origG)[1:]
    df = pd.DataFrame.from_dict(origG.degree().items())
    gb = df.groupby([1]).count()
    ## probability gb['p'] = gb.apply(lambda x: x[0]/float(origG.number_of_nodes()), axis=1)
    # print gb
    # exit()
    origG_sndd = scale_norm_dist(gb)
    # origG_sndd2 = gb.apply(lambda x: x[0]/float(origG.number_of_nodes()), axis=1)
    xs0, ys0 = ecdf(origG_sndd)
    results = []
    for g in glist:
        df = pd.DataFrame.from_dict(g.degree().items())
        gb = df.groupby([1]).count()
        # gb['p'] = gb.apply(lambda x: x[0]/float(g.number_of_nodes()), axis=1)
        # print gb.head()
        # exit()
        hstar_sndd = scale_norm_dist(gb)
        x1, ys2 = ecdf(hstar_sndd)
        # if we ploat x0,ys0 and x1,ys2 ECDF we can see where they
        # diverge most will be shown in D (or the ks_2sample) statistic
        # print origG_sndd[:4]
        # print hstar_sndd[:4]
        # hstar_sndd = gb.apply(lambda x: x[0]/float(g.number_of_nodes()), axis=1)
        # print origG_sndd2[:4]
        # print hstar_sndd[:4]
        # exit()
        D,p = ks_2samp(origG_sndd, hstar_sndd)
        # print "  {}, {}".format(D, p)
        results.append([D,p])

    return results
# def kronfit(G):
#   """
#
#   Notes:
#   23May16: changes to handle kronfit
#   """
#
#   from os import environ
#
#   with open('tmp.txt', 'w') as tmp:
#	   for e in G.edges():
#		   tmp.write(str(e[0]) + ' ' + str(e[1]) + '\n')
#
#   if environ['HOME'] == '/home/saguinag':
#	 args = ("/home/saguinag/Software/Snap-3.0/examples/kronfit/kronfit", "-i:tmp.txt","-n0:2", "-m:\"0.9 0.6; 0.6 0.1\"", "-gi:5")
#   elif environ['HOME'] == '/Users/saguinag':
#	 args = ("/Users/saguinag/Theory/MacOSTools/Snap-3.0/examples/kronfit/kronfit", "-i:tmp.txt","-n0:2", "-m:\"0.9 0.6; 0.6 0.1\"", "-gi:5")
#   else:
#	 args = ('./kronfit.exe -i:tmp.txt -n0:2 -m:"0.9 0.6; 0.6 0.1" -gi:5')
#   #print (args)
#   #args = args.split()
#   #options = {k: True if v.startswith('-') else v
#   #	   for k,v in zip(args, args[1:]+["--"]) if k.startswith('-')}
#
#   #proc = subprocess.Popen(shlex.split(args), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
#   """
#   proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
#   kill_proc = lambda p: p.kill()
#   timer = Timer(10, kill_proc, [proc])
#   try:
#	   timer.start()
#	   output, stderr = proc.communicate()
#   finally:
#	   timer.cancel()
#
#   print "out"
#   """
#
#   kronout = ""
#   while not kronout:
#	 popen = subprocess.Popen(args, stdout=subprocess.PIPE)
#	 popen.wait()
#	 output = popen.stdout.read()
#
#	 top = []
#	 bottom = []
#	 kronout = output.split('\n')


def kronfit(G):
  """

  Notes:
  23May16: changes to handle kronfit
  """

  from os import environ

  with open('tmp.txt', 'w') as tmp:
	  for e in G.edges():
		  tmp.write(str(e[0]) + ' ' + str(e[1]) + '\n')

  if environ['HOME'] == '/home/saguinag':
	args = ("/home/saguinag/Software/Snap-3.0/examples/kronfit/kronfit", "-i:tmp.txt","-n0:2", "-m:\"0.9 0.6; 0.6 0.1\"", "-gi:5")
  elif environ['HOME'] == '/Users/saguinag':
	args = ("/Users/saguinag/Theory/MacOSTools/Snap-3.0/examples/kronfit/kronfit", "-i:tmp.txt","-n0:2", "-m:\"0.9 0.6; 0.6 0.1\"", "-gi:5")
  else:
	args = ('./kronfit.exe -i:tmp.txt -n0:2 -m:"0.9 0.6; 0.6 0.1" -gi:5')
  #print (args)
  #args = args.split()
  #options = {k: True if v.startswith('-') else v
  #	   for k,v in zip(args, args[1:]+["--"]) if k.startswith('-')}

  #proc = subprocess.Popen(shlex.split(args), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  """
  proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  kill_proc = lambda p: p.kill()
  timer = Timer(10, kill_proc, [proc])
  try:
	  timer.start()
	  output, stderr = proc.communicate()
  finally:
	  timer.cancel()

  print "out"
  """

  kronout = ""
  while not kronout:
	popen = subprocess.Popen(args, stdout=subprocess.PIPE)
	popen.wait()
	output = popen.stdout.read()

	top = []
	bottom = []
	kronout = output.split('\n')

  for i in range(0, len(kronout)):
	  if kronout[i].startswith("FITTED PARAMS"):
		  top = kronout[i + 1].split()
		  top = [float(j) for j in top]
		  bottom = kronout[i + 2].split()
		  bottom = [float(j) for j in bottom]
		  break

  if not len(top):
	print 'top:',top,'bottom',bottom
	top = [0,0]
	bottom = [0,0]

  top[1] = bottom[0] = (top[1] + bottom[0]) / 2  # make symmetric by taking average
  fitted = [top, bottom]
  return fitted


def gcd():
	num_nodes = 1000

	ba_G = nx.barabasi_albert_graph(num_nodes, 3)
	er_G = nx.erdos_renyi_graph(num_nodes, .1)
	ws_G = nx.watts_strogatz_graph(num_nodes, 8, .1)
	nws_G = nx.newman_watts_strogatz_graph(num_nodes, 8, .1)

	graphs = [ba_G, er_G, ws_G, nws_G]

	samples = 50

	for G in graphs:
		chunglu_M = []
		for i in range(0, samples):
			chunglu_M.append(nx.expected_degree_graph(G.degree()))

		HRG_M, degree = HRG.stochastic_hrg(G, samples)
		pHRG_M = PHRG.probabilistic_hrg(G, samples)
		kron_M = []
		rmat_M = []
		for i in range(0, samples):
			P = kronfit(G)
			k = math.log(num_nodes, 2)
			kron_M.append(product.kronecker_random_graph(int(math.floor(k)), P, directed=False))

		df_g = metrics.external_rage(G)
		gcd_chunglu = []
		gcd_phrg = []
		gcd_hrg = []
		gcd_kron = []
		for chunglu_M_s in chunglu_M:
			df_chunglu = metrics.external_rage(chunglu_M_s)
			rgfd = metrics.tijana_eval_rgfd(df_g, df_chunglu)
			gcm_g = metrics.tijana_eval_compute_gcm(df_g)
			gcm_h = metrics.tijana_eval_compute_gcm(df_chunglu)
			gcd_chunglu.append(metrics.tijana_eval_compute_gcd(gcm_g, gcm_h))
		for HRG_M_s in HRG_M:
			df_hrg = metrics.external_rage(HRG_M_s)
			rgfd = metrics.tijana_eval_rgfd(df_g, df_hrg)
			gcm_g = metrics.tijana_eval_compute_gcm(df_g)
			gcm_h = metrics.tijana_eval_compute_gcm(df_hrg)
			gcd_hrg.append(metrics.tijana_eval_compute_gcd(gcm_g, gcm_h))
		for pHRG_M_s in pHRG_M:
			df_phrg = metrics.external_rage(pHRG_M_s)
			rgfd = metrics.tijana_eval_rgfd(df_g, df_phrg)
			gcm_g = metrics.tijana_eval_compute_gcm(df_g)
			gcm_h = metrics.tijana_eval_compute_gcm(df_phrg)
			gcd_phrg.append(metrics.tijana_eval_compute_gcd(gcm_g, gcm_h))
		for kron_M_s in kron_M:
			df_kron = metrics.external_rage(kron_M_s)
			rgfd = metrics.tijana_eval_rgfd(df_g, df_kron)
			gcm_g = metrics.tijana_eval_compute_gcm(df_g)
			gcm_h = metrics.tijana_eval_compute_gcm(df_kron)
			gcd_kron.append(metrics.tijana_eval_compute_gcd(gcm_g, gcm_h))

		print gcd_chunglu
		print gcd_hrg
		print gcd_phrg
		print gcd_kron
		print
		print

def synth_plots():
	num_nodes = 100
	samples = 5

	chunglu_M = []
	kron_M = []
	HRG_M = []
	pHRG_M = []
	G_M = []

	for i in range(0,samples):
		##BA Graph
		G = nx.erdos_renyi_graph(num_nodes, .1)
		G_M.append(G)

		for i in range(0, samples):
			chunglu_M.append(nx.expected_degree_graph(G.degree().values()))

		HRG_M_s, degree = HRG.stochastic_hrg(G, samples)
		HRG_M = HRG_M + HRG_M_s
		pHRG_M_s = PHRG.probabilistic_hrg(G, samples)
		pHRG_M = pHRG_M + pHRG_M_s
		for i in range(0, samples):
			P = kronfit(G)
			k = math.log(num_nodes, 2)
			kron_M.append(product.kronecker_random_graph(int(math.floor(k)), P, directed=False))

	metrics.draw_network_value(G_M, chunglu_M, HRG_M, pHRG_M, kron_M)

def sample_refrence_graph(G, ofname, scate, K_lst, n):
	if G is None: return

	G.remove_edges_from(G.selfloop_edges())
	giant_nodes = max(nx.connected_component_subgraphs(G), key=len)
	G = nx.subgraph(G, giant_nodes)

	if n is None:
			num_nodes = G.number_of_nodes()
	else:
			num_nodes = n

	gutil.graph_checks(G)

	if DEBUG: print
	if DEBUG: print "--------------------"
	if DEBUG: print "-Tree Decomposition-"
	if DEBUG: print "--------------------"
	prod_rules = {}
	left_deriv_prod_rules = []

	cntr = 1
	subgraph_objs_lst = []

	# subgraphs 50 to
	left_deriv_file_name = "ProdRules/{}_{}_{}_prodrules.txt".format(K_lst[0],scate,ofname)
	out_edgelist_fname = "data/{}_{}_{}_el.tsv".format(ofname,scate,K_lst[0])

	cntr += 1
	gprime_group =[]
	# print K_lst[0], n
	for Gprime in gs.rwr_sample(G, K_lst[0], n):
		if 0: nx.write_edgelist(Gprime, out_edgelist_fname, delimiter='\t')
		gprime_group.append(Gprime)
		# exit()
		# T = td.quickbb(Gprime)
		# root = list(T)[0]
		# T = td.make_rooted(T, root)
		# T = phrg.binarize(T)
		# root = list(T)[0]
		# root, children = T
		# td.new_visit(T, G, prod_rules, left_deriv_prod_rules)
	# subgraph_objs_dict[(scate,K)].append(gprime_group)
		# left_derive_file = open(left_deriv_file_name, 'w')
		# for r in left_deriv_prod_rules:
		# 	left_derive_file.write(r)
		# 	left_derive_file.write('\n')
		# left_derive_file.close()


	return gprime_group

def chunglu_subgraph_chunks(results_tup, gTrain, gTest, nbr_of_synth_g):
	z = gTrain.degree().values()
	cl_graph = nx.expected_degree_graph(z)
	tr_cl_graphs_lst =[]
	ts_cl_graphs_lst =[]
	tr_synth_grph = sample_refrence_graph(cl_graph, gTrain.name, "train",[nbr_of_synth_g], n=25)
	ts_synth_grph = sample_refrence_graph(cl_graph, gTrain.name, "test", [nbr_of_synth_g], n=25)
	for k in tr_synth_grph.keys():
		tr_cl_graphs_lst.append( [[j.number_of_nodes() for j in x] for x in synth_grph[k]])
	for k in ts_synth_grph.keys():
		ts_cl_graphs_lst.append( [[j.number_of_nodes() for j in x] for x in synth_grph[k]])
	results_tup= (tr_cl_graphs_lst,ts_cl_graphs_lst)
	return

def kronecker_subgraph_chunks(gTrain, nbr_of_synth_g):
	# --< get a set of 50 subgraphs using Kronecker
	from os import environ
	tsvGraphName = "/tmp/{}kpgraph.tsv".format(gTrain.name)
	tmpGraphName = "/tmp/{}kpgraph.tmp".format(gTrain.name)
	k = int(math.log(gTrain.number_of_nodes(),2))+1 # Nbr of Iterations
	P = kronfit(gTrain)
	M = '-m:"{} {}; {} {}"'.format(P[0][0], P[0][1], P[1][0], P[1][1])

	# tr_kpgraphs_lst = []
	if environ['HOME'] == '/home/saguinag':
	  args = ("/home/saguinag/Software/Snap-3.0/examples/krongen/krongen", "-o:"+tsvGraphName, M, "-i:{}".format(k))
	elif environ['HOME'] == '/Users/saguinag':
	  args = ("/Users/saguinag/Theory/MacOSTools/Snap-3.0/examples/krongen/krongen", "-o:"+tsvGraphName, M, "-i:{}".format(k))
	else:
	  args = ('./krongen.exe -o:{} '.format(tmpGraphName) +M +'-i:{}'.format(k+1))

	popen = subprocess.Popen(args, stdout=subprocess.PIPE)
	popen.wait()
	#output = popen.stdout.read()
	if os.path.exists(tsvGraphName):
	  KPG = nx.read_edgelist(tsvGraphName, nodetype=int)
	else:
	  print "!! Error, file is missing"

	for u,v in KPG.selfloop_edges():
	  KPG.remove_edge(u, v)

	# train_kron_d = sample_refrence_graph(KPG, gTrain.name, "train",[nbr_of_synth_g], 32)
	# test_kron_d  = sample_refrence_graph(KPG, gTrain.name, "test",[nbr_of_synth_g], 32)
	# tr_kp_graphs_lst =[]
	# for k in train_kron_d.keys():
	# 	tr_kp_graphs_lst.append( [[j.number_of_nodes() for j in x] for x in synth_grph[k]])
	# ts_kp_graphs_lst =[]
	# for k in test_kron_d.keys():
	# 	ts_kp_graphs_lst.append( [[j.number_of_nodes() for j in x] for x in synth_grph[k]])
	# results_tup= (tr_kp_graphs_lst,ts_kp_graphs_lst)

	return KPG
# --<
# --< main >--
# --<
if __name__ == '__main__':
	parser = get_parser()
	args	 = vars(parser.parse_args())

	edgelists = [args['train'][0], args['test'][0]]
	print edgelists

	fn_train = edgelists[0]
	fn_test  = edgelists[1]
	from treedecomps.load_edgelist_from_dataframe import Pandas_DataFrame_From_Edgelist
	df = Pandas_DataFrame_From_Edgelist([fn_train])
	Gtrain = nx.from_pandas_dataframe(df[0], 'src', 'trg')
	Gtrain.name = [x for x in os.path.basename(edgelists[0]).split('.') if len(x)>3][0]

	df = Pandas_DataFrame_From_Edgelist([fn_test])
	Gtst = nx.from_pandas_dataframe(df[0], 'src', 'trg')
	Gtst.name = [x for x in os.path.basename(edgelists[0]).split('.') if len(x)>3][0]

	chunglu_GM = []
	kronprd_GM = []
	trn_graphs = sample_refrence_graph(Gtrain, Gtrain.name, 'train', [500], 25)
	tst_graphs = sample_refrence_graph(Gtst, Gtst.name, 'test', [500], 25)
	print 'Got training and tests sets'

	z = Gtrain.degree().values()
	cl_graph = nx.expected_degree_graph(z)
	kp_graph = kronecker_subgraph_chunks(Gtrain,1)
	print 'Got Whole Graph'

	synth_cl_graphs = sample_refrence_graph(cl_graph, Gtrain.name, 'sythTrain', [500], 25)
	synth_kp_graphs = sample_refrence_graph(kp_graph, Gtrain.name, 'sythTrain', [500], 25)

	# # --< metrics
	print "ChungLu"
	metricx = ['gcd']
	print 'GCD'
	for j,synthg in enumerate(synth_cl_graphs):
		metrics.network_properties( [tst_graphs[j]], metricx, [synthg], name="clgm_"+Gtst.name, out_tsv=False)
	# --< Degree Distance (CDF)
	print '\nScaled-Normalized Degree Dist CDF'
	for j,synthg in enumerate(synth_cl_graphs):
		degree_KS_tst_chlu= SNDegree_CDF_ks_2samp([synthg], tst_graphs[j],	'chlu')
		dfc = pd.DataFrame(degree_KS_tst_chlu)
		print dfc.to_string(header=False, index=False)


	print "KRON"
	print 'GCD'
	for j, synthg in enumerate(synth_kp_graphs):
		metrics.network_properties( [tst_graphs[j]], metricx, [synthg], name="clgm_"+Gtst.name, out_tsv=False)
		# --< Degree Distance (CDF)
	print '\nScaled-Normalized Degree Dist CDF'
	for j, synthg in enumerate(synth_kp_graphs):
		degree_KS_tst_kron= SNDegree_CDF_ks_2samp([synthg], tst_graphs[j], 'kron')
		dfc = pd.DataFrame(degree_KS_tst_kron)
		print dfc.to_string(header=False, index=False)




'''
./gcd.sh data/out.sx-stackoverflow data/out.sx-superuser stack&
./gcd.sh data/out.wiki-Talk        data/out.wiki-Vote wiki&
./gcd.sh data/out.cit-HepPh        data/out.cit-HepTh cits&
./gcd.sh data/out.amazon0302 data/out.amazon0312 amaz&
./gcd.sh data/out.higgs-activity_time data/out.quotes_2009-04 twitr&
./gcd.sh data/out.as20000102  data/out.as-caida20071105 internet&
./gcd.sh data/nws_rnd_60k_exp3.tsv  data/nws_rnd_60k_exp3_test.tsv newast&
./gcd.sh data/ba_rnd_60k_exp3.tsv  data/ba_rnd_60k_exp3_test.tsv baralb&


data/wiki-Talk_train_1_el.tsv data/wiki-Vote_test_1_el.tsv
data/amazon0302_train_1_el.tsv data/amazon0312_test_1_el.tsv
data/amazon0302_train_1_el.tsv data/amazon0312_test_1_el.tsv&
data/cit-HepTh_train_1_el.tsv data/cit-HepPh_test_1_el.tsv&

'''
