[Mesh]
  type = FileMesh
  file = mesh_LF.e
  second_order = true
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = SECOND # FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = SECOND # FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = SECOND # FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = FIRST # CONSTANT #
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = FIRST # CONSTANT #
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = FIRST # CONSTANT #
    family = MONOMIAL
  [../]
  [./vmstress]
    order = FIRST # CONSTANT #
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./DynamicTensorMechanics]
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
    value = 0.001
  [../]
  [./freey1]
    type = DirichletBC
    variable = disp_y
    boundary = Top
    value = 0.001
  [../]
  [./freez1]
    type = DirichletBC
    variable = disp_z
    boundary = Top
    value = 0.001
  [../]
[]

# [NodalKernels]
#   [./force_y2]
#     type = ConstantRate
#     variable = disp_y
#     boundary = corner_node
#     rate = 5.0e-4
#   [../]
# []

[Materials]
  [./elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e2 # 2.60072400269
    poissons_ratio = 0.3 # -0.9998699638
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
  [./smp]
    type = SMP
    full = true
  [../]
[]
[Executioner]
  type = Steady
  # solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  # nl_max_its = 15
  # nl_abs_tol = 1e-12
  # nl_rel_tol = 1e-12
  # l_tol = 1e-12
  # l_abs_tol = 1e-12
  solve_type = NEWTON
  line_search = 'none'
  nl_max_its = 15
  nl_rel_tol = 1e-7
  nl_abs_tol = 1e-7

  # dt = 1
  # dtmin = 1
  # end_time = 2
[]

[Postprocessors]
  # [./disp_x1]
  #   type = PointValue
  #   point = '10.0 0.25 0.0'
  #   variable = disp_x
  # [../]
  # [./disp_y1]
  #   type = PointValue
  #   point = '10.0 0.25 0.0'
  #   variable = disp_y
  # [../]
  # [./stress_xx1]
  #   type = PointValue
  #   point = '0.0 0.0 0.0'
  #   variable = stress_xx
  # [../]
  # [./stress_yy1]
  #   type = PointValue
  #   point = '0.0 0.0 0.0'
  #   variable = stress_yy
  # [../]
  # # [./stress_xy1]
  # #   type = PointValue
  # #   point = '0.0 0.5 0.0'
  # #   variable = stress_xy
  # # [../]
  # [./moment_x_bot]
  #   type = SidesetMoment
  #   direction = '1 0 0'
  #   stress_tensor = stress
  #   boundary = 'left'
  #   ref_point = '0.0 0.0 0.0'
  #   leverarm_dir = 1
  # [../]
  # [./vmstress1]
  #   type = MaxVonMises
  #   variable_name = vmstress
  # [../]
  [./vmstress]
    type = PointValue
    point = '0.0 0.0 0.0'
    variable = vmstress
  [../]
[]

[Outputs]
  file_base = 'Cylinder_LF'
  exodus = false
  csv = true
[]
