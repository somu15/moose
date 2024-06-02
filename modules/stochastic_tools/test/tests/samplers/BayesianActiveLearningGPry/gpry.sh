#!/bin/bash
#PBS -M Som.Dhulipala@inl.gov
#PBS -m abe
#PBS -N GPRY_Pred_Prey
#PBS -P moose
#PBS -l select=1:ncpus=48:mpiprocs=48
#PBS -l walltime=6:00:00

JOB_NUM=${PBS_JOBID%%\.*}

cd $PBS_O_WORKDIR

\rm -f out
date > out

module purge
module load use.moose moose-dev

MV2_ENABLE_AFFINITY=0 mpiexec ~/projects/bison/bison-opt -i gpry_train_predprey.i >> out

date >> out