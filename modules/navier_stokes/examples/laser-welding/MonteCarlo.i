[StochasticTools]
[]

[Samplers]
  [sample]
    type = CSVSampler
    samples_file = 'inputs_test.csv' # test.csv
    execute_on = 'PRE_MULTIAPP_SETUP'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = 2d.i
    sampler = sample
  []
[]

[Transfers]
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'T_vec/T'
    stochastic_reporter = 'constant'
    from_multi_app = sub
    sampler = sample
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'surfacetemp power'
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = 'mc_test'
  [out]
    type = CSV # JSON
    # execute_system_information_on = NONE
  []
[]
