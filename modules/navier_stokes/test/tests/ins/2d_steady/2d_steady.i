[Mesh]
  # [gen]
  #   type = GeneratedMeshGenerator
  #   dim = 2
  #   xmin = -0.5
  #   xmax = 0.5
  #   ymin = -0.5
  #   ymax = 0.5
  #   nx = 20
  #   ny = 20
  # []
  # [./corner_node]
  #   type = ExtraNodesetGenerator
  #   new_boundary = 'pinned_node'
  #   nodes = '0'
  #   input = gen
  # [../]
  type = FileMesh
  file = mesh.e
[]

# [AuxVariables]
#   [u]
#     initial_condition = 1
#   []
# []

[Variables]
  [./velocity]
    family = LAGRANGE_VEC
  [../]
  [./p]
  [../]
  # [./temperature]
  # [../]
[]

# [ICs]
#   [velocity]
#     type = VectorConstantIC
#     x_value = 1e-5
#     y_value = 1e-5
#     variable = velocity
#   []
# []

[Kernels]
  [./mass]
    type = INSADMass
    variable = p
  [../]
  # [./mass_pspg]
  #   type = INSADMassPSPG
  #   variable = p
  # [../]
  [./momentum_convection]
    type = INSADMomentumAdvection
    variable = velocity
  [../]

  [./momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
  [../]

  [./momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    p = p
    integrate_p_by_parts = true
  [../]
  # [./momentum_supg]
  #   type = INSADMomentumSUPG
  #   variable = velocity
  #   velocity = velocity
  # [../]
[]

[BCs]
  [./pressure_top]
    type = DirichletBC
    variable = p
    boundary = 'Top'
    value = 4000.0
  [../]
  [./pressure_bot]
    type = DirichletBC
    variable = p
    boundary = 'Bottom'
    value = 4500.0
  [../]
  [./pressure_left]
    type = DirichletBC
    variable = p
    boundary = 'Left'
    value = 5500.0
  [../]
  [./pressure_right]
    type = DirichletBC
    variable = p
    boundary = 'Right'
    value = 6500.0
  [../]
[]

[Materials]
  [./const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '1000.0  8.9e-4' # 8.9e-4
  [../]
  [ins_mat]
    type = INSADMaterial # INSADStabilized3Eqn
    velocity = velocity
    pressure = p
    # temperature = temperature
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  # petsc_options_iname = '-pc_type -sub_pc_factor_levels -ksp_gmres_restart'
  # petsc_options_value = 'asm      6                     200'
  # line_search = 'none'
  nl_rel_tol = 1e-12
  nl_max_its = 10
  # automatic_scaling = true
[]

# [UserObjects]
#   [./tracker]
#     type = INSADObjectTracker
#   [../]
# []

[Postprocessors]
  [./Pressure]
    type = PointValue
    point = '0.0 0.0 0.0'
    variable = p
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  # [out]
  #   type = Exodus
  #   hide = 'u'
  # []
[]
