# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

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
    kwargs.setdefault("Ready4PhysicsAtStart", flags.OnlineEventDisplays.Ready4PhysicsAtStart)

    # This next bit sets the data types, then we set the associated public tools
    readAOD = False  # FIXME - set this properly
    readESD = False  # FIXME - set this properly
    haveRDO = (
        flags.Input.Format is Format.BS or "StreamRDO" in flags.Input.ProcessingTags
    )

    tools = []

    if not readAOD:
        from JiveXML.TruthTrackRetrieverConfig import TruthTrackRetrieverCfg
        tools += [result.getPrimaryAndMerge(TruthTrackRetrieverCfg(flags))]

    if haveRDO or readESD:
        if flags.Detector.EnableID and flags.Detector.GeometryID:
            from JiveXML.InDetRetrieversConfig import InDetRetrieversCfg
            cfg_ID, tools_ID = InDetRetrieversCfg(flags)
            result.merge(cfg_ID)
            tools += tools_ID

        if flags.Detector.EnableCalo and flags.Detector.GeometryCalo:
            from JiveXML.CaloRetrieversConfig import CaloRetrieversCfg
            cfg_Calo, tools_Calo = CaloRetrieversCfg(flags)
            result.merge(cfg_Calo)
            tools += tools_Calo

        if flags.Detector.EnableMuon and flags.Detector.GeometryMuon:
            from JiveXML.MuonRetrieversConfig import MuonRetrieversCfg
            cfg_Muon, tools_Muon = MuonRetrieversCfg(flags)
            result.merge(cfg_Muon)
            tools += tools_Muon
            
    from JiveXML.xAODRetrieversConfig import xAODRetrieversCfg
    cfg_xAOD, tools_xAOD = xAODRetrieversCfg(flags)
    result.merge(cfg_xAOD)
    tools += tools_xAOD
    
    from JiveXML.TriggerRetrieversConfig import TriggerRetrieversCfg
    cfg_Trigger, tools_Trigger = TriggerRetrieversCfg(flags)
    result.merge(cfg_Trigger)
    tools += tools_Trigger

    kwargs.setdefault("DataRetrieverTools", tools)
    the_alg = CompFactory.JiveXML.AlgoJiveXML(name="AlgoJiveXML", **kwargs)
    result.addEventAlgo(the_alg, primary=True)


    return result
