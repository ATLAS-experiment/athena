#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def PrimaryMeasContNamesCfg(flags):
    Containers =[]
    if flags.Detector.EnableMDT: 
        Containers+=["xMdtDriftCircles"]       
        Containers+=["xMdtTwinDriftCircles"]       
    from AthenaConfiguration.Enums import LHCPeriod
    if flags.Detector.EnableRPC: 
        if flags.GeoModel.Run >= LHCPeriod.Run4:
            Containers+=["xRpcStrips"] 
            Containers+=["xRpcBILStrips"] 
        else:
            Containers+=[ "xRpcMeasurements"]
    if flags.Detector.EnableTGC: 
        Containers+=["xTgcStrips"] 
    if flags.Detector.EnableMM:
        Containers+=["xAODMMClusters"] 
    if flags.Detector.EnablesTGC: 
        Containers+=["xAODsTgcStrips"] 
        Containers+=["xAODsTgcWires"] 
        Containers+=["xAODsTgcPads"] 
    return Containers
### Configuriation snippet to schedule all algorithms providing the 
### Uncalibrated measurements
def xAODUncalibMeasPrepCfg(flags):
    result = ComponentAccumulator()
    if not flags.Muon.usePhaseIIGeoSetup:
        return result

    if flags.Input.isMC:
        from MuonConfig.MuonSimHitToRdoConfig import MuonSimHitToRdoCnvCfg
        result.merge(MuonSimHitToRdoCnvCfg(flags))
    else:
        from MuonConfig.MuonBytestreamDecodeConfig import MuonByteStreamDecodersCfg
        result.merge(MuonByteStreamDecodersCfg(flags))
    from MuonConfig.MuonRdoDecodeConfig import MuonRDOtoPRDConvertorsCfg
    result.merge(MuonRDOtoPRDConvertorsCfg(flags))

    ### Schedule the truth segment maker & truth hit association
    if flags.Input.isMC:
        from MuonTruthAlgsR4.MuonTruthAlgsConfig import MuonTruthAlgsCfg
        result.merge(MuonTruthAlgsCfg(flags))
        from MuonObjectMarker.ObjectMarkerConfig import TruthMeasMarkerAlgCfg
        result.merge(TruthMeasMarkerAlgCfg(flags))
    return result