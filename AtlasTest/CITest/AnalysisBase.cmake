# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# CI test definitions for the AnalysisBase project
# --> README.md before you modify this file
#
#################################################################################
# CP Algorithms
#################################################################################
atlas_add_citest( CPAlgorithmsRun2Data_PHYS_Block
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --run 2 --no-systematics --direct-driver )
atlas_add_citest( CPAlgorithmsRun2Data_PHYS_Text
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --run 2 --no-systematics --text-config AnalysisAlgorithmsConfig/test_configuration_Run2.yaml )
atlas_add_citest( CPAlgorithmsRun2Data_PHYS_Comparison
   SCRIPT compareFlatTrees --require-same-branches analysis ../CPAlgorithmsRun2Data_PHYS_Block/submitDir/data-ANALYSIS/data.root ../CPAlgorithmsRun2Data_PHYS_Text/submitDir/data-ANALYSIS/data.root
   DEPENDS_SUCCESS CPAlgorithmsRun2Data_PHYS_Block CPAlgorithmsRun2Data_PHYS_Text )

atlas_add_citest( CPAlgorithmsRun3Data_PHYS_Block
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --run 3 --no-systematics --direct-driver )
atlas_add_citest( CPAlgorithmsRun3Data_PHYS_Text
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --run 3 --no-systematics --text-config AnalysisAlgorithmsConfig/test_configuration_Run3.yaml )
atlas_add_citest( CPAlgorithmsRun3Data_PHYS_Comparison
   SCRIPT compareFlatTrees --require-same-branches analysis ../CPAlgorithmsRun3Data_PHYS_Block/submitDir/data-ANALYSIS/data.root ../CPAlgorithmsRun3Data_PHYS_Text/submitDir/data-ANALYSIS/data.root
   DEPENDS_SUCCESS CPAlgorithmsRun3Data_PHYS_Block CPAlgorithmsRun3Data_PHYS_Text )

atlas_add_citest( CPAlgorithmsRun2FullSim_PHYS_Block
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fullsim --run 2 --direct-driver )
atlas_add_citest( CPAlgorithmsRun2FullSim_PHYS_Text
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fullsim --run 2 --text-config AnalysisAlgorithmsConfig/test_configuration_Run2.yaml )
atlas_add_citest( CPAlgorithmsRun2FullSim_PHYS_Comparison
   SCRIPT compareFlatTrees --require-same-branches analysis ../CPAlgorithmsRun2FullSim_PHYS_Block/submitDir/data-ANALYSIS/fullsim.root ../CPAlgorithmsRun2FullSim_PHYS_Text/submitDir/data-ANALYSIS/fullsim.root
   DEPENDS_SUCCESS CPAlgorithmsRun2FullSim_PHYS_Block CPAlgorithmsRun2FullSim_PHYS_Text )

atlas_add_citest( CPAlgorithmsRun3FullSim_PHYS_Block
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fullsim --run 3 --direct-driver )
atlas_add_citest( CPAlgorithmsRun3FullSim_PHYS_Text
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fullsim --run 3 --text-config AnalysisAlgorithmsConfig/test_configuration_Run3.yaml )
atlas_add_citest( CPAlgorithmsRun3FullSim_PHYS_Comparison
   SCRIPT compareFlatTrees --require-same-branches analysis ../CPAlgorithmsRun3FullSim_PHYS_Block/submitDir/data-ANALYSIS/fullsim.root ../CPAlgorithmsRun3FullSim_PHYS_Text/submitDir/data-ANALYSIS/fullsim.root
   DEPENDS_SUCCESS CPAlgorithmsRun3FullSim_PHYS_Block CPAlgorithmsRun3FullSim_PHYS_Text )

atlas_add_citest( CPAlgorithmsRun2FastSim_PHYS_Block
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fastsim --run 2 --direct-driver )
atlas_add_citest( CPAlgorithmsRun2FastSim_PHYS_Text
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fastsim --run 2 --text-config AnalysisAlgorithmsConfig/test_configuration_Run2.yaml )
atlas_add_citest( CPAlgorithmsRun2FastSim_PHYS_Comparison
   SCRIPT compareFlatTrees --require-same-branches analysis ../CPAlgorithmsRun2FastSim_PHYS_Block/submitDir/data-ANALYSIS/fastsim.root ../CPAlgorithmsRun2FastSim_PHYS_Text/submitDir/data-ANALYSIS/fastsim.root
   DEPENDS_SUCCESS CPAlgorithmsRun2FastSim_PHYS_Block CPAlgorithmsRun2FastSim_PHYS_Text )

