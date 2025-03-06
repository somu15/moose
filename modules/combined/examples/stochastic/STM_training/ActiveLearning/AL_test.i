[StochasticTools]
[]

[Samplers]
  [test_sample]
    type = CSVSampler
    samples_file = 'test.csv'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[Reporters]
  [samp_avg]
    type = EvaluateSurrogate
    model = GP_avg
    sampler = test_sample
    evaluate_std = 'true'
    parallel_type = ROOT
    execute_on = final
  []
[]

[Surrogates]
  [GP_avg]
    type = GaussianProcessSurrogate
    filename = 'al1_GP_al_trainer.rd'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
