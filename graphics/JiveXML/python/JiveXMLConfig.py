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

    if flags.Detector.EnablePixel:
        data_types += ["JiveXML::PixelClusterRetriever/PixelClusterRetriever"]
        if haveRDO:
            data_types += ["JiveXML::PixelRDORetriever/PixelRDORetriever"]
    if flags.Detector.EnableTRT:
        data_types += ["JiveXML::TRTRetriever/TRTRetriever"]
    if haveRDO and flags.Detector.EnableSCT:
        data_types += ["JiveXML::SCTRDORetriever/SCTRDORetriever"]

    # TODO this datatype is not yet understood by Atlantis
    data_types += ["JiveXML::BeamSpotRetriever/BeamSpotRetriever"]

    # Truth (from TruthJiveXML_DataTypes.py)
    if not readAOD:
        data_types += ["JiveXML::TruthTrackRetriever/TruthTrackRetriever"]
    data_types += ["JiveXML::TruthMuonTrackRetriever/TruthMuonTrackRetriever"]

    if flags.Detector.EnableCalo:
        # Taken from CaloJiveXML_DataTypes.py
        # TODO find correct flag and check the LArDigitRetriever is doing what we want it to do
        #if doLArDigits:
       # data_types += ["JiveXML::LArDigitRetriever/LArDigitRetriever"]
        #else:
        data_types += ["JiveXML::CaloFCalRetriever/CaloFCalRetriever"]
        data_types += ["JiveXML::CaloLArRetriever/CaloLArRetriever"]
        data_types += ["JiveXML::CaloHECRetriever/CaloHECRetriever"]
        #end of else
        data_types += ["JiveXML::CaloMBTSRetriever/CaloMBTSRetriever"]
        data_types += ["JiveXML::CaloTileRetriever/CaloTileRetriever"]
        data_types += ["JiveXML::CaloClusterRetriever/CaloClusterRetriever"]

    if flags.Detector.EnableMuon:
        # Taken from MuonJiveXML_DataTypes.py
        if flags.Detector.EnableMDT:
            data_types += ["JiveXML::MdtPrepDataRetriever/MdtPrepDataRetriever"]
        if flags.Detector.EnableTGC:
            data_types += ["JiveXML::TgcPrepDataRetriever/TgcPrepDataRetriever"]
        if flags.Detector.EnableRPC:
            data_types += ["JiveXML::RpcPrepDataRetriever/RpcPrepDataRetriever"]
        if flags.Detector.EnableCSC:
            data_types += ["JiveXML::CSCClusterRetriever/CSCClusterRetriever"]
            data_types += ["JiveXML::CscPrepDataRetriever/CscPrepDataRetriever"]
        if flags.Detector.EnablesTGC:
            data_types += ["JiveXML::sTgcPrepDataRetriever/sTgcPrepDataRetriever"]
        if flags.Detector.EnableMM:
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


def InDetRetrieversCfg(flags):
    result = ComponentAccumulator()
    # Do we need to add equivalent of InDetFlags.doSlimming=False (in JiveXML_RecEx_config.py)? If so, why?
    # Following is based on InDetJiveXML_DataTypes.py and TrkJiveXML_DataTypes.py
    if flags.Detector.EnablePixel:
        result.merge(PixelClusterRetrieverCfg(flags))

    if flags.Detector.EnableID:
        if flags.Detector.GeometryPixel and flags.Detector.GeometrySCT:
            result.merge(SiClusterRetrieverCfg(flags))
            result.merge(SiSpacePointRetrieverCfg(flags))
        result.merge(TrackRetrieverCfg(flags))

    if flags.Detector.EnableTRT:
        result.merge(TRTRetrieverCfg(flags))

    result.merge(BeamSpotRetrieverCfg(flags))

    return result