atlas_add_citest( CPAlgorithmsRun3FastSim_PHYS_Block
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fastsim --run 3 --direct-driver )
atlas_add_citest( CPAlgorithmsRun3FastSim_PHYS_Text
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fastsim --run 3 --text-config AnalysisAlgorithmsConfig/test_configuration_Run3.yaml )
atlas_add_citest( CPAlgorithmsRun3FastSim_PHYS_Comparison
   SCRIPT compareFlatTrees --require-same-branches analysis ../CPAlgorithmsRun3FastSim_PHYS_Block/submitDir/data-ANALYSIS/fastsim.root ../CPAlgorithmsRun3FastSim_PHYS_Text/submitDir/data-ANALYSIS/fastsim.root
   DEPENDS_SUCCESS CPAlgorithmsRun3FastSim_PHYS_Block CPAlgorithmsRun3FastSim_PHYS_Text )

atlas_add_citest( CPAlgorithmsRun2Data_PHYSLITE_Block
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --run 2 --physlite --no-systematics --direct-driver )
atlas_add_citest( CPAlgorithmsRun2Data_PHYSLITE_Text
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --run 2 --physlite --no-systematics --text-config AnalysisAlgorithmsConfig/test_configuration_Run2.yaml )
atlas_add_citest( CPAlgorithmsRun2Data_PHYSLITE_Comparison
   SCRIPT compareFlatTrees --require-same-branches analysis ../CPAlgorithmsRun2Data_PHYSLITE_Block/submitDir/data-ANALYSIS/data.root ../CPAlgorithmsRun2Data_PHYSLITE_Text/submitDir/data-ANALYSIS/data.root
   DEPENDS_SUCCESS CPAlgorithmsRun2Data_PHYSLITE_Block CPAlgorithmsRun2Data_PHYSLITE_Text )

atlas_add_citest( CPAlgorithmsRun3Data_PHYSLITE_Block
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --run 3 --physlite --no-systematics --direct-driver )
atlas_add_citest( CPAlgorithmsRun3Data_PHYSLITE_Text
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --run 3 --physlite --no-systematics --text-config AnalysisAlgorithmsConfig/test_configuration_Run3.yaml )
atlas_add_citest( CPAlgorithmsRun3Data_PHYSLITE_Comparison
   SCRIPT compareFlatTrees --require-same-branches analysis ../CPAlgorithmsRun3Data_PHYSLITE_Block/submitDir/data-ANALYSIS/data.root ../CPAlgorithmsRun3Data_PHYSLITE_Text/submitDir/data-ANALYSIS/data.root
   DEPENDS_SUCCESS CPAlgorithmsRun3Data_PHYSLITE_Block CPAlgorithmsRun3Data_PHYSLITE_Text )

atlas_add_citest( CPAlgorithmsRun2FullSim_PHYSLITE_Block
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fullsim --run 2 --physlite --direct-driver )
atlas_add_citest( CPAlgorithmsRun2FullSim_PHYSLITE_Text
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fullsim --run 2 --physlite --text-config AnalysisAlgorithmsConfig/test_configuration_Run2.yaml )
atlas_add_citest( CPAlgorithmsRun2FullSim_PHYSLITE_Comparison
   SCRIPT compareFlatTrees --require-same-branches analysis ../CPAlgorithmsRun2FullSim_PHYSLITE_Block/submitDir/data-ANALYSIS/fullsim.root ../CPAlgorithmsRun2FullSim_PHYSLITE_Text/submitDir/data-ANALYSIS/fullsim.root
   DEPENDS_SUCCESS CPAlgorithmsRun2FullSim_PHYSLITE_Block CPAlgorithmsRun2FullSim_PHYSLITE_Text )

atlas_add_citest( CPAlgorithmsRun3FullSim_PHYSLITE_Block
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fullsim --run 3 --physlite --direct-driver )
atlas_add_citest( CPAlgorithmsRun3FullSim_PHYSLITE_Text
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fullsim --run 3 --physlite --text-config AnalysisAlgorithmsConfig/test_configuration_Run3.yaml )
atlas_add_citest( CPAlgorithmsRun3FullSim_PHYSLITE_Comparison
   SCRIPT compareFlatTrees --require-same-branches analysis ../CPAlgorithmsRun3FullSim_PHYSLITE_Block/submitDir/data-ANALYSIS/fullsim.root ../CPAlgorithmsRun3FullSim_PHYSLITE_Text/submitDir/data-ANALYSIS/fullsim.root
   DEPENDS_SUCCESS CPAlgorithmsRun3FullSim_PHYSLITE_Block CPAlgorithmsRun3FullSim_PHYSLITE_Text )

