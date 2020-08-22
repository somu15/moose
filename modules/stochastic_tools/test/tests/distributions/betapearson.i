[StochasticTools]
[]

[Distributions]
  [betapearson]
    type = Gamma # BetaPearson
    alpha = 5
    beta = 5
    # location = 5
    # scale = 2
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = betapearson
    value = 1.2
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = betapearson
    value = 1.2
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = betapearson
    value = 0.6#0.9999999999
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
