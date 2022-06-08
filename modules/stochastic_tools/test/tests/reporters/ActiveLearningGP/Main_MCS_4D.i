[StochasticTools]
[]

[Distributions]
  # [k_dist]
  #   type = Uniform
  #   lower_bound = 0
  #   upper_bound = 20
  # []
  # [q_dist]
  #   type = Uniform
  #   lower_bound = 7000
  #   upper_bound = 13000
  # []
  # [L_dist]
  #   type = Uniform
  #   lower_bound = 0.0
  #   upper_bound = 0.1
  # []
  # [Tinf_dist]
  #   type = Uniform
  #   lower_bound = 270
  #   upper_bound = 330
  # []
  [k_dist]
    type = TruncatedNormal
    mean = 5
    standard_deviation = 2
    lower_bound = 0
  []
  [q_dist]
    type = TruncatedNormal
    mean = 10000
    standard_deviation = 500
    lower_bound = 0
  []
  [L_dist]
    type = TruncatedNormal
    mean = 0.03
    standard_deviation = 0.01
    lower_bound = 0
  []
  [Tinf_dist]
    type = TruncatedNormal
    mean = 300
    standard_deviation = 10
    lower_bound = 0
  []
[]

[Samplers]
  [mc]
    type = MonteCarlo
    num_rows = 5000
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    seed = 10
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    sampler = mc
    input_files = 'Sub4D.i'
    execute_on = 'TIMESTEP_BEGIN'
    mode = batch-reset
  []
[]

[Controls]
  [cmdline]
    type = MultiAppCommandLineControl
    multi_app = sub
    sampler = mc
    param_names = 'Materials/conductivity/prop_values Kernels/source/value Mesh/xmax BCs/right/value'
  []
[]

[Transfers]
  [avg]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = mc
    to_vector_postprocessor = avg
    from_postprocessor = avg
  []
[]

[VectorPostprocessors]
  [avg]
    type = StochasticResults
    execute_on = 'TIMESTEP_END'
  []
  [sampler_data]
    type = SamplerData
    sampler = mc
    execute_on = 'TIMESTEP_END'
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