atlas_add_citest( CPAlgorithmsRun2FastSim_PHYSLITE_Block
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fastsim --run 2 --physlite --direct-driver )
atlas_add_citest( CPAlgorithmsRun2FastSim_PHYSLITE_Text
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fastsim --run 2 --physlite --text-config AnalysisAlgorithmsConfig/test_configuration_Run2.yaml )
atlas_add_citest( CPAlgorithmsRun2FastSim_PHYSLITE_Comparison
   SCRIPT compareFlatTrees --require-same-branches analysis ../CPAlgorithmsRun2FastSim_PHYSLITE_Block/submitDir/data-ANALYSIS/fastsim.root ../CPAlgorithmsRun2FastSim_PHYSLITE_Text/submitDir/data-ANALYSIS/fastsim.root
   DEPENDS_SUCCESS CPAlgorithmsRun2FastSim_PHYSLITE_Block CPAlgorithmsRun2FastSim_PHYSLITE_Text )

atlas_add_citest( CPAlgorithmsRun3FastSim_PHYSLITE_Block
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fastsim --run 3 --physlite --direct-driver )
atlas_add_citest( CPAlgorithmsRun3FastSim_PHYSLITE_Text
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fastsim --run 3 --physlite --text-config AnalysisAlgorithmsConfig/test_configuration_Run3.yaml )
atlas_add_citest( CPAlgorithmsRun3FastSim_PHYSLITE_Comparison
   SCRIPT compareFlatTrees --require-same-branches analysis ../CPAlgorithmsRun3FastSim_PHYSLITE_Block/submitDir/data-ANALYSIS/fastsim.root ../CPAlgorithmsRun3FastSim_PHYSLITE_Text/submitDir/data-ANALYSIS/fastsim.root
   DEPENDS_SUCCESS CPAlgorithmsRun3FastSim_PHYSLITE_Block CPAlgorithmsRun3FastSim_PHYSLITE_Text )

# special test for only nominal overlap removal
atlas_add_citest( CPAlgorithmsRun2FullSim_PHYS_OnlyNominalOR
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fullsim --run 2 --only-nominal-or --direct-driver )
atlas_add_citest( CPAlgorithmsRun3FastSim_PHYSLITE_OnlyNominalOR
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type fastsim --run 3 --physlite --only-nominal-or --direct-driver )

# this test is for testing that the algorithm monitors defined in EventLoop
# don't break a job of reasonable complexity.  they are tested here instead of
# in the EventLoop package, because we have a much more complex payload here.
atlas_add_citest( CPAlgorithmsRun2Data_PHYS_Benchmark
   SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --run 2 --no-systematics --direct-driver --algorithm-timer --algorithm-memory )


#################################################################################
# SUSYTools
#################################################################################

atlas_add_citest( SUSYTools_data18_PHYS
   SCRIPT "SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/data18_13TeV.39757132_p6266.PHYS.pool.root maxEvents=500 isData=1 isAtlfast=0 Debug=0"
   )

atlas_add_citest( SUSYTools_data22_PHYS
   SCRIPT "SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/data22_13p6TeV.39672246_p6269.PHYS.pool.root maxEvents=1000 isData=1 isAtlfast=0 Debug=0"
   )

atlas_add_citest( SUSYTools_mc20e_PHYS
   SCRIPT "SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/DAOD_PHYS.mc20_13TeV.410470.FS_mc20e_p6266.PHYS.pool.root maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0 ilumicalcFile=GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root"
   )

atlas_add_citest( SUSYTools_mc23a_PHYS
   SCRIPT "SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/mc23_13p6TeV.601229.FS_mc23a_p6266.PHYS.pool.root maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0 ilumicalcFile=GoodRunsLists/data22_13p6TeV/20230116/ilumicalc_histograms_None_431810-440613_OflLumi-Run3-002.root"
   )

atlas_add_citest( SUSYTools_mc23a_PHYSLITE
   SCRIPT "SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/mc23_13p6TeV.601229.FS_mc23a_p6266.PHYSLITE.pool.root maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0 ilumicalcFile=GoodRunsLists/data22_13p6TeV/20230116/ilumicalc_histograms_None_431810-440613_OflLumi-Run3-002.root"
   )
