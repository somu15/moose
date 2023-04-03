[StochasticTools]
[]

[Distributions]
  [left]
    type = Normal
    mean = 0.0
    standard_deviation = 1.0
  []
  [right]
    type = Normal
    mean = 0.0
    standard_deviation = 1.0
  []
[]

[Likelihoods]
  [gaussian]
    type = Gaussian
    noise = 0.05
    file_name = 'exp_0_05.csv'
    log_likelihood = true
  []
[]

[Samplers]
  [sample]
    type = AffineInvariantStretchSampler
    prior_distributions = 'left right'
    num_parallel_proposals = 5
    file_name = 'confg.csv'
    execute_on = PRE_MULTIAPP_SETUP
    seed = 2547
    previous_state = 'mcmc_reporter/inputs'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
  []
[]

[Transfers]
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'constant'
    from_multi_app = sub
    sampler = sample
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'left_bc right_bc mesh1'
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [mcmc_reporter]
    type = AffineInvariantStretchDecision
    output_value = constant/reporter_transfer:average:value
    sampler = sample
    likelihoods = 'gaussian'
    prior_distributions = 'left right'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1000
[]

[Outputs]
  file_base = 'ss_5prop'
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
