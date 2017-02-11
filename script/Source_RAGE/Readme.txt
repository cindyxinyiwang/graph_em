RAGE - A Rapid Graphlet Enumerator Tool
--------------------------------------------------------------
Usage (command line):
RAGE [edge-list-graph.txt]

edge-list-graph.txt - A text file in the form of an edge list, [NODE_A] [NODE_B] \n
e.g:
1 2
2 3
3 4
1 3
....

Optional file for classification: 
NodeTypesList.txt - OPTIONAL text file in the form [NODE_ID] [SHOT_NAME (NO SPACES!)] [classification_ID]\n e.g.:
1 tel_aviv_univ EDU
2 jerusalem_univ EDU
3 at&t LTP
...

Results will appear in the folder "results" with the following id for each graphlet:

ID: 78_0 Three node path, node with degree 1
ID: 78,1 Three node path, node with degree 2
ID: 238_0 Triangle
ID: 4698_0 Four node Path, node with degree 2
ID: 4698_1 Four node Path, node with degree 1
ID: 13260_0 Cycle
ID: 4958_0 Tailed Triangle, node with degree 3
ID: 4958_1 Tailed Triangle, node with degree 2
ID: 4958_2 Tailed Triangle, node with degree 1
ID: 13278_0 Chordal Cycle, node with degree 3 (node touching the chord).
ID: 13278_1 Chordal Cycle, node with degree 2 (node not touching the chord).
ID: 4382_0 Four Node Star (or Claw) node with degree 3
ID: 4382_1 Four Node Star (or Claw) node with degree 1
ID: 31710_0 Clique

For more info and citation instruction see: http://www.eng.tau.ac.il/~shavitt/motifs.html