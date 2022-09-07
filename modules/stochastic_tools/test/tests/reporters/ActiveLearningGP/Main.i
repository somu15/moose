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
  [Tinf_dist]
    type = Uniform
    lower_bound = 250
    upper_bound = 350
  []
#   [k_dist]
#     type = Uniform
#     lower_bound = 1
#     upper_bound = 10
#   []
#   [q_dist]
#     type = Uniform
#     lower_bound = 9000
#     upper_bound = 11000
#   []
[]

[Samplers]
  [mc]
    type = ActiveLearningMonteCarloSampler
    num_rows = 2 # 1
    distributions = 'k_dist q_dist Tinf_dist' # 'k_dist q_dist' # L_dist 
    flag_sample = 'conditional/flag_sample'
    seed = 5
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    sampler = mc
    input_files = 'Sub.i'
    mode = batch-reset
    should_run_reporter = conditional/need_sample
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [sub]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = mc
    parameters = 'Materials/conductivity/prop_values Kernels/source/value BCs/right/value' # Mesh/xmax 
    to_control = 'stochastic'
    check_multiapp_execute_on = false
  []
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'avg/value'
    stochastic_reporter = 'conditional'
    from_multi_app = sub
    sampler = mc
  []
[]

# [Controls]
#     [cmdline]
#         type = MultiAppSamplerControl
#         multi_app = sub
#         sampler = mc
#         param_names = 'Materials/conductivity/prop_values Kernels/source/value'
#     []
# []

[Reporters]
  [conditional]
    type =  ActiveLearningGPDecision
    sampler = mc
    parallel_type = ROOT
    execute_on = 'initial timestep_begin'
    flag_sample = 'flag_sample'
    inputs = 'inputs'
    gp_mean = 'gp_mean'
    gp_std = 'gp_std'
    n_train = 6
    al_gp = GP_al_trainer
    # gp_evaluator = GP_avg
  []
[]

[Trainers]
  [GP_al_trainer]
    type = ActiveLearningGaussianProcess
    # execute_on = timestep_end
    covariance_function = 'covar'
    standardize_params = 'true'
    standardize_data = 'true'
    tune_parameters = 'signal_variance length_factor'
    tuning_algorithm = 'adam'
    iter_adam = 1000
    batch_size = 20
    learning_rate_adam = 0.005
  []
[]

# [Surrogates]
#   [GP_avg]
#     type = GaussianProcess
#     trainer = GP_al_trainer
#   []
# []

[Covariance]
  [covar]
    type= SquaredExponentialCovariance # MaternHalfIntCovariance #
    # p = 1.0
    signal_variance = 1.0
    noise_variance = 1e-4
    length_factor = '1.0 1.0 1.0'     # 1.0
  []
[]

[Executioner]
  type = Transient
  num_steps = 200 # 12 # 6 # 
[]

[Outputs]
  # execute_on = timestep_end
  perf_graph = true
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]
