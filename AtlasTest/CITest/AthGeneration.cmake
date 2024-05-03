# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# CI test definitions for the AthGeneration project
# --> README.md before you modify this file
#

atlas_add_citest( DuplicateClass
   SCRIPT python -c 'import ROOT'
   PROPERTIES FAIL_REGULAR_EXPRESSION "class .* is already in" )

atlas_add_citest( Generation_PhPy8_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid 421356 )

atlas_add_citest( Generation_H7_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid 421106 )

atlas_add_citest( Generation_MGPy8_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid 421107 )

atlas_add_citest( Generation_Sherpa_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid 421003 )

atlas_add_citest( Generation_ParticleGun_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid 421119 )

atlas_add_citest( Generation_JetFilter_13p6TeV
   SCRIPT RunWorkflowTests_Run3.py --CI -g --dsid 421114 )

atlas_add_citest( Generation_PhPy8_13TeV
   SCRIPT RunWorkflowTests_Run2.py --CI -g --dsid 421356 )

atlas_add_citest( Generation_PhPy8_14TeV
   SCRIPT RunWorkflowTests_Run4.py --CI -g --dsid 421356 )
