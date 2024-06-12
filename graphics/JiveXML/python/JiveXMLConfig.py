# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format


def getATLASVersion():
    import os

    if "AtlasVersion" in os.environ:
        return os.environ["AtlasVersion"]
    if "AtlasBaseVersion" in os.environ:
        return os.environ["AtlasBaseVersion"]
    return "Unknown"


def getDataTypes(flags, haveRDO=False, readAOD=False):
    data_types = []  # These need to match the tools added later
    if flags.Detector.EnableID:
        # General ID types
        if flags.Detector.GeometryPixel and flags.Detector.GeometrySCT:
            data_types += ["JiveXML::SiSpacePointRetriever/SiSpacePointRetriever"]
            data_types += ["JiveXML::SiClusterRetriever/SiClusterRetriever"]
        data_types += ["JiveXML::VertexRetriever/VertexRetriever"]
        # These options will retrieve any available collection of segments/tracks from storegate
        data_types += ["JiveXML::SegmentRetriever/SegmentRetriever"]
        data_types += ["JiveXML::TrackRetriever/TrackRetriever"]

    if flags.Detector.EnablePixel and flags.Detector.GeometryPixel:
        data_types += ["JiveXML::PixelClusterRetriever/PixelClusterRetriever"]
        if haveRDO:
            data_types += ["JiveXML::PixelRDORetriever/PixelRDORetriever"]
    if flags.Detector.EnableTRT and flags.Detector.GeometryTRT:
        data_types += ["JiveXML::TRTRetriever/TRTRetriever"]
    if haveRDO and flags.Detector.EnableSCT and flags.Detector.GeometrySCT:
        data_types += ["JiveXML::SCTRDORetriever/SCTRDORetriever"]

    if not flags.OnlineEventDisplays.OfflineTest:
        # TODO this datatype is not yet understood by Atlantis
        data_types += ["JiveXML::BeamSpotRetriever/BeamSpotRetriever"]

    # Truth (from TruthJiveXML_DataTypes.py)
    if not readAOD:
        data_types += ["JiveXML::TruthTrackRetriever/TruthTrackRetriever"]
    data_types += ["JiveXML::TruthMuonTrackRetriever/TruthMuonTrackRetriever"]

    if flags.Detector.EnableCalo and flags.Detector.GeometryCalo:
        # Taken from CaloJiveXML_DataTypes.py
        # TODO find correct flag and check the LArDigitRetriever is doing what we want it to do
        #if doLArDigits:
            #data_types += ["JiveXML::LArDigitRetriever/LArDigitRetriever"]
        #else:
        if flags.Detector.EnableLAr and flags.Detector.GeometryLAr:
            data_types += ["JiveXML::CaloFCalRetriever/CaloFCalRetriever"]
            data_types += ["JiveXML::CaloHECRetriever/CaloHECRetriever"]
            data_types += ["JiveXML::CaloLArRetriever/CaloLArRetriever"]
        #end of else
        if flags.Detector.EnableMBTS and flags.Detector.GeometryMBTS:
            data_types += ["JiveXML::CaloMBTSRetriever/CaloMBTSRetriever"]
        if flags.Detector.EnableTile and flags.Detector.GeometryTile:
            data_types += ["JiveXML::CaloTileRetriever/CaloTileRetriever"]
        data_types += ["JiveXML::CaloClusterRetriever/CaloClusterRetriever"]

    if flags.Detector.EnableMuon and flags.Detector.GeometryMuon:
        # Taken from MuonJiveXML_DataTypes.py
        if flags.Detector.EnableMDT and flags.Detector.GeometryMDT:
            data_types += ["JiveXML::MdtPrepDataRetriever/MdtPrepDataRetriever"]
        if flags.Detector.EnableTGC and flags.Detector.GeometryTGC:
            data_types += ["JiveXML::TgcPrepDataRetriever/TgcPrepDataRetriever"]
            data_types += ["JiveXML::sTgcPrepDataRetriever/sTgcPrepDataRetriever"]
        if flags.Detector.EnableRPC and flags.Detector.GeometryRPC:
            data_types += ["JiveXML::RpcPrepDataRetriever/RpcPrepDataRetriever"]
        if flags.Detector.EnableCSC and flags.Detector.GeometryCSC:
            data_types += ["JiveXML::CSCClusterRetriever/CSCClusterRetriever"]
            data_types += ["JiveXML::CscPrepDataRetriever/CscPrepDataRetriever"]
        if flags.Detector.EnableMM and flags.Detector.GeometryMM:
            data_types += ["JiveXML::MMPrepDataRetriever/MMPrepDataRetriever"]
        # TODO Not sure if below are still needed?
        # data_types += ["JiveXML::TrigMuonROIRetriever/TrigMuonROIRetriever"]
        # data_types += ["JiveXML::MuidTrackRetriever/MuidTrackRetriever]
        # data_types += ["JiveXML::TrigRpcDataRetriever/TrigRpcDataRetriever"]

    # Taken from xAODJiveXML_DataTypes.py
    data_types += ["JiveXML::xAODCaloClusterRetriever/xAODCaloClusterRetriever"]
    data_types += ["JiveXML::xAODElectronRetriever/xAODElectronRetriever"]
    data_types += ["JiveXML::xAODMissingETRetriever/xAODMissingETRetriever"]
    data_types += ["JiveXML::xAODMuonRetriever/xAODMuonRetriever"]
    data_types += ["JiveXML::xAODPhotonRetriever/xAODPhotonRetriever"]
    data_types += ["JiveXML::xAODJetRetriever/xAODJetRetriever"]
    data_types += ["JiveXML::xAODTauRetriever/xAODTauRetriever"]
    data_types += ["JiveXML::xAODTrackParticleRetriever/xAODTrackParticleRetriever"]
    data_types += ["JiveXML::xAODVertexRetriever/xAODVertexRetriever"]

    if flags.Reco.EnableTrigger:
        # Taken from TrigJiveXML_DataTypes.py
        data_types += ["JiveXML::LVL1ResultRetriever/LVL1ResultRetriever"]
        data_types += ["JiveXML::TriggerInfoRetriever/TriggerInfoRetriever"]
        data_types += ["JiveXML::xAODEmTauROIRetriever/xAODEmTauROIRetriever"]
        data_types += ["JiveXML::xAODJetROIRetriever/xAODJetROIRetriever"]
        data_types += ["JiveXML::xAODMuonROIRetriever/xAODMuonROIRetriever"]
        data_types += ["JiveXML::xAODTriggerTowerRetriever/xAODTriggerTowerRetriever"]

    return data_types


