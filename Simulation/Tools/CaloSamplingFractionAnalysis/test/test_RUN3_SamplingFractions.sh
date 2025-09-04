#!/bin/sh
#
# art-description: Run simulation of single photon/electrons in LAr with calibration hits and then calculate the sampling fractions
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: summary.txt
# art-output: comparison.txt
# art-output: summary_*F_LAr*.pdf
# art-output: SF_LAr_*.pdf
# art-output: ELAr_hit_*.pdf
# art-output: mcoutput/mc.PG_pid*.HITS.pool.root
# art-output: mcoutput/mc.PG_pid*.NTUP.pool.root
# art-output: mcoutput/mc.PG_pid*.aan.root
# art-output: mcoutput/LArEM_SF_*.root

export ATHENA_CORE_NUMBER=8

run_all_SamplingFraction_simulation.sh 500

ArtPackage=$1
ArtJobName=$2
art.py compare grid ${ArtPackage} ${ArtJobName} --txt-file=summary.txt
status=$?
echo  "art-result: $status regression"

exit $status

