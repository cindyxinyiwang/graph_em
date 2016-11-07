# graph em

This project contains code to run EM algorithm for graph grammar on a tree
`new_em.py` contains main EM code and related tree construction/reading code
`decomp_new.txt` is the tree derivation of the input graph

run "python `new_em.py`" would extract rules with nonterminals of 3 splits from the input tree. It also samples 100 graphs from the trained grammar and record the size of the graphs. 


## Grow a graph
- git clone the following project in `graph_em`: 
	`cd graph_em`
	`git clone https://github.com/nddsg/PHRG.git`
- Added the folder PHRG (which are the necessary files from the nddsg:Phoenix project) 
  This lets us use probabilistic generation to grow a graph

