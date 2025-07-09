# Process parameters
scanning_speed=1.0725498e+00 # m/s
power=7.4774826e+01 # W (this is the effective power so multiplied by eta)
R=1.3681599e-04 # m (this is the effective radius)

# Geometric parameters
thickness=1e-4 # m
xmin=-0.1e-3 # m
xmax=0.75e-3 # m
ymin=${fparse -thickness}
surfacetemp=300 # K (temperature at the other side of the plate)

# Time stepping parameters
endtime=8e-4 # s
timestep=${fparse endtime/1000} # s
