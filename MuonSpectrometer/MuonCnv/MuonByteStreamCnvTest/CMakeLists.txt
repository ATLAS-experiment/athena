# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonByteStreamCnvTest )

# Component(s) in the package:
atlas_add_component( MuonByteStreamCnvTest
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES MuonCablingData MuonCondData 
                                    MuonCSC_CnvToolsLib MuonMDT_CnvToolsLib MuonRPC_CnvToolsLib 
                                    MuonTGC_CnvToolsLib MuonSTGC_CnvToolsLib MuonMM_CnvToolsLib 
                                    AthenaBaseComps GaudiKernel MuonReadoutGeometry MuonRDO StoreGateLib 
                                    TrigT1RPChardwareLib TrigT1RPClogicLib TGCcablingInterfaceLib MuonIdHelpersLib 
                                    RPC_CondCablingLib MuonDigToolInterfacesLib CscCalibToolsLib MuonDigitContainer 
                                    NSWCalibToolsLib xAODMuonRDO EventInfoMgtLib)
