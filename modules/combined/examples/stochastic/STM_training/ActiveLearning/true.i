[StochasticTools]
  auto_create_executioner = false
[]

[Samplers]
  [sample]
    type = CSVSampler
    samples_file = 'test.csv'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = NavierStokes.i
    sampler = sample
    execute_on = 'TIMESTEP_BEGIN'
    mode = batch-reset
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'param1 param4'
  []
[]

[Transfers]
  [sic_failure_overall]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = sample
    to_vector_postprocessor = resultant_velocity5
    from_postprocessor = resultant_velocity5
  []
[]

[VectorPostprocessors]
  [resultant_velocity5]
    type = StochasticResults
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
   csv = true
[]
