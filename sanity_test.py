import pandas as pd
import networkx as nx
from scipy.stats import ks_2samp



g = nx.karate_club_graph()
d = g.degree()
df = pd.DataFrame.from_dict(d.items())
gb = df.groupby([1]).count()

print gb.to_string()
print gb[0], gb[0]
print ks_2samp(gb[0].values,gb[0].values)

