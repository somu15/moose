[Mesh]
  type = FileMesh
  file = NewHF_R2.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vmstress]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
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
    boundary = Bottom
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = Bottom
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = Bottom
    value = 0.0
  [../]
  # [./freex1]
  #   type = DirichletBC
  #   variable = disp_x
  #   boundary = Top
  #   value = '0.03591140858949984'
  # [../]
  # [./freey1]
  #   type = DirichletBC
  #   variable = disp_y
  #   boundary = Top
  #   value = '0.1536467185512155'
  # [../]
  # [./freez1]
  #   type = DirichletBC
  #   variable = disp_z
  #   boundary = Top
  #   value = '0.06609279628750095'
  # [../]
  [./freex1]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = Top
    function = function_x
  [../]
  [./freey1]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = Top
    function = function_y
  [../]
  [./freez1]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = Top
    function = function_z
  [../]
[]

[Functions]
  [./function_x]
    type = PiecewiseLinear
    x = '0.0 1.0 2.0 3.0'
    y = '0.0 0.01015856154685741 0.02031712309371482 0.03047568464057223'
  [../]
  [./function_y]
    type = PiecewiseLinear
    x = '0.0 1.0 2.0 3.0'
    y = '0.0 0.01783554436321292 0.03567108872642584 0.053506633089638767'
  [../]
  [./function_z]
    type = PiecewiseLinear
    x = '0.0 1.0 2.0 3.0'
    y = '0.0 0.008154420992119275 0.01630884198423855 0.024463262976357824'
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityTensor
    C_ijkl = '295.9563568332149 103.02491153147164 131.95057823509003 372.8248229787782 157.169528908155'
    fill_method = axisymmetric_rz
  [../]
  [./strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type =  ComputeFiniteStrainElasticStress
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  solve_type = NEWTON
  line_search = 'none'
  nl_max_its = 15
  l_max_its = 10
  nl_rel_tol = 1e-3
  l_tol = 1e-4
  nl_abs_tol = 1e-2
  start_time = 0.0
  end_time = 3.0
  dt = 1.0
  automatic_scaling = true
[]

[Postprocessors]
  [./vmstress1]
    type = MaxVonMises
    variable_name = vmstress
  [../]
[]

[Outputs]
  file_base = 'Cylinder_HF_R1'
  exodus = false
  csv = true
  perf_graph = true
[]
