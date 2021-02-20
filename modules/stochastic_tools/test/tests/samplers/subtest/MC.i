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
    type = MonteCarlo
    num_rows = 1
    distributions = 'mu1 mu2'
    execute_on = 'PRE_MULTIAPP_SETUP TIMESTEP_BEGIN' # 'initial timestep_end'
    seed = 1012
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    # mode = batch-reset
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
    check_multiapp_execute_on = false
  []
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    sampler = sample
    to_vector_postprocessor = results
    from_postprocessor = 'average'
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

[VectorPostprocessors]
  [results]
    type = StochasticResults
    parallel_type = REPLICATED
    # execute_on = 'TIMESTEP_END'
  []
  [data]
    type = SamplerData
    sampler = sample
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = Transient
  num_steps = 200
  # execute_on = 'TIMESTEP_END'
[]

[Outputs]
  # execute_on = 'TIMESTEP_END'
  csv = true
[]
