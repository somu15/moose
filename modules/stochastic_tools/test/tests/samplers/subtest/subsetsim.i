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
[]

[Samplers]
  [sample]
    type = SubTest
    num_rows = 1
    distributions = 'mu1 mu2'
    execute_on = PRE_MULTIAPP_SETUP
    # inputs_vpp = data1
    # inputs_names = 'sample_0 sample_1'
    inputs_reporter = 'adaptive_MC/inputs_reporter1'
    results_reporter = 'adaptive_MC/results_reporter'
    subset_probability = 0.1
    proposal_std = '1.0 1.0'
    num_samplessub = 10
    seed = 101
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
  [sub1]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Kernels/nonlin_function/mu1 Kernels/nonlin_function/mu2'
    to_control = 'stochastic'
    check_multiapp_execute_on = false
    # keep_solve_fail_value = true
  []
  # [sub1]
  #   type = MultiAppReporterTransfer
  #   to_reporters = 'Kernels/nonlin_function/mu1 Kernels/nonlin_function/mu2'
  #   from_reporters = 'inputs_reporter/inputs_reporter1'
  #   direction = to_multiapp
  #   multi_app = sub
  # []
  [data]
    type = MultiAppReporterTransfer
    to_reporters = 'adaptive_MC/results_reporter'
    from_reporters = 'average/value'
    direction = from_multiapp
    multi_app = sub
    subapp_index = 0
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

[Reporters]
  # [results_reporter]
  #   type = ConstantReporter
  #   real_names = results_reporter1
  #   real_values = '0.0'
  #   vector_real_names = inputs_reporter1
  #   vector_real_values = '0.0 0.0'
  # []
  [adaptive_MC]
    type = AdaptiveMonteCarlo
    real_names = results_reporter
    real_values = '0.0'
    real_vec_names = inputs_reporter
    real_vec_values = '0.0 0.0'
    sampler = sample
  []
[]

# [VectorPostprocessors]
#   [data1]
#     type = SamplerData
#     sampler = sample
#     execute_on = 'TIMESTEP_BEGIN'
#     # execute_on = 'TIMESTEP_END'
#   []
# []

[Executioner]
  type = Transient
  num_steps = 20 # 2000
[]

[Outputs]
  csv = true
  exodus = false
[]
