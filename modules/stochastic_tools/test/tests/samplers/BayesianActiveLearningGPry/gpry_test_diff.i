[StochasticTools]
[]

[Distributions]
  [left]
    type = Normal
    mean = 0.0
    standard_deviation = 1.0
    # lower_bound = -1.0
    # upper_bound = 1.0
  []
  [right]
    type = Normal
    mean = 0.0
    standard_deviation = 1.0
    # lower_bound = -1.0
    # upper_bound = 1.0
  []
  [prior_variance]
    type = Uniform
    lower_bound = 0.0
    upper_bound = 0.2
  []
[]

[Likelihood]
  [gaussian]
    type = Gaussian
    noise = 'mcmc_reporter/noise'
    file_name = 'exp_0_05.csv'
    log_likelihood = true
  []
[]

[Samplers]
  [sample]
    type = AffineInvariantDES
    prior_distributions = 'left right'
    num_parallel_proposals = 50
    file_name = 'confg.csv'
    execute_on =  PRE_MULTIAPP_SETUP
    seed = 100
    initial_values = '0.5 0.5'
    previous_state = 'mcmc_reporter/inputs'
    previous_state_var = 'mcmc_reporter/variance'
    prior_variance = 'prior_variance'
    lower_bound = '-1.0 -1.0'
    upper_bound = '1.0 1.0'
    variance_bound = 0.2
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
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

[Transfers]
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'constant'
    from_multi_app = sub
    sampler = sample
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [mcmc_reporter]
    type = AffineInvariantDifferentialDecisionwithGPry
    output_value = constant/reporter_transfer:average:value
    sampler = sample
    likelihoods = 'gaussian'
    gp_evaluator = GP_eval
  []
[]

[Surrogates]
  [GP_eval]
    type = GaussianProcess
    filename = 'gpry_train_diff_out2_GP_al_trainer.rd'
  []
[]

[Executioner]
  type = Transient
  num_steps = 500
[]

[Outputs]
  execute_on = TIMESTEP_END
  file_base = 'des_gpry_diff'
  perf_graph = true
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
