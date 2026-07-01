# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonMDT_CnvToolsR4 )

# Component(s) in the package:
atlas_add_component( MuonMDT_CnvToolsR4
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES MuonMDT_CnvToolsLib GaudiKernel AthenaBaseComps AthenaKernel StoreGateLib 
                                    MuonCondData MuonCablingData MuonReadoutGeometryR4 MuonIdHelpersLib MuonRDO xAODMuonViews  
                                    MuonCnvToolInterfacesLib xAODMuonPrepData MdtCalibInterfacesLib MuonDigitContainer)