def AlgoJiveXMLCfg(flags, name="AlgoJiveXML", **kwargs):
    # This is based on a few old-style configuation files:
    # JiveXML_RecEx_config.py
    # JiveXML_jobOptionBase.py
    result = ComponentAccumulator()

    kwargs.setdefault("AtlasRelease", getATLASVersion())
    kwargs.setdefault("WriteToFile", True)
    ### Enable this to recreate the geometry XML files for Atlantis
    kwargs.setdefault("WriteGeometry", False)
    kwargs.setdefault("StreamToServerTool", None)

    # This next bit sets the data types, then we set the associated public tools
    readAOD = False  # FIXME - set this properly
    readESD = False  # FIXME - set this properly
    haveRDO = (
        flags.Input.Format is Format.BS or "StreamRDO" in flags.Input.ProcessingTags
    )
    kwargs.setdefault("DataTypes", getDataTypes(flags, haveRDO, readAOD))

    if not readAOD:
        from JiveXML.TruthTrackRetrieverConfig import TruthTrackRetrieverCfg
        result.merge(TruthTrackRetrieverCfg(flags))

    if haveRDO or readESD:
        if flags.Detector.EnableID and flags.Detector.GeometryID:
            from JiveXML.InDetRetrieversConfig import InDetRetrieversCfg
            result.merge(InDetRetrieversCfg(flags))

        if flags.Detector.EnableCalo and flags.Detector.GeometryCalo:
            from JiveXML.CaloRetrieversConfig import CaloRetrieversCfg
            result.merge(CaloRetrieversCfg(flags))

        if flags.Detector.EnableMuon and flags.Detector.GeometryMuon:
            from JiveXML.MuonRetrieversConfig import MuonRetrieversCfg
            result.merge(MuonRetrieversCfg(flags))

    from JiveXML.xAODRetrieversConfig import xAODRetrieversCfg
    result.merge(xAODRetrieversCfg(flags))

    from JiveXML.TriggerRetrieversConfig import TriggerRetrieversCfg
    result.merge(TriggerRetrieversCfg(flags))

    the_alg = CompFactory.JiveXML.AlgoJiveXML(name="AlgoJiveXML", **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result
