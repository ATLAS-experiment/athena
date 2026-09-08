#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from TriggerMenuMT.HLT.Muon.TrigMuonKeys import muonNames
muNames = muonNames().getNames('RoI')
muNamesFS = muonNames().getNames('FS')

def MuonMatchingToolConfig(flags, **kwargs):
    
    acc = ComponentAccumulator()

    from TrigT1MuctpiPhase1.TrigT1MuctpiPhase1Config import TrigThresholdDecisionToolCfg
    kwargs.setdefault("TrigThresholdDecisionTool", 
                      acc.popToolsAndMerge(TrigThresholdDecisionToolCfg(flags, 
                                                                        name="TrigThresholdDecisionTool", 
                                                                        AODinput = flags.Trigger.triggerConfig == 'INFILE')))

    kwargs.setdefault("isPhaseII", False)

    # Set the containers that always exist but change name depending on whether we are running Phase II or not
    kwargs.setdefault("L2StandAloneMuonContainerName", muNames.L2SAMuons)
    kwargs.setdefault("L2CombinedMuonContainerName", muNames.L2CBMuons)
    kwargs.setdefault("EFSAMuonContainerName", muNames.EFSAMuonsPhII if kwargs["isPhaseII"] else muNames.EFSAMuons)
    kwargs.setdefault("EFCBMuonContainerName", muNames.EFCBMuons)
    kwargs.setdefault("EFSAFSMuonContainerName", muNamesFS.EFSAMuonsPhII if kwargs["isPhaseII"] else muNamesFS.EFSAMuons)
    kwargs.setdefault("EFCBFSMuonContainerName", muNamesFS.EFCBMuons)
    
    # Set the containers that only exist when runing Phase II software & Run4 menu
    from TrigConfigSvc.TriggerConfigAccess import getHLTMenuAccess
    isPhIIAndR4Menu = kwargs["isPhaseII"] and "run4" in getHLTMenuAccess(flags).name()

    kwargs.setdefault("FastRecoSAContainerName", muNames.L2SAMuonsPhII if isPhIIAndR4Menu  else "")
    kwargs.setdefault("EFSAMlbktMuonContainerName", muNames.EFSAMuonsPhIIMlbkt if isPhIIAndR4Menu else "")
    kwargs.setdefault("EFSANewFastMuonContainerName", muNames.EFSAMuonsPhIINewFast if isPhIIAndR4Menu else "")
    kwargs.setdefault("EFSAFSMlbktMuonContainerName", muNamesFS.EFSAMuonsPhIIMlbkt if isPhIIAndR4Menu else "")
    kwargs.setdefault("EFSAFSNewFastMuonContainerName", muNamesFS.EFSAMuonsPhIINewFast if isPhIIAndR4Menu else "")
   
    acc.setPrivateTools(CompFactory.MuonMatchingTool("MuonMatchingTool", **kwargs))

    return acc