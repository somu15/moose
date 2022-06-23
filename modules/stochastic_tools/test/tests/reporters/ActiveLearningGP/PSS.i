[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Uniform
    lower_bound = 5
    upper_bound = 20
  []
  [q_dist]
    type = Uniform
    lower_bound = 7000
    upper_bound = 13000
  []
  #[L_dist]
  #  type = Uniform
  #  lower_bound = 0.05
  #  upper_bound = 0.15
  #[]
  [Tinf_dist]
    type = Uniform
    lower_bound = 250
    upper_bound = 350
  []
[]

[Samplers]
  [sample]
    type = ParallelSubsetSimulation
    distributions = 'k_dist q_dist Tinf_dist' # L_dist 
    num_samplessub = 4000
    num_parallel_chains = 4
    output_reporter = 'constant/reporter_transfer:avg:value'
    inputs_reporter = 'adaptive_MC/inputs'
    seed = 1012
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = Sub4D.i
    sampler = sample
  []
[]

[Transfers]
  [param]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sample
    parameters = 'Materials/conductivity/prop_values Kernels/source/value BCs/right/value' # Mesh/xmax 
    to_control = 'stochastic'
  []
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'avg/value'
    stochastic_reporter = 'constant'
    from_multi_app = sub
    sampler = sample
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
    outputs = none
  []
  [adaptive_MC]
    type = AdaptiveMonteCarloDecision
    output_value = constant/reporter_transfer:avg:value
    inputs = 'inputs'
    sampler = sample
  []
[]

[Executioner]
  type = Transient
  num_steps = 2000
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]