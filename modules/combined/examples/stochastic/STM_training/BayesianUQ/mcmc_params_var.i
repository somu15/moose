[StochasticTools]
[]

[Distributions]
  [vel1]
    type = Uniform
    lower_bound = 0.5
    upper_bound = 1.5
  []
  [vel2]
    type = Uniform
    lower_bound = 1.0
    upper_bound = 4.0
  []
  [variance]
    type = Uniform
    lower_bound = 0.0
    upper_bound = 0.5
  []
[]

[Likelihood]
  [gaussian]
    type = Gaussian
    noise = 'mcmc_reporter/noise'
    file_name = 'logexp_Stokes_noise_0_1.csv'
    log_likelihood = true
  []
[]

[Samplers]
  [sample]
    type = AffineInvariantDES
    prior_distributions = 'vel1 vel2'
    num_parallel_proposals = 5
    file_name = 'confg.csv'
    num_columns = 2
    initial_values = '0.7 3.5'
    lower_bound = '0.5 1.0'
    upper_bound = '1.5 4.0'
    scales = '1.0 1.0'
    execute_on = PRE_MULTIAPP_SETUP
    seed = 100
    previous_state = 'mcmc_reporter/inputs'
    previous_state_var = 'mcmc_reporter/variance'
    prior_variance = 'variance'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = Stokes.i
    sampler = sample
  []
[]

[Transfers]
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'log_resultant_velocity/value'
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
    param_names = 'param1 param4 PP_x PP_y'
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [mcmc_reporter]
    type = AffineInvariantDifferentialDecision
    output_value = constant/reporter_transfer:log_resultant_velocity:value
    sampler = sample
    likelihoods = 'gaussian'
  []
[]

[Executioner]
  type = Transient
  num_steps = 100
[]

[Outputs]
  file_base = 'stokes_noise_0_1'
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]

