# %%
import torch
from matplotlib import pyplot as plt
import numpy as np

# %%
samples = 500
radius = torch.distributions.uniform.Uniform(1.25e-4,1.55e-4).rsample(sample_shape=torch.Size([samples]))
power = torch.distributions.uniform.Uniform(60,75).rsample(sample_shape=torch.Size([samples]))
speed = torch.distributions.uniform.Uniform(1.0,1.5).rsample(sample_shape=torch.Size([samples]))

params = torch.zeros((samples,3))
params[:,0] = radius
params[:,1] = power
params[:,2] = speed

np.savetxt('/Users/dhulls/projects/moose/modules/combined/examples/stochastic/HPC/params_large.csv', params.numpy(), delimiter=',')
