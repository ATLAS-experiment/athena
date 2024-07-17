# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonPatternCnv )




# Component(s) in the package:
atlas_add_component( MuonPatternCnv
                     src/*.cxx
                     src/components/*.cxx                  
                     LINK_LIBRARIES GaudiKernel AthenaBaseComps StoreGateLib MuonPatternEvent MuonPrepRawData
                                    MuonRecToolInterfaces MuonLayerHough MuonIdHelpersLib MuonPattern )

atlas_install_python_modules( python/*.py)