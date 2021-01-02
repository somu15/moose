[StochasticTools]
  # auto_create_executioner = false
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
  # [mu1]
  #   type = Uniform
  #   lower_bound = 0.21
  #   upper_bound = 0.39
  # []
  # [mu2]
  #   type = Uniform
  #   lower_bound = 6.3
  #   upper_bound = 11.7
  # []
[]

[Samplers]
  [sample]
    type = SubsetSimulation
    num_rows = 1
    distributions = 'mu1 mu2'
    execute_on = PRE_MULTIAPP_SETUP # TIMESTEP_BEGIN
    inputs_vpp = data1
    inputs_names = 'sample_0 sample_1'
    results_vpp = results
    vpp_names = 'data:average'
    subset_probability = 0.1
    proposal_std = '0.01 0.25'
    num_samplessub = 200
    seed = 101
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    # mode = normal
    ignore_solve_not_converge = true
  []
[]

[Transfers]
  [sub1]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Kernels/nonlin_function/mu1 Kernels/nonlin_function/mu2'
    to_control = 'stochastic'
    # execute_on =  'None' # TIMESTEP_BEGIN # INITIAL #
    check_multiapp_execute_on = false
    # keep_solve_fail_value = true
  []
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    sampler = sample
    to_vector_postprocessor = results
    from_postprocessor = 'average' # 'left_bc right_bc central_value'
    # keep_solve_fail_value = true
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
  [results]
    type = StochasticResults
    parallel_type = REPLICATED
    # execute_on = 'TIMESTEP_END'
  []
  [data1]
    type = SamplerData
    sampler = sample
    execute_on = 'TIMESTEP_BEGIN'
    # execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  # auto_advance = true
  num_steps = 800 # 2000
  # error_on_dtmin = false
  # type = Steady
[]

[Outputs]
  csv = true
  exodus = false
  # execute_on = 'TIMESTEP_END' # TIMESTEP_BEGIN
  # print_linear_converged_reason = false
  # print_nonlinear_converged_reason = false
[]
