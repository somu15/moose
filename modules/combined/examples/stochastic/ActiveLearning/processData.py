# %%
## Imports
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
import math
import matplotlib as mpl
mpl.rcParams["axes.labelsize"] = 12
mpl.rcParams['axes.linewidth'] = 1.5
plt.rc('font', family='serif', size=12)
plt.rc('xtick', labelsize=12)
plt.rc('ytick', labelsize=12)
plt.rc('legend', fontsize=12)

# %%
# Gaussian process Monte Carlo training
gp_mc = pd.read_csv('/Users/dhulls/projects/moose/modules/combined/examples/stochastic/GaussianProcess/test_out_samp_avg_0002.csv').to_numpy().squeeze()
# Gaussian process active learning training
gp_al = pd.read_csv('/Users/dhulls/projects/moose/modules/combined/examples/stochastic/ActiveLearning/AL_test_out_samp_avg_0002.csv').to_numpy().squeeze()
# True simulation values
true = pd.read_csv('/Users/dhulls/projects/moose/modules/combined/examples/stochastic/ActiveLearning/true_out_resultant_velocity5_0001.csv').to_numpy().squeeze()

plt.figure(figsize=(6, 6))
plt.scatter(true,gp_mc[:,0],label='Monte Carlo')
plt.scatter(true,gp_al[:,0],label='Active learning')
plt.plot([0.0,2.2],[0.0,2.2],color='k',linewidth=)
plt.xlabel('True velocity')
plt.ylabel('Gaussian process prediction')
plt.legend(frameon=False)
