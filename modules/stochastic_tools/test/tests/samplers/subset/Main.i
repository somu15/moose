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
    type = SSP_1
    distributions = 'mu1 mu2'
    execute_on = PRE_MULTIAPP_SETUP
    subset_probability = 0.1
    num_samplessub = 100
    use_absolute_value = true
    inputs_reporter = 'adaptive_MC/inputs'
    output_reporter = constant/data1:average:value
    data_reporter = 'adaptive_MC/data'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
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
  [data1]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'constant'
    multi_app = sub
    sampler = sample
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
  [constant]
    type = StochasticReporter
  []
  [adaptive_MC]
    type = AMCP_1
    output = constant/data1:average:value
    inputs = 'inputs'
    sampler = sample
    data = 'data'
  []
[]

[Executioner]
  type = Transient
  num_steps = 40
[]

[Outputs]
  json = true
  csv = false
  exodus = false
  perf_graph = true
[]
