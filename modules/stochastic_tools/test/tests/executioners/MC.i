[StochasticTools]
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 1
    upper_bound = 10
  []
[]

[Samplers]
  [dynamic]
    type = MonteCarlo
    num_rows = 1
    distributions = 'uniform'
    execute_on = "SAMPLER1"
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = dynamic
    input_files = 'sub.i'
    execute_on = "RUNAPP"
  []
[]

[Transfers]
  [parameters]
    type = SamplerParameterTransfer
    multi_app = runner
    sampler = dynamic
    parameters = 'BCs/right/value'
    to_control = 'stochastic'
  []
  # [results]
  #   type = SamplerPostprocessorTransfer
  #   multi_app = runner
  #   sampler = dynamic
  #   to_vector_postprocessor = results
  #   from_postprocessor = 'center'
  # []
[]

[Executioner]
  type = ActiveLearning # Transient
  # num_steps = 2
[]

[VectorPostprocessors]
  [results]
    type = StochasticResults
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = NONE
    vectorpostprocessors_as_reporters = true
  []
[]
