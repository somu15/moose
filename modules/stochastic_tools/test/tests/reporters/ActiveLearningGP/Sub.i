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
        value = 8550.22
    []
[]

[Materials]
    [conductivity]
        type = GenericConstantMaterial
        prop_names = k
        prop_values = 8.85185
    []
[]

[BCs]
    [right]
        type = DirichletBC
        variable = T
        boundary = right
        value = 299.6
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
