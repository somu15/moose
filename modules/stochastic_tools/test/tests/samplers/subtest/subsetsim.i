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
    type = SubsetSimulation
    num_rows = 1
    distributions = 'mu1 mu2'
    execute_on = PRE_MULTIAPP_SETUP
    subset_probability = 0.1
    proposal_std = '0.03375 1.0125' # '0.15 0.15'
    num_samplessub = 750
    use_absolute_value = true
    seed = 1012
    inputs_reporter = 'adaptive_MC/mu1 adaptive_MC/mu2'
    output_reporter = 'adaptive_MC/output_reporter1'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub1.i
    sampler = sample
    # mode = batch-reset
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
    subapp_index = 0
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
  num_steps = 3000
[]

[Outputs]
  csv = true
  exodus = false
[]
