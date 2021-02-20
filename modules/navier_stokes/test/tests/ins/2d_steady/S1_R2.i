[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1.0
    ymin = 0
    ymax = 1.0
    nx = 40
    ny = 40
    # elem_type = QUAD9
  []
  [./corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = gen
  [../]
  second_order = true
[]

[AuxVariables]
  [vel_x]
    order = SECOND
  []
  [vel_y]
    order = SECOND
  []
[]

[AuxKernels]
  [vel_x]
    type = VectorVariableComponentAux
    variable = vel_x
    vector_variable = velocity
    component = 'x'
  []
  [vel_y]
    type = VectorVariableComponentAux
    variable = vel_y
    vector_variable = velocity
    component = 'y'
  []
[]

[Variables]
  [./velocity]
    order = SECOND
    family = LAGRANGE_VEC
  [../]
  [./p]
  [../]
[]

[Kernels]
  [./mass]
    type = INSADMass
    variable = p
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
[]

[BCs]
  [./lid]
    type = VectorDirichletBC
    variable = velocity
    boundary = 'top'
    values = '1.3410784360135477 0.0 0.0'
  [../]
  [./lid1]
    type = VectorDirichletBC
    variable = velocity
    boundary = 'bottom'
    values = '1.0013901112571446 0.0 0.0'
  [../]
  [./lid2]
    type = VectorDirichletBC
    variable = velocity
    boundary = 'left'
    values = '0.0 0.8545803474960022 0.0'
  [../]
  [./lid3]
    type = VectorDirichletBC
    variable = velocity
    boundary = 'right'
    values = '0.0 0.8989251182456928 0.0'
  [../]

  [./pressure_pin]
    type = DirichletBC
    variable = p
    boundary = 'pinned_node'
    value = 0.0
  [../]
[]

[Materials]
  [./const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '1.351229505990859 0.007525065471609147'
  [../]
  [ins_mat]
    type = INSADMaterial
    velocity = velocity
    pressure = p
  []
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
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  nl_rel_tol = 1e-12
  nl_max_its = 100
  l_max_its = 20
  automatic_scaling = true
[]

[Postprocessors]
  [./vel_x]
    type = PointValue
    point = '0.5 0.5 0.0'
    variable = vel_x
  [../]
  [./vel_y]
    type = PointValue
    point = '0.5 0.5 0.0'
    variable = vel_y
  [../]
[]

[Outputs]
  file_base = S1_R2
  exodus = false
  perf_graph = false
  csv = true
[]
