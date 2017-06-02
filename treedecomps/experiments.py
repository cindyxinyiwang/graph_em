#!/usr/bin/env python
__authors__ = "saguinag"
__email__ = "{saguinag,tweninge,cpennycu} (at) nd dot edu"
__version__ = "0.1.0"

import math
import os
import pickle
from argparse import ArgumentParser

import networkx as nx
import numpy as np
import pandas as pd

#import data.graphs as graphs
#import exact_phrg as xphrg
# import kron_sal as kron
import net_metrics
#from derive_prules_givenagraph import derive_prules_from

def get_parser():
  parser = ArgumentParser(description='experiments: given an original edgeslist (orig) & a newGList*.p file in <dout>\n'+
                            'e.g., "--newglist mmoreno_vdb_vdb/newGList_100_2.p"')
  parser.add_argument('--orig', metavar='ORIGG', nargs=1, help='Path to the original (edgelist) graph')
  parser.add_argument('--newglist', nargs=1, help='Path to newGList*.p found in the <dout> folder')
  return parser

