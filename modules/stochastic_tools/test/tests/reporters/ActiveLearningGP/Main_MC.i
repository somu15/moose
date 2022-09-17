[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Uniform
    lower_bound = 5
    upper_bound = 20
  []
  [q_dist]
    type = Uniform
    lower_bound = 7000
    upper_bound = 13000
  []
  [Tinf_dist]
    type = Uniform
    lower_bound = 250
    upper_bound = 350
  []
#   [k_dist]
#     type = Uniform
#     lower_bound = 1
#     upper_bound = 10
#   []
#   [q_dist]
#     type = Uniform
#     lower_bound = 9000
#     upper_bound = 11000
#   []
[]

[Samplers]
  [mc]
    type = MonteCarlo
    num_rows = 4000
    distributions = 'k_dist q_dist Tinf_dist' # 'k_dist q_dist' # L_dist 
    seed = 5
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    sampler = mc
    input_files = 'Sub.i'
    # mode = batch-reset
    # execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [sub]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = mc
    parameters = 'Materials/conductivity/prop_values Kernels/source/value BCs/right/value' # Mesh/xmax 
    to_control = 'stochastic'
    check_multiapp_execute_on = false
  []
  [out1]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = mc
    to_vector_postprocessor = out1
    from_postprocessor = avg
  []
[]

[Executioner]
  type = Steady
[]

[VectorPostprocessors]
    [out1]
      type = StochasticResults
      execute_on = 'TIMESTEP_END'
    []
[]

[Outputs]
  # execute_on = timestep_end
  perf_graph = true
  #file_base = 'MC_4000'
  csv = true
[]
