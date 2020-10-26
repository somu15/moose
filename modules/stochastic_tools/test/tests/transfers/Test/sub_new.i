[Functions]
  [source]
    type = ParsedFunction
    value = "100 * sin(2 * pi * x) * sin(2 * pi * y)"
  []
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    xmin = 0
    xmax = 1
    ny = 20
    ymin = 0
    ymax = 1
  []
[]

[Variables]
  [U]
    family = lagrange
    order = first
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = U
  []
  [nonlin_function]
    type = ExponentialReaction
    variable = U
    mu1 = 0.385
    mu2 = 11.5
  []
  [source]
    type = BodyForce
    variable = U
    function = source
  []
[]

[BCs]
  [dirichlet_all]
    type = DirichletBC
    variable = U
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  # automatic_scaling = 'true'
  nl_abs_tol = 1e-13
  nl_rel_tol = 1e-13
  l_tol = 1e-13
  l_abs_tol = 1e-13
  l_max_its = 7
[]

[Postprocessors]
  # [max]
  #   type = ElementExtremeValue
  #   variable = U
  # []
  # [min]
  #   type = ElementExtremeValue
  #   variable = U
  #   value_type = min
  # []
  [average]
    type = ElementAverageValue
    variable = U
  []
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[Outputs]
[]
