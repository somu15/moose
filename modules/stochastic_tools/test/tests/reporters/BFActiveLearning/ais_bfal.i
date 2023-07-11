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
  [mc]
    type = AISActiveLearning
    distributions = 'k_dist q_dist Tinf_dist'
    proposal_std = '1.0 1.0 1.0'
    output_limit = 350.0
    num_samples_train = 1500
    num_importance_sampling_steps = 5000
    std_factor = 0.9
    initial_values = '13.193115206257518 12504.740930938022 345.08192071763744'
    inputs_reporter = 'adaptive_MC/inputs'
    use_absolute_value = true
    flag_sample = 'conditional/flag_sample'
    seed = 9874
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub_lf]
    type = SamplerFullSolveMultiApp
    sampler = mc
    input_files = 'sub_lf.i'
  []
  [sub]
    type = SamplerFullSolveMultiApp
    sampler = mc
    input_files = 'sub.i'
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
    parameters = 'Materials/conductivity/prop_values Kernels/source/value BCs/right/value'
    to_control = 'stochastic'
    check_multiapp_execute_on = false
  []
  [sub_lf]
    type = SamplerParameterTransfer
    to_multi_app = sub_lf
    sampler = mc
    parameters = 'Materials/conductivity/prop_values Kernels/source/value BCs/right/value'
    to_control = 'stochastic'
    check_multiapp_execute_on = false
  []
  [reporter_transfer_lf]
    type = SamplerReporterTransfer
    from_reporter = 'avg/value'
    stochastic_reporter = 'constant'
    from_multi_app = sub_lf
    sampler = mc
  []
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'avg/value'
    stochastic_reporter = 'conditional'
    from_multi_app = sub
    sampler = mc
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [conditional]
    type = BFActiveLearningGPDecision
    sampler = mc
    parallel_type = ROOT
    execute_on = 'timestep_begin'
    flag_sample = 'flag_sample'
    inputs = 'inputs'
    gp_mean = 'gp_mean'
    lf_corrected = 'lf_corrected'
    gp_std = 'gp_std'
    n_train = 30
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
    learning_function='Ufunction'
    learning_function_parameter = 350.0
    learning_function_threshold=2.0
    outputs_lf = constant/reporter_transfer_lf:avg:value
  []
  [adaptive_MC]
    type = AdaptiveMonteCarloDecision
    output_value = conditional/lf_corrected
    inputs = 'inputs'
    sampler = mc
    gp_decision = conditional
  []
  [ais_stats]
    type = AdaptiveImportanceStats
    output_value = conditional/lf_corrected
    sampler = mc
    flag_sample = 'conditional/flag_sample'
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
    learning_rate_adam = 0.005
    show_optimization_details = true
    batch_size = 100
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
    type= SquaredExponentialCovariance
    signal_variance = 1.0
    noise_variance = 1e-8
    length_factor = '1.0 1.0 1.0'
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  # file_base = 'monte_carlo'
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]
