[StochasticTools]
  auto_create_executioner = false
[]

# [Distributions]
#   [R_dist]
#     type = Uniform
#     lower_bound = 1.25e-4
#     upper_bound = 1.55e-4
#   []
#   [power_dist]
#     type = Uniform
#     lower_bound = 60
#     upper_bound = 75
#   []
#   [sc_dist]
#     type = Uniform
#     lower_bound = 1.0
#     upper_bound = 1.5
#   []
# []

[Samplers]
  [sample]
    type = CSVSampler
    samples_file = 'params_large.csv'
    execute_on = PRE_MULTIAPP_SETUP
    min_procs_per_row = 8
    max_procs_per_row = 8
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = 2d.i
    sampler = sample
    execute_on = 'TIMESTEP_BEGIN'
    mode = batch-reset
    min_procs_per_row = 8
    max_procs_per_row = 8
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'R power scanning_speed'
  []
[]

[VectorPostprocessors]
  [data]
    type = SamplerData
    sampler = sample
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
