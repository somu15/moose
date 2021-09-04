[StochasticTools]
  auto_create_executioner = false
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
    type = LatinHypercube
    num_rows = 50000 # Number of Monte Carlo samples
    distributions = 'mu1 mu2' # uniform
    execute_on = 'PRE_MULTIAPP_SETUP'
    # seed = 111
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    mode = batch-reset
  []
[]

[Transfers]
  [sic_failure_overall]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    sampler = sample
    to_vector_postprocessor = average
    from_postprocessor = average
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
  [average]
    type = StochasticResults
    # execute_on = 'TIMESTEP_END'
    # parallel_type = DISTRIBUTED
  []
  [data]
    type = SamplerData
    sampler = sample
    # execute_on = 'initial timestep_end'
    # parallel_type = DISTRIBUTED
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  exodus = false
  execute_on = 'TIMESTEP_END'
  print_linear_converged_reason = false
  print_nonlinear_converged_reason = false
[]
