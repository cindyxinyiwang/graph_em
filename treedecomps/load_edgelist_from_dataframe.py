import pandas as pd
import numpy as np

def Pandas_DataFrame_From_Edgelist(dataset_files_path):
  dataframes = []

  for input_graph in dataset_files_path:
    dat = []
    with open(input_graph) as ifile:
      for line in ifile:
        if not ("%" or "#" in line):
          lparts = line.rstrip('\r\n').split()
          if len(lparts)>=2:
            dat.append(lparts)
    print 'shape', np.shape(dat)
		if np.shape(dat)[1] == 4:
        df = pd.DataFrame(dat, columns=['src', 'trg','w', 'ts'], dtype=np.int64)
    elif np.shape(dat)[1]==3:
        df = pd.DataFrame(dat, columns=['src', 'trg','ts'], dtype=np.int64)
    else:
        df = pd.DataFrame(dat, columns=['src', 'trg'], dtype=np.int64)
    if 0: print '... dropping duplicates'
    df = df.drop_duplicates()
    #if 0: print '  sorting by ts ...'
    #df.sort_values(by=['ts'], inplace=True)
    #df = df[df['ts']>0]
    dataframes.append(df)

  return dataframes
