# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigTauMonitoring )

atlas_add_library( TrigTauMonitoringLib
                   TrigTauMonitoring/*.h Root/*.cxx
                   PUBLIC_HEADERS TrigTauMonitoring
)

atlas_add_dictionary( TrigTauMonitoringDict
                      TrigTauMonitoring/TrigTauMonitoringDict.h TrigTauMonitoring/selection.xml
                      LINK_LIBRARIES TrigTauMonitoringLib )

# Component(s) in the package:
atlas_add_component( TrigTauMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES TrigTauMonitoringLib AthenaMonitoringKernelLib AthenaMonitoringLib CxxUtils xAODTau xAODTrigger xAODTruth LArRecEvent TruthUtils )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