def PixelClusterRetrieverCfg(flags, name="PixelClusterRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("PixelTruthMap", "")
    the_tool = CompFactory.JiveXML.PixelClusterRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def SiClusterRetrieverCfg(flags, name="SiClusterRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("SCT_TruthMap", "")
    the_tool = CompFactory.JiveXML.SiClusterRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def SiSpacePointRetrieverCfg(flags, name="SiSpacePointRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("PRD_TruthPixel", "")
        kwargs.setdefault("PRD_TruthSCT", "")
    the_tool = CompFactory.JiveXML.SiSpacePointRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def TRTRetrieverCfg(flags, name="TRTRetriever", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        kwargs.setdefault("TRTTruthMap", "")
    the_tool = CompFactory.JiveXML.TRTRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def TrackRetrieverCfg(flags, name="TrackRetriever", **kwargs):
    # Based on TrkJiveXML_DataTypes
    result = ComponentAccumulator()
    # FIXME - this is copied from TrkJiveXML_DataTypes.py, but we can do better
    kwargs.setdefault("PriorityTrackCollection", "Tracks")
    kwargs.setdefault(
        "OtherTrackCollections",
        [
            "CombinedMuonTracks",
            "MuonSpectrometerTracks",
            "ConvertedStacoTracks",
            "ConvertedMuIdCBTracks",
            "CombinedInDetTracks",
            "GSFTracks",
        ],
    )
    ### The Event Filter track collections are not written to XML by default.
    ### To write them out, you must uncomment the following line:
    # kwargs.setdefault("DoWriteHLT", True)
    ### switch residual data off:
    kwargs.setdefault("DoWriteResiduals", False)
    if flags.Common.isOnline:    
        doHitsDetails = False
    else:
        doHitsDetails = True
    kwargs.setdefault("DoHitsDetails", doHitsDetails)
    the_tool = CompFactory.JiveXML.TrackRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def BeamSpotRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.BeamSpotRetriever(name="BeamSpotRetriever", **kwargs)
    result.addPublicTool(the_tool)
    return result


def TruthTrackRetrieverCfg(flags, name="TruthTrackRetriever", **kwargs):
    # Based on TruthJiveXML_DataTypes.py (and JiveXML_RecEx_config.py)
    result = ComponentAccumulator()
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg

    extrap = result.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
    result.addPublicTool(extrap)

    kwargs.setdefault("StoreGateKey", "TruthEvent")
    the_tool = CompFactory.JiveXML.TruthTrackRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result


def CaloRetrieversCfg(flags, **kwargs):
    result = ComponentAccumulator()
    from LArRecUtils.LArADC2MeVCondAlgConfig import LArADC2MeVCondAlgCfg
    result.merge(LArADC2MeVCondAlgCfg (flags))

    rawChannelContainer = flags.Tile.RawChannelContainer
    digitsContainer = "TileDigitsCnt"
    if "TileDigitsFlt" in flags.Input.Collections:
        digitsContainer = "TileDigitsFlt"

    result.addPublicTool(
        CompFactory.JiveXML.CaloClusterRetriever(name = "CaloClusterRetriever",**kwargs
        )
    )
    
    if flags.Common.isOnline:
        doTileCellDetails = True
        doTileDigit = True
    else:
        doTileCellDetails = False
        doTileDigit = False
    result.addPublicTool(
        CompFactory.JiveXML.CaloTileRetriever(
            name = "CaloTileRetriever",
            TileDigitsContainer = digitsContainer,
            TileRawChannelContainer = rawChannelContainer,
            DoTileCellDetails = doTileCellDetails,
            DoTileDigit = doTileDigit,
            DoBadTile = False,
        )
    )

    if flags.Common.isOnline:
        doMBTSDigits = True
        doMBTSCellDetails = True
    else:
        doMBTSDigits = False
        doMBTSCellDetails = False

    result.addPublicTool(
        CompFactory.JiveXML.CaloMBTSRetriever(
            name = "CaloMBTSRetriever",
            TileDigitsContainer= digitsContainer,
            TileRawChannelContainer = rawChannelContainer,
            DoMBTSDigits = doMBTSDigits,
            DoMBTSCellDetails = doMBTSCellDetails,
        )
    )

    result.addPublicTool(
        CompFactory.JiveXML.CaloFCalRetriever(
            name = "CaloFCalRetriever",
            DoFCalCellDetails = False,
            DoBadFCal = False,
        )
    )

    result.addPublicTool(
        CompFactory.JiveXML.CaloLArRetriever(
            name = "CaloLArRetriever",
            DoLArCellDetails = False,
            DoBadLAr = False,
        )
    )

    result.addPublicTool(
        CompFactory.JiveXML.CaloHECRetriever(
            name = "CaloHECRetriever",
            DoHECCellDetails = False,
            DoBadHEC = False,
        )
    )

    result.addPublicTool(
        CompFactory.JiveXML.LArDigitRetriever(
            name = "LArDigitRetriever",
            DoLArDigit = False,
            DoHECDigit = False,
            DoFCalDigit = False,
        )
    )
    return result


def MuonRetrieversCfg(flags, **kwargs):
    result = ComponentAccumulator()
    # TODO add in other retrievers
    # Based on MuonJiveXML_DataTypes.py
    #kwargs.setdefault("StoreGateKey", "MDT_DriftCircles")

    result.addPublicTool(
        CompFactory.JiveXML.MdtPrepDataRetriever(name = "MdtPrepDataRetriever",**kwargs
        )
    )
    return result


def xAODRetrieversCfg(flags):
    # Based on xAODJiveXML_DataTypes.py
    result = ComponentAccumulator()
    # It's not really necessary to configure these, since nothing depends on flags.
    # We could just make all this the default in cpp (if it is not already)
    result.addPublicTool(
        CompFactory.JiveXML.xAODElectronRetriever(
            name="xAODElectronRetriever",
            StoreGateKey="Electrons",
            OtherCollections=["Electrons"],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODMissingETRetriever(
            name="xAODMissingETRetriever",
            FavouriteMETCollection="MET_Reference_AntiKt4EMPFlow",
            OtherMETCollections=[
                "MET_Reference_AntiKt4EMTopo",
                "MET_Calo",
                "MET_LocHadTopo",
                "MET_Core_AntiKt4LCTopo",
            ],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODMuonRetriever(
            name="xAODMuonRetriever", StoreGateKey="Muons", OtherCollections=["Muons"]
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODPhotonRetriever(
            name="xAODPhotonRetriever",
            StoreGateKey="Photons",
            OtherCollections=["Photons"],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODJetRetriever(
            name="xAODJetRetriever",
            FavouriteJetCollection="AntiKt4EMPFlowJets",
            OtherJetCollections=[
                "AntiKt4EMTopoJets",
                "AntiKt4LCTopoJets",
                "AntiKt10LCTopoJets",
            ],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODTauRetriever(
            name="xAODTauRetriever", StoreGateKey="TauJets"
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODTrackParticleRetriever(
            name="xAODTrackParticleRetriever",
            StoreGateKey="InDetTrackParticles",
            OtherTrackCollections=[
                "InDetLargeD0TrackParticles",
                "CombinedMuonTrackParticles",
                "GSFTrackParticles",
            ],
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODVertexRetriever(
            name="xAODVertexRetriever",
            PrimaryVertexCollection="PrimaryVertices",
            SecondaryVertexCollection="BTagging_AntiKt2TrackSecVtx",
        )
    )
    result.addPublicTool(
        CompFactory.JiveXML.xAODCaloClusterRetriever(
            name="xAODCaloClusterRetriever",
            FavouriteClusterCollection="egammaClusters",
            OtherClusterCollections=["CaloCalTopoClusters"],
        )
    )
    return result


def TriggerRetrieversCfg(flags):
    result = ComponentAccumulator()
    if flags.Reco.EnableTrigger:
        #--- LVL1 result from TrigDecision
        result.addPublicTool(
            CompFactory.JiveXML.LVL1ResultRetriever(
                name="LVL1ResultRetriever"
            )
        )

        #--- TriggerInfo (Etmiss, etc)
        result.addPublicTool(
            CompFactory.JiveXML.TriggerInfoRetriever(
                name="TriggerInfoRetriever" 
            )
        )

        # new xAOD retrievers
        result.addPublicTool(
            CompFactory.JiveXML.xAODEmTauROIRetriever(
                name="xAODEmTauROIRetriever"
            )
        )

        result.addPublicTool(
            CompFactory.JiveXML.xAODJetROIRetriever(
                name="xAODJetROIRetriever"
            )
        )

        result.addPublicTool(
            CompFactory.JiveXML.xAODMuonROIRetriever(
                name="xAODMuonROIRetriever"
            )
        )

        result.addPublicTool(
            CompFactory.JiveXML.xAODTriggerTowerRetriever(
                name="xAODTriggerTowerRetriever"
            )
        )
    return result


def AlgoJiveXMLCfg(flags, name="AlgoJiveXML", **kwargs):
    # This is based on a few old-style configuation files:
    # JiveXML_RecEx_config.py
    # JiveXML_jobOptionBase.py
    result = ComponentAccumulator()

    kwargs.setdefault("AtlasRelease", getATLASVersion())
    kwargs.setdefault("WriteToFile", True)
    # Enable this to recreate the geometry XML files for Atlantis
    kwargs.setdefault("WriteGeometry", False)
    if flags.Common.isOnline:
        kwargs.setdefault("OnlineMode", True)
    else:
        kwargs.setdefault("OnlineMode", False)

    # This next bit sets the data types, then we set the associated public tools
    readAOD = False  # FIXME - set this properly
    readESD = False  # FIXME - set this properly
    haveRDO = (
        flags.Input.Format is Format.BS or "StreamRDO" in flags.Input.ProcessingTags
    )
    kwargs.setdefault("DataTypes", getDataTypes(flags, haveRDO, readAOD))

    if not readAOD:
        result.merge(TruthTrackRetrieverCfg(flags))

    if haveRDO or readESD:
        if flags.Detector.EnableID:
            result.merge(InDetRetrieversCfg(flags))

        if flags.Detector.EnableCalo:
            result.merge(CaloRetrieversCfg(flags))

        if flags.Detector.EnableMuon:
            result.merge(MuonRetrieversCfg(flags))

    result.merge(xAODRetrieversCfg(flags))

    result.merge(TriggerRetrieversCfg(flags))

    the_alg = CompFactory.JiveXML.AlgoJiveXML(name="AlgoJiveXML", **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    
    
    return result
