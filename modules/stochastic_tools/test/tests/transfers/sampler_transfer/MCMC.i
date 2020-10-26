[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  # [uniform_left]
  #   type = Normal
  #   mean = 15.0
  #   standard_deviation = 4.0
  # []
  # [uniform_right]
  #   type = Normal
  #   mean = 8.0
  #   standard_deviation = 2.0
  # []
  [uniform_left]
    type = Uniform
    lower_bound = 0
    upper_bound = 0.5
  []
  [uniform_right]
    type = Uniform
    lower_bound = 1
    upper_bound = 2
  []
[]

[Samplers]
  [sample]
    type = Dummy # MonteCarloSampler
    num_rows = 1
    distributions = 'uniform_left uniform_right'
    execute_on = TIMESTEP_BEGIN # INITIAL
    inputs_vpp = data1 # inputs11
    inputs_names = 'sample_0 sample_1' # 'data11:left_bc data11:right_bc'
    proposal_std = '0.1 0.1'
    seeds = '0.45 1.95'
    # seeds = '0.3771954 1.542878'
    # proposal_std = '0.01 0.01'
    # seeds = '20.02595 8.691617'
    vpp_limits = '1.2' # '13.93'
    results_vpp = results
    vpp_names = 'data:central_value'
    num_samples_adaptive = 100
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    mode = normal # batch-reset
  []
[]

[Transfers]
  [sub1]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'BCs/left/value BCs/right/value'
    to_control = 'stochastic'
    # execute_on =  'None' # TIMESTEP_BEGIN # INITIAL #
    check_multiapp_execute_on = false
  []
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    sampler = sample
    to_vector_postprocessor = results
    from_postprocessor = 'central_value' # 'left_bc right_bc central_value'
  []
  [data11]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    sampler = sample
    to_vector_postprocessor = inputs11
    from_postprocessor = 'left_bc right_bc' # 'left_bc right_bc central_value'
  []
[]

[VectorPostprocessors]
  [inputs11]
    type = StochasticResults
    parallel_type = REPLICATED
  []
  [results]
    type = StochasticResults
    parallel_type = REPLICATED
  []
  [data1]
    type = SamplerData
    sampler = sample
    execute_on = 'INITIAL'
  []
[]

[Executioner]
  type = Transient
  num_steps = 10 # 2000
  dt = 0.01
[]

[Outputs]
  execute_on = 'TIMESTEP_BEGIN' # 'INITIAL'
  csv = true
  # [./out]
  #   # execute_on = 'TIMESTEP_BEGIN'
  #   type = CSV
  #   file_base = Test
  # [../]
[]
