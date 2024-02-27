# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# CI test definitions for the AnalysisBase project
# --> README.md before you modify this file
#

atlas_add_citest( AnalysisTop_EMPFlowData
   LOG_IGNORE_PATTERN "Cannot interpolate outside histogram domain" # ANALYSISTO-1165
   SCRIPT CI_EMPFlowDatatest.py )

atlas_add_citest( AnalysisTop_EMPFlowMC
   SCRIPT CI_EMPFlowMCtest.py )

atlas_add_citest( AnalysisTop_EMPFlowData22
   SCRIPT CI_EMPFlowData22test.py )

atlas_add_citest( AnalysisTop_EMPFlowMC21
   SCRIPT CI_EMPFlowMC21test.py )

#################################################################################
# SUSYTools
#################################################################################

atlas_add_citest( SUSYTools_data18_PHYS
   SCRIPT "SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/data18_13TeV.00356250_p5855.PHYS.pool.root maxEvents=500 isData=1 isAtlfast=0 Debug=0"
   )

atlas_add_citest( SUSYTools_data22_PHYS
   SCRIPT "SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/data22_13p6TeV.00440543_p5858.PHYS.pool.root maxEvents=1000 isData=1 isAtlfast=0 Debug=0"
   )

atlas_add_citest( SUSYTools_mc20e_PHYS
   SCRIPT "SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/mc20_13TeV.410470.FS_mc20e_p5855.PHYS.pool.root maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0 ilumicalcFile=GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root"
   )

atlas_add_citest( SUSYTools_mc23a_PHYS
   SCRIPT "SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/mc23_13p6TeV.601229.FS_mc23a_p5855.PHYS.pool.root maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0 ilumicalcFile=GoodRunsLists/data22_13p6TeV/20230116/ilumicalc_histograms_None_431810-440613_OflLumi-Run3-002.root"
   )

atlas_add_citest( SUSYTools_mc23a_PHYSLITE
   SCRIPT "SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/mc23_13p6TeV.601229.FS_mc23a_p5855.PHYSLITE.pool.root maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0 ilumicalcFile=GoodRunsLists/data22_13p6TeV/20230116/ilumicalc_histograms_None_431810-440613_OflLumi-Run3-002.root"
   )