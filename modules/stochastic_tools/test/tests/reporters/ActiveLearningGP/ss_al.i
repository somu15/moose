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
[]

[Samplers]
  [sample]
    type = SubsetSimulationActiveLearning
    distributions = 'k_dist q_dist Tinf_dist'
    num_samplessub = 1000
    num_subsets = 2
    # num_parallel_chains = 15
    output_reporter = 'conditional/gp_mean'
    inputs_reporter = 'adaptive_MC/inputs'
    use_absolute_value = true
    flag_sample = 'conditional/flag_sample'
    seed = 1012
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    sampler = sample
    input_files = 'sub.i'
    mode = batch-reset
    should_run_reporter = conditional/need_sample
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [param]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sample
    parameters = 'Materials/conductivity/prop_values Kernels/source/value BCs/right/value'
    check_multiapp_execute_on = false
  []
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'avg/value'
    stochastic_reporter = 'conditional'
    from_multi_app = sub
    sampler = sample
  []
[]

[Reporters]
  [conditional]
    type = ActiveLearningGPDecision
    sampler = sample
    parallel_type = ROOT
    execute_on = 'initial timestep_begin'
    flag_sample = 'flag_sample'
    inputs = 'inputs'
    gp_mean = 'gp_mean'
    gp_std = 'gp_std'
    n_train = 20
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
    learning_function='DynamicUfunction'
    learning_function_parameter = 349.345
    learning_function_threshold = 2.0
  []
  [adaptive_MC]
    type = AdaptiveMonteCarloDecision
    output_value = conditional/gp_mean
    inputs = 'inputs'
    sampler = sample
    gp_decision = conditional
  []
[]

[Trainers]
  [GP_al_trainer]
    type = ActiveLearningGaussianProcess
    covariance_function = 'covar'
    standardize_params = 'true'
    standardize_data = 'true'
    tune_parameters = 'signal_variance length_factor'
    tuning_algorithm = 'adam'
    iter_adam = 2000
    learning_rate_adam = 0.001
    show_optimization_details = true
    # batch_size = 100
  []
[]

[Surrogates]
  [GP_eval]
    type = GaussianProcess
    trainer = GP_al_trainer
  []
[]

[Covariance]
  [covar]
    type = SquaredExponentialCovariance
    signal_variance = 1.0
    noise_variance = 1e-8
    length_factor = '1.0 1.0 1.0'
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  file_base = 'ss_al_new'
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
