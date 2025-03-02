[StochasticTools]
[]

[Distributions]
  [rho]
    type = Uniform
    lower_bound = 0.5
    upper_bound = 1.5
  []
  [mu]
    type = Uniform
    lower_bound = 0.002
    upper_bound = 0.05
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
    file_name = 'logexp_Stokes_noise_0_2.csv'
    log_likelihood = true
  []
[]

[Samplers]
  [sample]
    type = AffineInvariantDES
    prior_distributions = 'rho mu'
    num_parallel_proposals = 5
    file_name = 'confg.csv'
    num_columns = 2
    initial_values = '0.7 0.003'
    execute_on = PRE_MULTIAPP_SETUP
    seed = 2547
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
    param_names = 'rho1 mu1 PP_x PP_y'
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
  file_base = 'stokes_noise'
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]

