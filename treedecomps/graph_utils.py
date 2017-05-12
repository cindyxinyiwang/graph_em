import networkx as nx
import sys

def graph_checks (G):
  ## Target number of nodes
  global num_nodes
  num_nodes = G.number_of_nodes()

  if not nx.is_connected(G):
    if DEBUG: print "Graph must be connected";
    os._exit(1)

  if G.number_of_selfloops() > 0:
    if DEBUG: print "Graph must be not contain self-loops";
    sys.exit(1)
