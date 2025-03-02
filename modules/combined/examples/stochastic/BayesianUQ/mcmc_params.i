[StochasticTools]
[]

[Distributions]
  [vel]
    type = Uniform
    lower_bound = 0.5
    upper_bound = 1.5
  []
  [mu]
    type = Uniform
    lower_bound = 0.002
    upper_bound = 0.05
  []
[]

[Likelihood]
  [gaussian]
    type = Gaussian
    noise = 'noise_specified/noise_specified'
    file_name = 'logexp_Stokes.csv'
    log_likelihood = true
  []
[]

[Samplers]
  [sample]
    type = AffineInvariantDES
    prior_distributions = 'vel mu'
    num_parallel_proposals = 5
    file_name = 'confg.csv'
    num_columns = 2
    initial_values = '0.7 0.003'
    lower_bound = '0.5 0.002'
    upper_bound = '1.5 0.05'
    scales = '1.0 0.048'
    execute_on = PRE_MULTIAPP_SETUP
    seed = 2547
    previous_state = 'mcmc_reporter/inputs'
    previous_state_var = 'mcmc_reporter/variance'
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
    param_names = 'param1 mu1 PP_x PP_y'
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [noise_specified]
    type = ConstantReporter
    real_names = 'noise_specified'
    real_values = '0.05'
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
  file_base = 'stokes_noNoise'
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]

