[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmax = 0.13061533868990033
  elem_type = EDGE3
[]

[Variables]
  [T]
    order = SECOND
    family = LAGRANGE
  []
[]

[Kernels]
  [diffusion]
    type = MatDiffusion
    variable = T
    diffusivity = k
  []
  [source]
    type = BodyForce
    variable = T
    value = 12451.650665507455
  []
[]

[Materials]
  [conductivity]
    type = GenericConstantMaterial
    prop_names = k
    prop_values = 5.666647074589191
  []
[]

[BCs]
  [right]
    type = DirichletBC
    variable = T
    boundary = right
    value = 332.770533464511
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [avg]
    type = AverageNodalVariableValue
    variable = T
  []
  [max]
    type = NodalExtremeValue
    variable = T
    value_type = max
  []
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[Outputs]
[]
