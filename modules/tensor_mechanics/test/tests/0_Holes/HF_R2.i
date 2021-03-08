[Mesh]
  type = FileMesh
  file = HF_R2.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
    # order = SECOND
    # family = LAGRANGE
  [../]
  [./disp_y]
    # order = SECOND
    # family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
    # order = FIRST
    # family = LAGRANGE
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
    # order = FIRST
    # family = LAGRANGE
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
    # order = FIRST
    # family = LAGRANGE
  [../]
  [./vmstress]
    order = CONSTANT
    family = MONOMIAL
    # order = FIRST
    # family = LAGRANGE
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y'
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  [../]
  [./vmstress1]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = vmstress
    scalar_type = VonMisesStress
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = Left
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = Left
    value = 0.0
  [../]
  [./freex1]
    type = DirichletBC
    variable = disp_x
    boundary = Right
    value = '0.8282360723366369'
  [../]
  [./freey1]
    type = DirichletBC
    variable = disp_y
    boundary = Right
    value = '0.0'
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 154.12452295182027
    poissons_ratio = 0.21668527442657742
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y'
  [../]
  [./stress]
    type =  ComputeLinearElasticStress
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  solve_type = NEWTON
  line_search = 'none'
  nl_max_its = 50
  l_max_its = 10
  nl_rel_tol = 1e-3
  nl_abs_tol = 1e-4
  # automatic_scaling = true
[]

[Postprocessors]
  [./vmstress1]
    type = MaxVonMises
    variable_name = vmstress
  [../]
[]

[Outputs]
  file_base = 'HF_R2'
  # exodus = true
  csv = true
  # perf_graph = true
[]
