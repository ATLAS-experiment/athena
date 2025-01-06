#!/bin/bash
# 
# Scheduling system configuration options.
#
# All option lines start with '#SBATCH' followed by the 
# option to be set.  The options are exactly the same as
# may be used on the command line when submitting a job using
# the sbatch command.  See man sbatch for details.
#
# Give your job a sensible name.
# The name will appear in the listing of jobs by squeue.
#SBATCH --job-name=st3-v2--bb4l-dl-c1280
#
# Define the stdout and stderr output files.
# By default the current directory at submission time is taken.
# However, you may add absolute paths to it (recommended).
#SBATCH --output=slurm-%j.out
#SBATCH --error=slurm-%j.err
#
# Choose a partition of suitable length.
#SBATCH --partition=normal
#
#SBATCH --ntasks=1280
#SBATCH --cpus-per-task=1
#
# Show command line arguments provided at sbatch submission time.
# First argument is used below to overwrite source code path.
echo "Cmd line args: $*"

POWHEGDIR=/beegfs/voss
source ${POWHEGDIR}/setup.sh
mpirun -np 1280 ${POWHEGDIR}/powheg-box-res-bb4l-sl-beta/b_bbar_4l/run-example/runparallel-mpi-st3 "/beegfs/voss/powheg-box-res-bb4l-sl-beta/b_bbar_4l/pwhg_main" 
