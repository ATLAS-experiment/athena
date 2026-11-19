# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
import TrigLongLivedParticlesHypo.TrigLongLivedParticlesHypoMonitoring as HypoMon

def TrigLongLivedParticlesHypoToolFromDict(flags, chainDict):
    """
    Generates the Hypo Tool from the chain dictionary
    """
    name = chainDict['chainName']

    ## Initialize default-configured HypoTool as 'tool'
    from TrigLongLivedParticlesHypo.TrigLongLivedParticlesHypoConfig import MuonClusterHypoToolConfig
    return MuonClusterHypoToolConfig(flags, name)


def MuonClusterHypoAlgConfig(flags, name="MuonClusterHypoAlgConfig"):
    """Monitoring Tool Configuration for HypoAlg"""
    return CompFactory.MuonClusterHypoAlg(name,
        MonTool = HypoMon.trigMuonClusterHypoAlgMonitoring(flags))


def MuonClusterHypoToolConfig(flags, name="MuonClusterHypoToolConfig"):
    """Monitoring Tool Configuration for HypoTool"""
    return CompFactory.MuonClusterHypoTool(name,
        MonTool = HypoMon.trigMuonClusterHypoToolMonitoring(flags),
        acceptAll = False)

def MlMuonDVtxHypoAlgConfig(flags, name="MlMuonDVtxHypoAlgConfig"):
    """Monitoring Tool Configuration for HypoAlg"""
    return CompFactory.MlMuonDVtxHypoAlg(name)

def MlMuonDVtxHypoToolConfig(flags, name="MlMuonDVtxHypoToolConfig"):
    """Monitoring Tool Configuration for HypoTool"""
    the_tool =  CompFactory.MlMuonDVtxHypoTool(name)
    the_tool.score_cut = 0.94
    return the_tool

def MlMuonDVtxHypoToolFromDict(flags, chainDict):
    name = chainDict['chainName']
    return MlMuonDVtxHypoToolConfig(flags, name)

def MlMuonDVtxTrkVetoHypoAlgConfig(flags, name="MlMuonDVtxTrkVetoHypoAlg", **kwargs):
    """Prompt-track veto after the ML score cut"""
    return CompFactory.MlMuonDVtxTrkVetoHypoAlg(name, **kwargs)

def MlMuonDVtxTrkVetoHypoToolConfig(flags, name="MlMuonDVtxTrkVetoHypoTool"):
    the_tool = CompFactory.MlMuonDVtxTrkVetoHypoTool(name)
    the_tool.MinTrackPt = 5000.  # MeV
    the_tool.MaxTrackDR = 0.4
    the_tool.MaxDz0SinTheta = 2.  # mm
    the_tool.MinClusterSegments = 2
    the_tool.OnlyLeadingCluster = True
    the_tool.MaxClusters = 5  # used with OnlyLeadingCluster = False
    return the_tool

def MlMuonDVtxTrkVetoHypoToolFromDict(flags, chainDict):
    name = chainDict['chainName']
    return MlMuonDVtxTrkVetoHypoToolConfig(flags, name)
