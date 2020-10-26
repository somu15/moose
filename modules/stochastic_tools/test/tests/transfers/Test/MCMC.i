[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  # [mu1]
  #   type = Normal
  #   mean = 0.3
  #   standard_deviation = 0.045
  # []
  # [mu2]
  #   type = Normal
  #   mean = 9
  #   standard_deviation = 1.35
  # []
  [mu1]
    type = Uniform
    lower_bound = 0.21
    upper_bound = 0.39
  []
  [mu2]
    type = Uniform
    lower_bound = 6.3
    upper_bound = 11.7
  []
[]

[Samplers]
  [sample]
    type = Dummy
    num_rows = 1
    distributions = 'mu1 mu2'
    execute_on = TIMESTEP_BEGIN
    inputs_vpp = data1
    inputs_names = 'sample_0 sample_1'
    proposal_std = '0.075 0.75'
    seeds = '0.25 8.0'
    # vpp_limits = '-0.16' # '13.93'
    # results_vpp = results
    vpp_names = 'data:average'
    num_samples_adaptive = 5
    seed = 200
  []
[]

# [Reporters]
#   # [b]
#   #   type = TestGetReporter
#   #   int_reporter = a/int
#   #   real_reporter = a/real
#   #   vector_reporter = a/vector
#   #   string_reporter = a/string
#   #   broadcast_reporter = a/broadcast
#   #   scatter_reporter = a/scatter
#   #   gather_reporter = a/gather
#   # []
#   [a]
#     type = TestDeclareReporter
#   []
# []

[MultiApps]
  [sub]
    type = SamplerTransientMultiApp # SamplerFullSolveMultiApp
    input_files = sub_new.i
    sampler = sample
    mode = batch-restore
    reset_apps = 0
    reset_time = 1
  []
  # [sub]
  #   type = SamplerFullSolveMultiApp
  #   input_files = sub_new.i
  #   sampler = sample
  #   # mode = batch-restore
  #   # reset_apps = 0
  #   # reset_time = 1
  # []
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
  []
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    sampler = sample
    to_vector_postprocessor = results
    from_postprocessor = 'average' # 'left_bc right_bc central_value'
  []
[]

[VectorPostprocessors]
  [results]
    type = StochasticResults
    parallel_type = REPLICATED
  []
  [data1]
    type = SamplerData
    sampler = sample
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  auto_advance = true
  num_steps = 5000 # 2000
  # n_startup_steps = 11
  # dt = 0.01
[]

[Outputs]
  execute_on = 'TIMESTEP_BEGIN'
  csv = true
[]
