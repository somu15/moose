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
    num_rows = 1
    distributions = 'mu1 mu2'
    execute_on = 'PRE_MULTIAPP_SETUP TIMESTEP_BEGIN' #
    proposal_std = '0.75 0.75'
    output_limit = 0.1
    num_samples_train = 500
    std_factor = 0.85
    use_absolute_value = true
    # seed = 1012
    seeds = '0.2875 11.059'
    inputs_reporter = 'adaptive_MC/mu1 adaptive_MC/mu2'
    output_reporter = 'adaptive_MC/output_reporter1'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub1.i
    sampler = sample
    mode = normal
  []
[]

[Transfers]
  [sub1]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Kernels/nonlin_function/mu1 Kernels/nonlin_function/mu2'
    to_control = 'stochastic'
    execute_on  = 'TIMESTEP_BEGIN'
    # check_multiapp_execute_on = false
  []
  [data]
    type = MultiAppReporterTransfer
    to_reporters = 'adaptive_MC/output_reporter1'
    from_reporters = 'average/value'
    direction = from_multiapp
    multi_app = sub
    subapp_index = 0
    execute_on  = 'TIMESTEP_END'
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
    execute_on = 'TIMESTEP_BEGIN TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1000
[]

[Outputs]
  csv = true
  exodus = false
[]
