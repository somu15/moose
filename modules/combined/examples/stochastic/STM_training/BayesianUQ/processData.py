# %%
# Imports
%cd /Users/dhulls/projects/moose/modules/combined/examples/stochastic/STM_training/BayesianUQ
import numpy as np
import json
from matplotlib import pyplot as plt
import pandas as pd
from scipy.stats.distributions import norm, uniform
import seaborn as sns
from autocorr import int_tim_chains, flat_samples, integrated_time, int_tim_chains_2d
import math
import matplotlib as mpl
mpl.rcParams["axes.labelsize"] = 12
mpl.rcParams['axes.linewidth'] = 1.5
plt.rc('font', family='serif', size=12)
plt.rc('xtick', labelsize=12)
plt.rc('ytick', labelsize=12)
plt.rc('legend', fontsize=12)

# %%
# Function definitions
def outlier_metric(exp,out,var):
  steps,n_exp,chains = out.shape
  metric = np.zeros(chains)
  for ii in range(chains):
    metric[ii] = np.mean(np.sum(np.log(norm(out[:,:,ii],np.sqrt(np.repeat(var[:,ii],n_exp).reshape((steps,n_exp)))).pdf(exp)),axis=1))
  return metric

def processMCMCjson(exp_req, file, steps, dim, prop, N_exp, var_flag, burn_in):
    f = open(file,)
    data = json.load(f)
    inputs = np.zeros((steps*prop,dim))
    outputs = np.zeros((steps, N_exp,prop))
    variance = np.zeros(steps*prop)
    c1 = 0
    for ii in np.arange(1,(steps+1),1):
        for jj in range(prop):
            inputs[c1,:] = np.array(data["time_steps"][ii]['mcmc_reporter']['inputs'])[jj].reshape(dim)
            variance[c1] = np.array(data["time_steps"][ii]['mcmc_reporter']['variance'])[jj]
            c1 = c1 + 1
        outputs[ii-1, :, :] = np.array(data["time_steps"][ii]['mcmc_reporter']['outputs_required']).reshape(N_exp,prop)

    input_chains = np.empty((steps, prop, dim))
    variance_chains = np.empty((steps, prop))
    for ii in np.arange(1,(steps+1),1):
        input_chains[ii-1,:,:] = np.array(data["time_steps"][ii]['mcmc_reporter']['inputs'])
        variance_chains[ii-1,:] = np.array(data["time_steps"][ii]['mcmc_reporter']['variance'])
    
    k1 = outlier_metric(exp_req, outputs, variance_chains)
    q1 = np.quantile(k1,0.25)
    q3 = np.quantile(k1,0.75)
    iqr = q3-q1
    val1 = q1-1.5*iqr
    val2 = q3+1.5*iqr
    good_chains = (k1<val1) | (k1>val2)
    ind_bad = np.where(good_chains==True)[0]

    input_chains1 = np.delete(input_chains, np.unique(ind_bad).astype(int), 1)
    variance_chains1 = np.delete(variance_chains, np.unique(ind_bad).astype(int), 1)
    output_chains1 = np.delete(outputs, np.unique(ind_bad).astype(int), 2)

    int_time = int_tim_chains(input_chains1)
    int_time_var = int_tim_chains_2d(variance_chains1)
    if var_flag:
        avg_int = np.mean(np.concatenate((int_time, int_time_var)))
    else:
       avg_int = np.mean(int_time)
    i1, ind1 = flat_samples(input_chains1, discard=burn_in, thin=int(avg_int/2))
    o1 = output_chains1[ind1,:,:]
    if var_flag:
        sh = variance_chains1.shape
        i2, ind2 = flat_samples(variance_chains1.reshape((sh[0], sh[1], 1)), discard=burn_in, thin=int(avg_int/2))
    else:
       i2 = np.zeros(1)
    return i1, i2, o1
# %%
# User-specified information
# Experimental values file
exp_req = pd.read_csv('/Users/dhulls/projects/moose/modules/combined/examples/stochastic/STM_training/BayesianUQ/logexp_Stokes.csv',header=None).to_numpy().squeeze()
# The output json file
file  = '/Users/dhulls/projects/moose/modules/combined/examples/stochastic/STM_training/BayesianUQ/gold/stokes_noNoise.json'
# Number of MCMC steps
steps = 400
# Number of model parameters calibrated
dim = 2
# Number of MCMC parallel proposals
prop = 5
N_exp = len(exp_req)
burn_in = 250

# %%
# Plotting
model_params_noNoise, var_noNoise, model_outputs_noNoise = processMCMCjson(exp_req, file, steps, dim, prop, N_exp, False, burn_in)
mean_pred_noNoise = np.zeros(N_exp)
for ii in range(N_exp):
  mean_pred_noNoise[ii] = np.mean(model_outputs_noNoise[:,ii,:])

plt.figure(figsize=(6, 6))
sns.kdeplot(uniform(0.5,1.0).rvs(model_params.shape[0]),fill=True,label='Prior')
sns.kdeplot(model_params_noNoise[:,0],fill=True,label='Posterior')
plt.axvline(x=1.0, color='k', linewidth=2)
plt.xlabel('Velocity 1')
plt.legend(frameon=False)

plt.figure(figsize=(6, 6))
sns.kdeplot(uniform(1.0,3.0).rvs(model_params.shape[0]),fill=True,label='Prior')
sns.kdeplot(model_params_noNoise[:,1],fill=True,label='Posterior')
plt.axvline(x=2.5, color='k', linewidth=2)
plt.xlabel('Velocity 2')
plt.legend(frameon=False)

plt.figure(figsize=(6, 6))
sns.kdeplot(np.sqrt(uniform(0.0,0.5).rvs(model_params.shape[0])),fill=True,label='Prior')
sns.kdeplot(np.sqrt(var[:,0]),fill=True,label='Posterior')
plt.axvline(x=0.1, color='k', linewidth=2)
plt.xlabel('Noise')
plt.legend(frameon=False)

