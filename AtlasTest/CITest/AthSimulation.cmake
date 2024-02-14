# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# CI test definitions for the AthSimulation project
# --> README.md before you modify this file
#

atlas_add_citest( DuplicateClass
   SCRIPT python -c 'import ROOT'
   PROPERTIES FAIL_REGULAR_EXPRESSION "class .* is already in" )

atlas_add_citest( SimulationRun2FullSim
   SCRIPT RunWorkflowTests_Run2.py --CI -s -w FullSim --threads 4 -e '--maxEvents 10' --run-only
   LOG_IGNORE_PATTERN "WARNING FPE"  # ignore FPEs from Geant4
   PROPERTIES PROCESSORS 4 )

atlas_add_citest( SimulationRun2FullSimChecks
   SCRIPT RunWorkflowTests_Run2.py --CI -s -w FullSim --checks-only --output-path ../SimulationRun2FullSim
   LOG_IGNORE_PATTERN "WARNING FPE"  # ignore FPEs from Geant4
   DEPENDS_SUCCESS SimulationRun2FullSim)

atlas_add_citest( SimulationRun3FullSim
   SCRIPT RunWorkflowTests_Run3.py --CI -s -w FullSim --threads 4 -e '--maxEvents 50' --run-only
   LOG_IGNORE_PATTERN "WARNING FPE"  # ignore FPEs from Geant4
   PROPERTIES PROCESSORS 4 )

atlas_add_citest( SimulationRun3FullSimChecks
   SCRIPT RunWorkflowTests_Run3.py --CI -s -w FullSim --checks-only --output-path ../SimulationRun3FullSim
   LOG_IGNORE_PATTERN "WARNING FPE"  # ignore FPEs from Geant4
   DEPENDS_SUCCESS SimulationRun3FullSim )

atlas_add_citest( SimulationRun4FullSim
   SCRIPT RunWorkflowTests_Run4.py --CI -s -w FullSim --threads 4 -e '--maxEvents 10' --run-only
   LOG_IGNORE_PATTERN "WARNING FPE"  # ignore FPEs from Geant4
   PROPERTIES PROCESSORS 4 )

atlas_add_citest( SimulationRun4FullSimChecks
   SCRIPT RunWorkflowTests_Run4.py --CI -s -w FullSim --checks-only --output-path ../SimulationRun4FullSim
   LOG_IGNORE_PATTERN "WARNING FPE"  # ignore FPEs from Geant4
   DEPENDS_SUCCESS SimulationRun4FullSim )

atlas_add_citest( SimulationRun3HitsMergeWithSort
   SCRIPT RunWorkflowTests_Run3.py --CI -s -w HitsMerge -e '--inputHITSFile ../../SimulationRun3FullSim/run_s4006/myHITS.pool.root'  # go two levels up as the test runs in a subfolder
   DEPENDS_SUCCESS SimulationRun3FullSim )

atlas_add_citest( SimulationRun3HitsFilter
   SCRIPT RunWorkflowTests_Run3.py --CI -s -w HitsFilter -e '--maxEvents 10')
