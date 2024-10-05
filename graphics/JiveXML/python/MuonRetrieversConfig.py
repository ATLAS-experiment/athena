# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MdtPrepDataRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.MdtPrepDataRetriever(name="MdtPrepDataRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result

def TgcPrepDataRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.TgcPrepDataRetriever(name="TgcPrepDataRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result

def sTgcPrepDataRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.sTgcPrepDataRetriever(name="sTgcPrepDataRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result

def RpcPrepDataRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.RpcPrepDataRetriever(name="RpcPrepDataRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result

def CSCClusterRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.CSCClusterRetriever(name="CSCClusterRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result

def CscPrepDataRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.CscPrepDataRetriever(name="CscPrepDataRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result

def MMPrepDataRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.MMPrepDataRetriever(name="MMPrepDataRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def MuonRetrieversCfg(flags, **kwargs):
    result = ComponentAccumulator()
    #kwargs.setdefault("StoreGateKey", "MDT_DriftCircles")
    tools = []

    if flags.Detector.EnableMuon and flags.Detector.GeometryMuon:
        # Taken from MuonJiveXML_DataTypes.py
        if flags.Detector.EnableMDT and flags.Detector.GeometryMDT:
            tools += [result.getPrimaryAndMerge(MdtPrepDataRetrieverCfg(flags))]

        if flags.Detector.EnableTGC and flags.Detector.GeometryTGC:
            tools += [result.getPrimaryAndMerge(TgcPrepDataRetrieverCfg(flags))]
            tools += [result.getPrimaryAndMerge(sTgcPrepDataRetrieverCfg(flags))]

        if flags.Detector.EnableRPC and flags.Detector.GeometryRPC:
            tools += [result.getPrimaryAndMerge(RpcPrepDataRetrieverCfg(flags))]

        if flags.Detector.EnableCSC and flags.Detector.GeometryCSC:
            tools += [result.getPrimaryAndMerge(CSCClusterRetrieverCfg(flags))]
            tools += [result.getPrimaryAndMerge(CscPrepDataRetrieverCfg(flags))]

        if flags.Detector.EnableMM and flags.Detector.GeometryMM:
            tools += [result.getPrimaryAndMerge(MMPrepDataRetrieverCfg(flags))]
        # TODO Not sure if below are still needed?
        # data_types += ["JiveXML::TrigMuonROIRetriever/TrigMuonROIRetriever"]
        # data_types += ["JiveXML::MuidTrackRetriever/MuidTrackRetriever]
        # data_types += ["JiveXML::TrigRpcDataRetriever/TrigRpcDataRetriever"]

    return result, tools
