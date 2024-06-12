# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonRetrieversCfg(flags, **kwargs):
    result = ComponentAccumulator()
    #kwargs.setdefault("StoreGateKey", "MDT_DriftCircles")

    if flags.Detector.EnableMuon and flags.Detector.GeometryMuon:
        # Taken from MuonJiveXML_DataTypes.py
        if flags.Detector.EnableMDT and flags.Detector.GeometryMDT:
            result.addPublicTool(CompFactory.JiveXML.MdtPrepDataRetriever(name="MdtPrepDataRetriever"), **kwargs)
        if flags.Detector.EnableTGC and flags.Detector.GeometryTGC:
            result.addPublicTool(CompFactory.JiveXML.TgcPrepDataRetriever(name="TgcPrepDataRetriever"), **kwargs)
            result.addPublicTool(CompFactory.JiveXML.sTgcPrepDataRetriever(name="sTgcPrepDataRetriever"), **kwargs)
        if flags.Detector.EnableRPC and flags.Detector.GeometryRPC:
            result.addPublicTool(CompFactory.JiveXML.RpcPrepDataRetriever(name="RpcPrepDataRetriever"), **kwargs)
        if flags.Detector.EnableCSC and flags.Detector.GeometryCSC:
            result.addPublicTool(CompFactory.JiveXML.CSCClusterRetriever(name="CSCClusterRetriever"), **kwargs)
            result.addPublicTool(CompFactory.JiveXML.CscPrepDataRetriever(name="CscPrepDataRetriever"), **kwargs)
        if flags.Detector.EnableMM and flags.Detector.GeometryMM:
            result.addPublicTool(CompFactory.JiveXML.MMPrepDataRetriever(name="MMPrepDataRetriever"), **kwargs)
        # TODO Not sure if below are still needed?
        # data_types += ["JiveXML::TrigMuonROIRetriever/TrigMuonROIRetriever"]
        # data_types += ["JiveXML::MuidTrackRetriever/MuidTrackRetriever]
        # data_types += ["JiveXML::TrigRpcDataRetriever/TrigRpcDataRetriever"]

    return result
