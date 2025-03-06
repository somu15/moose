[StochasticTools]
  auto_create_executioner = false
[]

[Samplers]
  [sample]
    type = CSVSampler
    samples_file = 'confg.csv'
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
    param_names = 'PP_x PP_y'
  []
[]

[Transfers]
  [log_resultant_velocity]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = sample
    to_vector_postprocessor = log_resultant_velocity
    from_postprocessor = log_resultant_velocity
  []
[]

[VectorPostprocessors]
  [data]
    type = SamplerData
    sampler = sample
    execute_on = 'initial timestep_end'
  []
  [log_resultant_velocity]
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