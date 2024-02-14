# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigTauMonitoring )

find_package( Boost )

atlas_add_library( TrigTauMonitoringLib
                   TrigTauMonitoring/*.h Root/*.cxx
                   PUBLIC_HEADERS TrigTauMonitoring
                   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} )

atlas_add_dictionary( TrigTauMonitoringDict
                      TrigTauMonitoring/TrigTauMonitoringDict.h TrigTauMonitoring/selection.xml
                      INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                      LINK_LIBRARIES TrigTauMonitoringLib ${Boost_LIBRARIES} )

# Component(s) in the package:
atlas_add_component( TrigTauMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES TrigTauMonitoringLib AthenaMonitoringKernelLib AthenaMonitoringLib CxxUtils xAODTau xAODTrigger xAODTruth LArRecEvent TruthUtils ${Boost_LIBRARIES} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
