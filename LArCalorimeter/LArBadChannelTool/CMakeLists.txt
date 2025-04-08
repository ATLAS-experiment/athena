# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArBadChannelTool )

# External dependencies:
find_package( COOL COMPONENTS CoolKernel )
find_package( CORAL COMPONENTS CoralBase CoralKernel RelationalAccess )

# Component(s) in the package:
atlas_add_library( LArBadChannelToolLib
                   src/*.cxx
                   PUBLIC_HEADERS LArBadChannelTool
                   INCLUDE_DIRS ${COOL_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${COOL_LIBRARIES} ${CORAL_LIBRARIES} CaloIdentifier AthenaBaseComps AthenaKernel AthenaPoolUtilities Identifier GaudiKernel LArIdentifier LArRecConditions StoreGateLib LArCablingLib )

atlas_add_component( LArBadChannelTool
                     src/components/*.cxx
                     LINK_LIBRARIES LArBadChannelToolLib )

atlas_add_dictionary( BadChanDict
                      LArBadChannelTool/LArBadChannelDBToolsDict.h
                      LArBadChannelTool/selection.xml
                      LINK_LIBRARIES LArBadChannelToolLib )

atlas_add_test( LArBadChannelConfigTest
   SCRIPT python -m LArBadChannelTool.LArBadChannelConfig
   POST_EXEC_SCRIPT nopost.sh )


atlas_add_test ( LArBadFebWritingTest
  SCRIPT python -m LArBadChannelTool.LArMissingFebsDBAlg -o testFeb.db  -f /LAR/BadChannelsOfl/MissingFEBs -t LARBadChannelsOflMissingFEBs-RUN2-UPD4-02 -r 480000 ${CMAKE_CURRENT_SOURCE_DIR}/share/badfebs.txt
  POST_EXEC_SCRIPT noerror.sh )

atlas_add_test ( LArBadChanWritingTest
  SCRIPT python -m LArBadChannelTool.LArBadChannelDBAlg -o testChan.db -f /LAR/BadChannelsOfl/BadChannels -t  LARBadChannelsOflBadChannels-TEST -r 480000 ${CMAKE_CURRENT_SOURCE_DIR}/share/badchannels.txt
  POST_EXEC_SCRIPT noerror.sh )


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( share/LArBuildBadChannelDB.sh share/LArBuildMissingFebDB.sh )
