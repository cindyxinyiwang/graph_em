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
# import HRG
# import treedecomps.PHRG as PHRG
# import product
import pprint as pp
# from treedecomps.salPHRG import grow_graphs_using_krongen

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
    print
    print 'Scaled-Normalized Degree Dist CDF', label
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

	# Gtrain = nx.read_edgelist(fn_train, comments="%", data=(('wt', int),('ts',int),))
	#Gtrain = nx.read_edgelist(fn_train, comments="%", data=(('ts', int),))
	#Gtrain = nx.read_edgelist(fn_train)
	Gtrain.name = [x for x in os.path.basename(edgelists[0]).split('.') if len(x)>3][0]

	df = Pandas_DataFrame_From_Edgelist([fn_test])
	Gtst = nx.from_pandas_dataframe(df[0], 'src', 'trg')
	# Gtst = nx.read_edgelist(fn_test, comments="%", data=(('ts',int),))
	# Gtst = nx.read_edgelist(fn_test, comments="%", data=(('weight',int),('ts',int),))
	# Gtst = nx.read_edgelist(fn_test)
	Gtst.name = [x for x in os.path.basename(edgelists[0]).split('.') if len(x)>3][0]

	chunglu_GM = []
	kronprd_GM = []

	# --<
	# --< chung lu graph >--
	# --<
	z = Gtrain.degree().values()
	for i in range(0,50):
		cl_graph = nx.expected_degree_graph(z)
		chunglu_GM.append(cl_graph)
	# --<
	# --< Kronecker product graph >--
	# --<
	from os import environ
	tsvGraphName = "/tmp/{}kpgraph.tsv".format(Gtst.name)
	tmpGraphName = "/tmp/{}kpgraph.tmp".format(Gtst.name)
	k = int(math.log(Gtrain.number_of_nodes(),2))+1 # Nbr of Iterations

	P = kronfit(Gtrain)
	for i in range(0,50):
			# KPG = product.kronecker_random_graph(k, P)
		M = '-m:"{} {}; {} {}"'.format(P[0][0], P[0][1], P[1][0], P[1][1])
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
		kp_graph = KPG
		kronprd_GM.append(kp_graph)

	# # --< metrics
	metricx = ['gcd']
	metrics.network_properties( [Gtst], metricx, chunglu_GM, name="clgm_"+Gtst.name, out_tsv=True)
	metrics.network_properties( [Gtst], metricx, kronprd_GM, name="kpgm_"+Gtst.name, out_tsv=True)

	# --< Degree Distance (CDF)
	degree_KS_tst_chlu= SNDegree_CDF_ks_2samp	(chunglu_GM, Gtst,	'chlu')
	degree_KS_tst_kron= SNDegree_CDF_ks_2samp	(kronprd_GM, Gtst,	'kron')
	print "CHLU"
	dfc = pd.DataFrame(degree_KS_tst_chlu)
	print dfc.to_string()
	print "KRON"
	dfc = pd.DataFrame(degree_KS_tst_kron)
	print dfc.to_string()



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
