[Mesh]
  type = FileMesh
  file = NewHF0_R3.e
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
  [./freex1]
    type = DirichletBC
    variable = disp_x
    boundary = Top
    value = '1.3313596176971743'
  [../]
  [./freey1]
    type = DirichletBC
    variable = disp_y
    boundary = Top
    value = '0.2321070482675563'
  [../]
  [./freez1]
    type = DirichletBC
    variable = disp_z
    boundary = Top
    value = '0.18582845203489032'
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityTensor
    C_ijkl = '220.67694654622548 77.65329952368425 86.21826524606472 355.97944774401776 156.08390925234983'
    fill_method = axisymmetric_rz
  [../]
  [./strain]
    type = ComputeSmallStrain # ComputeFiniteStrain #
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type =  ComputeLinearElasticStress # ComputeFiniteStrainElasticStress #
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
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  solve_type = NEWTON
  line_search = 'none'
  nl_max_its = 15
  l_max_its = 10
  nl_rel_tol = 1e-2
  l_tol = 1e-4
  nl_abs_tol = 1e-3
  automatic_scaling = true
[]

[Postprocessors]
  [./vmstress1]
    type = MaxVonMises
    variable_name = vmstress
  [../]
[]

[Outputs]
  file_base = 'Cylinder_HF1_R3'
  exodus = false
  csv = true
  perf_graph = true
[]
