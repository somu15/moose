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
    execute_on = 'PRE_MULTIAPP_SETUP' # TIMESTEP_BEGIN
    subset_probability = 0.1
    proposal_std = '0.25 0.25'
    num_samplessub = 10
    use_absolute_value = true
    # seed = 1012
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
  [stats]
    type = StatisticsReporter
    reporters = 'adaptive_MC/output_reporter1 adaptive_MC/mu1 adaptive_MC/mu2'
    compute = 'mean'
  []
[]

[Executioner]
  type = Transient
  num_steps = 20
[]

[Outputs]
  csv = true
  exodus = false
[]
