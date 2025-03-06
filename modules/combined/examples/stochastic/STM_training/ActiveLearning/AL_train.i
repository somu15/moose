[StochasticTools]
[]

[Distributions]
  [vel1]
    type = Uniform
    lower_bound = -3.5
    upper_bound = 3.5
  []
  [vel2]
    type = Uniform
    lower_bound = -3.5
    upper_bound = 3.5
  []
[]

[ParallelAcquisition]
  [eigf]
    type = ExpectedImprovementGlobalFit
  []
[]

[Samplers]
  [sample]
    type = GenericActiveLearningSampler
    distributions = 'vel1 vel2'
    sorted_indices = 'conditional/sorted_indices'
    num_parallel_proposals = 4
    num_tries = 3000
    seed = 200
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = NavierStokes.i
    sampler = sample
  []
[]

[Transfers]
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'resultant_velocity5/value'
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
    param_names = 'param1 param4'
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [conditional]
    type = GenericActiveLearner
    output_value = constant/reporter_transfer:resultant_velocity5:value
    sampler = sample
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
    acquisition = 'eigf'
  []
[]

[Trainers]
  [GP_al_trainer]
    type = ActiveLearningGaussianProcess
    covariance_function = 'covar'
    standardize_params = 'true'
    standardize_data = 'true'
    tune_parameters = 'covar:signal_variance covar:length_factor'
    num_iters = 2000
    learning_rate = 0.01
    show_every_nth_iteration = 100
    # batch_size = 350
  []
[]

[Surrogates]
  [GP_eval]
    type = GaussianProcessSurrogate
    trainer = GP_al_trainer
  []
[]

[Covariance]
  [covar]
    type = SquaredExponentialCovariance
    signal_variance = 4.0
    noise_variance = 1e-6
    length_factor = '4.0 4.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 6
[]

[Outputs]
  file_base = 'al1'
  [out1_parallelAL]
    type = JSON
    execute_system_information_on = NONE
  []
  [out2_parallelAL]
    type = SurrogateTrainerOutput
    trainers = 'GP_al_trainer'
  []
[]
