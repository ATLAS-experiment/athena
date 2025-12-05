#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrigMuonEFTrackIsolationToolCfg(flags, name = "TrigMuonTrackIsoTool", **kwargs):

    acc = ComponentAccumulator()
    trackIsolation = CompFactory.TrigMuonEFTrackIsolationTool
    kwargs.setdefault('deltaZCut', flags.Trigger.Muon.IsolationDzCut)
    kwargs.setdefault('removeSelf',True)
    kwargs.setdefault('useAnnulus',False)
    kwargs.setdefault('useVarIso',True)
    isoTool = trackIsolation(name=name, **kwargs)
    acc.setPrivateTools(isoTool)
    return acc


def TrigMuonEFTrackIsolationAlgCfg(flags, name = "TrigMuonEFTrackIsolation", **kwargs):

    trackIsolationAlg = CompFactory.TrigMuonEFTrackIsolationAlg
    acc = TrigMuonEFTrackIsolationToolCfg(flags)

    kwargs.setdefault('OnlineIsolationTool', acc.popPrivateTools())
    kwargs.setdefault('IdTrackParticles', 'InDetTrigTrackingxAODCnv_Muon_IDTrig')
    kwargs.setdefault('requireCombinedMuon', True)
    kwargs.setdefault('useVarIso', True)
    kwargs.setdefault('MuonContName', 'MuonsIso')
    kwargs.setdefault('ptcone02Name', 'MuonsIso.ptcone02')
    kwargs.setdefault('ptcone02Name', 'MuonsIso.ptcone03')

    isoAlg = trackIsolationAlg(name, **kwargs)
    acc.addEventAlgo(isoAlg)

    return acc

def MuonFilterAlgCfg(flags, name="FilterZeroMuons", **kwargs):

    filterZeroMuons = CompFactory.MuonFilterAlg
    acc = ComponentAccumulator()
    kwargs.setdefault("MuonContainerLocation", "MuonsCB")

    acc.addEventAlgo(filterZeroMuons(name, **kwargs))

    return acc

def MergeEFMuonsAlgCfg(flags, name="MergeEFMuonsAlg", **kwargs):

    mergeMuons = CompFactory.MergeEFMuonsAlg
    acc = ComponentAccumulator()
    kwargs.setdefault("MuonCBContainerLocation", "MuonsCB_outsideIn")
    kwargs.setdefault("MuonInsideOutContainerLocation", "MuonsCB_insideOut")
    kwargs.setdefault("MuonOutputLocation", "MuonsCB")

    acc.addEventAlgo(mergeMuons(name, **kwargs))
    return acc

def MuonChainFilterAlgCfg(flags, name="MuonChainFilter", **kwargs):

    filterAlg = CompFactory.MuonChainFilterAlg
    acc = ComponentAccumulator()
    acc.addEventAlgo(filterAlg(name, **kwargs))
    return acc

def MergeMuonInDetTracksAlgCfg(flags, name="MergeMuonInDetTracksAlg", **kwargs):

    mergeTracks = CompFactory.MergeMuonInDetTracksAlg
    acc = ComponentAccumulator()
    acc.addEventAlgo(mergeTracks(name, **kwargs))
    return acc

def GetL2CBmuonInDetTracksAlgCfg(flags, name="GetL2CBmuonInDetTracksAlg", **kwargs):

    getTracks = CompFactory.GetL2CBmuonInDetTracksAlg
    acc = ComponentAccumulator()
    kwargs.setdefault("MuonL2CBContainerLocation", "MuonsL2CB")
    acc.addEventAlgo(getTracks(name, **kwargs))
    return acc

