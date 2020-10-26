[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0.3771
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1.5428
  []
[]

[Executioner]
  type = Transient
  num_steps = 70
  dt = 0.01
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
  l_tol = 1e-10
  l_abs_tol = 1e-10
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[Postprocessors]
  # [left_bc]
  #   type = NodalVariableValue
  #   nodeid = 0
  #   variable = u
  # []
  # [right_bc]
  #   type = NodalVariableValue
  #   nodeid = 10
  #   variable = u
  # []
  # [central_value]
  #   type = NodalVariableValue
  #   nodeid = 5
  #   variable = u
  # []
  [average]
    type = ElementAverageValue
    variable = U
  []
[]

[Outputs]
  csv = true
[]
