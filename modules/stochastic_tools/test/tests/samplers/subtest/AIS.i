[StochasticTools]
  # auto_create_executioner = false
[]

[Distributions]
  [mu1]
    type = Normal
    mean = 0.3
    standard_deviation = 0.045
  []
  [mu2]
    type = Normal
    mean = 9
    standard_deviation = 1.35
  []
[]

[Samplers]
  [sample]
    type = AIS
    distributions = 'mu1 mu2'
    execute_on = 'PRE_MULTIAPP_SETUP' # TIMESTEP_END
    proposal_std = '0.75 0.75'
    output_limit = 0.19
    num_samples_train = 50
    std_factor = 0.85
    use_absolute_value = true
    # seed = 1012
    initial_values = '0.36477415890676 12.104205698488'
    proposal_distribution = 'StandardNormal'
    inputs_reporter = 'adaptive_MC/mu1 adaptive_MC/mu2'
    output_reporter = 'adaptive_MC/output_reporter1'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub1.i
    sampler = sample
    # mode = normal
  []
[]

[Transfers]
  [sub1]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Kernels/nonlin_function/mu1 Kernels/nonlin_function/mu2'
    to_control = 'stochastic'
    check_multiapp_execute_on = false
  []
  [data]
    type = MultiAppReporterTransfer
    to_reporters = 'adaptive_MC/output_reporter1'
    from_reporters = 'average/value'
    direction = from_multiapp
    multi_app = sub
  []
[]

[Controls]
  [cmdline]
    type = MultiAppCommandLineControl
    multi_app = sub
    sampler = sample
    param_names = 'Kernels/nonlin_function/mu1 Kernels/nonlin_function/mu2'
  []
[]

[Reporters]
  [adaptive_MC]
    type = AdaptiveMonteCarloDecision
    output_names = output_reporter1
    output_values = '0.0'
    inputs_names = 'mu1 mu2'
    inputs_values = '0.0 0.0'
    sampler = sample
  []
[]

[Executioner]
  type = Transient
  num_steps = 150
[]

[Outputs]
  csv = true
  exodus = false
  perf_graph = true
[]
