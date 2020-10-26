[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  # [uniform_left]
  #   type = Uniform
  #   lower_bound = 0
  #   upper_bound = 0.5
  # []
  # [uniform_right]
  #   type = Uniform
  #   lower_bound = 1
  #   upper_bound = 2
  # []
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
    num_rows = 20000
    distributions = 'mu1 mu2'
    execute_on = INITIAL
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub_new.i
    sampler = sample
  []
[]

[Transfers]
  [sub]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Kernels/nonlin_function/mu1 Kernels/nonlin_function/mu2'
    to_control = 'stochastic'
    execute_on = INITIAL
    check_multiapp_execute_on = false
  []
  [average]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    sampler = sample
    to_vector_postprocessor = average
    from_postprocessor = average
  []
[]

[VectorPostprocessors]
  [sampler_data]
    type = SamplerData
    sampler = sample
    execute_on = 'TIMESTEP_END'
  []
  [average]
    type = StochasticResults
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Steady
  # num_steps = 1
  # dt = 0.01
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL TIMESTEP_END'
[]
