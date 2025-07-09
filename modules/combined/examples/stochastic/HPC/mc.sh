#!/bin/bash
#PBS -M Som.Dhulipala@inl.gov
#PBS -m abe
#PBS -N AM_2D
#PBS -P moose
#PBS -l select=100:ncpus=40:mpiprocs=40
#PBS -l walltime=60:00:00

JOB_NUM=${PBS_JOBID%%\.*}

cd $PBS_O_WORKDIR

\rm -f out
date > out

module purge
module load use.moose moose-dev-openmpi/5e9be02

mpiexec -n 4000 moose-dev-exec /scratch/dhullaks/projects/moose/modules/combined/combined-opt -i monte_carlo.i --allow-test-objects >> out

date >> out
