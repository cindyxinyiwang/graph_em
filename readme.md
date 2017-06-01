# graph em

This project contains code to run EM algorithm for graph grammar on a tree
Here listed are the typical steps of the experiment

1. prepare tree decomposition data for training and testing graph
	work under the folder prepare_tree_rules/. put training graph and test graph files in prepare_tree_rules/data/. Then run `python grow_graph.py train_graph_name train_folder_name` and `python grow_graph_1.py test_graph_name test_folder_name` to prepare tree decomposition

2. run EM
	work under the root directory. `python loglikelihood_exper.py train_folder_name test_folder_name result_folder_name split`
	this would output the n-split grammar and experiment results into results_new/result_folder_name/ folder
	eg. `python loglikelihood_exper.py cit-HepTh cit-HepPh cit 2` extract 2-split grammar from cit-HepTh, test on cit-HepPh, and put results in a folder results_new/cit/
	***Note: You could modify the "test_nums" variable in loglikelihood_exper.py to run experiments on certain test samples only

3. generate new graphs from grammar
	work under the root directory `python genrate_graphs.py result_folder_name graph_size split` generate a graph of certain size with grammar under result_folder_name
	eg. `python generate_graphs.py cit 5000 1` generate graphs of size 5000 from citation graph grammar of split 1

4. Calculate GCD
	work under the root `python gcd.py train_graph_name result_folder_name` calculates the gcd between training graph and generated graphs
	eg. `python gcd.py cit-HepTh cit`
