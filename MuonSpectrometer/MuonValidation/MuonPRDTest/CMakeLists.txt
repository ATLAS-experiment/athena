# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonPRDTest )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( CLHEP )

atlas_add_library (MuonPRDTestLib
                   MuonPRDTest/*.h Root/*.cxx
                   PUBLIC_HEADERS MuonPRDTest
                   LINK_LIBRARIES AthenaBaseComps GeneratorObjects MuonDigitContainer MuonIdHelpersLib MuonReadoutGeometry MuonSimData MuonSimEvent 
                                  MuonPrepRawData MuonRDO MuonCSC_CnvToolsLib MuonTesterTreeLib StoreGateLib TrackRecordLib MuonTGC_CablingLib AsgAnalysisInterfaces
                   PRIVATE_LINK_LIBRARIES AthenaKernel AtlasHepMCLib )

# Component(s) in the package:
atlas_add_component( MuonPRDTest
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} MuonPRDTestLib AthenaKernel GaudiKernel  
                                    AthAnalysisBaseCompsLib xAODCutFlow PileupReweightingLib EventBookkeeperMetaData xAODMetaData MuonPattern)

# Install files from the package:
atlas_install_scripts( scripts/*.py )
atlas_install_xmls( data/*.xml )
atlas_install_python_modules( python/*.py )
