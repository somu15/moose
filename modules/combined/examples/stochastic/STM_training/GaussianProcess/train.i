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

[Samplers]
  [train_sample]
    type = MonteCarlo
    num_rows = 20
    distributions = 'vel1 vel2'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = NavierStokes.i
    sampler = train_sample
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = train_sample
    param_names = 'param1 param4'
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    sampler = train_sample
    stochastic_reporter = results
    from_reporter = 'resultant_velocity5/value'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
    parallel_type = ROOT
  []
[]

[Trainers]
  [GP_trainer]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'covar' #Choose a squared exponential for the kernel
    standardize_params = 'true' #Center and scale the training params
    standardize_data = 'true' #Center and scale the training data
    sampler = train_sample
    response = results/data:resultant_velocity5:value
    tune_parameters = 'covar:signal_variance covar:length_factor'
    num_iters = 2000
    learning_rate = 0.01
    show_every_nth_iteration = 100
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

[Outputs]
  file_base = gauss_process_training
  [out]
    type = SurrogateTrainerOutput
    trainers = 'GP_trainer'
  []
[]

[Executioner]
  type = Steady
[]
