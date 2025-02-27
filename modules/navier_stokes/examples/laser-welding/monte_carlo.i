[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [R_dist]
    type = Uniform
    lower_bound = 1.25e-4
    upper_bound = 1.55e-4
  []
  [power_dist]
    type = Uniform
    lower_bound = 60
    upper_bound = 75
  []
  [sc_dist]
    type = Uniform
    lower_bound = 1.0
    upper_bound = 1.5
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 120
    distributions = 'R_dist power_dist sc_dist'
    execute_on = PRE_MULTIAPP_SETUP
    # min_procs_per_row = ?
    # max_procs_per_row = ?
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = 2d.i
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
    param_names = 'R power scanning_speed'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
#   perf_graph = true
#   csv = true
#   exodus = false
#   execute_on = 'TIMESTEP_END'
#   print_linear_converged_reason = false
#   print_nonlinear_converged_reason = false
[]