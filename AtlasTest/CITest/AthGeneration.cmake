# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# CI test definitions for the AthGeneration project
# --> README.md before you modify this file
#

atlas_add_citest( DuplicateClass
   SCRIPT python -c 'import ROOT'
   PROPERTIES FAIL_REGULAR_EXPRESSION "class .* is already in" )

atlas_add_citest( DuplicateComponent
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/DuplicateComponentsCheck.py )

atlas_add_citest( Generation_PhPy8_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid 421356 )

atlas_add_citest( Generation_H7_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid 421106 )

atlas_add_citest( Generation_MGPy8_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid 421107 )

atlas_add_citest( Generation_Sherpa_13TeV
   SCRIPT RunWorkflowTests_Run2.py --CI -g --dsid 421003 )

atlas_add_citest( Generation_ParticleGun_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid 421119 )

atlas_add_citest( Generation_JetFilter_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid 421114 )

atlas_add_citest( Generation_PhPy8_13TeV
   SCRIPT RunWorkflowTests_Run2.py --CI -g --dsid 421356 )

atlas_add_citest( Generation_PhPy8_14TeV
   SCRIPT RunWorkflowTests_Run4.py --CI -g --dsid 421356 )

atlas_add_citest( Generation_P8B_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid 421439 )

atlas_add_citest( Generation_Filters_13TeV
   SCRIPT RunWorkflowTests_Run2.py --CI -g --dsid 421408 -e '--inputEVNT_PreFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Evgen_E2E/mc15_13TeV/EVNT.25508216._000003.pool.root.1,/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Evgen_E2E/mc15_13TeV/EVNT.25508216._000001.pool.root.1' )

# CA Config
atlas_add_citest( Generation_CA_Sherpa_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid Test421001 -e '--CA True' )

atlas_add_citest( Generation_CA_MGPy8_ttbar_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid Test421007 -e '--CA True' )

atlas_add_citest( Generation_CA_MGPy8_toponium_fromLHE_13TeV
   SCRIPT RunWorkflowTests_Run2.py --CI -g --dsid Test802381 -e '--CA True --inputGeneratorFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Generators/mc15_13TeV.440655.toponium_singlet_2l.evgen.TXT.e0000/TXT.440655._001077.tar.gz' )

atlas_add_citest( Generation_CA_H7_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid Test421106 -e '--CA True' )

atlas_add_citest( Generation_CA_Py8_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid Test421113 -e '--CA True' )

atlas_add_citest( Generation_CA_PhPy8_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid Test950070 -e '--CA True' )

atlas_add_citest( Generation_CA_P8B_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid Test801918 -e '--CA True' )

atlas_add_citest( Generation_CA_ParticleGun_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid Test950555 -e '--CA True' )

atlas_add_citest( Generation_CA_MG_LHE_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid Test950807 -e '--CA True --outputTXTFile events.lhe' )

atlas_add_citest( Generation_CA_Filters_13TeV
   SCRIPT RunWorkflowTests_Run2.py --CI -g --dsid Test421408 -e '--CA True --inputEVNT_PreFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Evgen_E2E/mc15_13TeV/EVNT.25508216._000003.pool.root.1,/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Evgen_E2E/mc15_13TeV/EVNT.25508216._000001.pool.root.1' )
