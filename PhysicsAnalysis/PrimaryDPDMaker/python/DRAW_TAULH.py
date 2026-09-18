# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#!/usr/bin/env python
# ====================================================================
# DRAW_TAULH.py
# This defines DRAW_TAULH, a skimmed DRAW format
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import seqAND
from AthenaCommon.Logging import logging
from PrimaryDPDMaker.DRAWCommonByteStream import DRAWCommonByteStreamCfg


def DRAW_TAULHKernelCfg(flags, name='DRAW_TAULHKernel', **kwargs):
    """Configure DRAW_TAULH kernel"""

    mlog = logging.getLogger(name)
    mlog.info('Start configuration')

    acc = ComponentAccumulator()
    acc.addSequence(seqAND('DRAW_TAULHSequence'))

    # trigger-based skimming
    # unprescaled - single electron and muon triggers
    filterList = []
    if flags.Trigger.EDMVersion >=0:
        triggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(
            name = "TAULH_TriggerSkimmingTool",
            TriggerListOR = ["HLT_e26_lhtight_ivarloose_L1eEM26M",
                             "HLT_e60_lhmedium_L1eEM26M",
                             "HLT_e140_lhloose_L1eEM26M",
                             "HLT_mu24_ivarmedium_L1MU14FCH",
                             "HLT_mu50_L1MU14FCH"] )
        acc.addPublicTool(triggerSkimmingTool)
        filterList += [triggerSkimmingTool]

    # tau selection tool
    from TauAnalysisTools.TauAnalysisToolsConfig import TauSelectionToolCfg
    TauSelectorMedium = acc.popToolsAndMerge(TauSelectionToolCfg(flags,
                                                                 name = 'TauSelectorMedium_TAULH',
                                                                 ConfigPath = 'TauAnalysisAlgorithms/tau_selection_gntau_medium_noeleid.conf'))
    acc.addPublicTool(TauSelectorMedium)

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import AsgSelectionToolWrapperCfg
    TauMediumWrapper = acc.addPublicTool(acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                         name               = "TauMediumWrapper_TAULH",
                                                                         ContainerName      = "TauJets",
                                                                         StoreGateEntryName = "TauMedium_TAULH",
                                                                         AsgSelectionTool   = TauSelectorMedium)))
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("TauMediumWrapper_TAULHAug", AugmentationTools = [TauMediumWrapper])

    # muon selection tool
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonLoosenedNonCalibratedSelectionToolCfg
    MuonSelectorMedium = acc.popToolsAndMerge(MuonLoosenedNonCalibratedSelectionToolCfg(flags, name="MuonSelectorMedium_TAULH",
                                                                   MaxEta=3, MuQuality=2))

    acc.addPublicTool(MuonSelectorMedium)

    MuonMediumWrapper = acc.addPublicTool(acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                          name               = "MuonMediumWrapper_TAULH",
                                                                          ContainerName      = "Muons",
                                                                          StoreGateEntryName = "MuonMedium_TAULH",
                                                                          AsgSelectionTool   = MuonSelectorMedium)))
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("MuonMediumWrapper_TAULHAug", AugmentationTools = [MuonMediumWrapper])

    # The Ztautau Lep-Had skimming using AOD string skimming and delta-R tool
    # split into selections and requirements
    el_sel  = "(Electrons.pt > 27.0*GeV) && (abs(Electrons.eta) < 2.5)  && (Electrons.LHMedium)"
    mu_sel  = "(Muons.pt > 25.0*GeV) && (abs(Muons.eta) < 3.0) && (Muons.MuonMedium_TAULH==1)"
    tau_sel = "(TauJets.pt > 20*GeV) && (abs(TauJets.charge)==1)" \
              " && ((TauJets.nChargedTracks == 1) || (TauJets.nChargedTracks == 3)) && (TauJets.TauMedium_TAULH==1)"


    #
    elRequirement = '( count( ' + el_sel + '  ) == 1 )'
    muRequirement = '( count( ' + mu_sel + '  ) == 1 )'
    tauRequirement = '( count( ' + tau_sel + '  ) >= 1 )'

    # additional delta-R requirement for electrons to avoid double counting object
    expression = "( ("+ elRequirement + " && (count (TAUEH_DeltaR > 0.1) >=1) ) || " + muRequirement + ") && " + tauRequirement
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    stringSkimmingTool = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
        flags, name='TAULH_stringSkimmingTool', expression = expression))
    filterList += [stringSkimmingTool]

    # require trigger AND rec. selection requirements
    combTool = CompFactory.DerivationFramework.FilterCombinationAND(
        name="tauSkim", FilterList=filterList)
    acc.addPublicTool(combTool,primary = True)

    # The kernel for delta-R tool
    for tool in [TauMediumWrapper, MuonMediumWrapper]:
        acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(tool.name+"Aug", AugmentationTools = [tool]), sequenceName='DRAW_TAULHSequence')
    DRAW_TAULHPreKernel = CompFactory.DerivationFramework.DerivationKernel(
        name='DRAW_TAULHPreKernel',
        SkimmingTools=[])
    acc.addEventAlgo(DRAW_TAULHPreKernel, sequenceName='DRAW_TAULHSequence')

    # Add the deltaR tool for electrons
    tauLH_DeltaRTool = acc.addPublicTool(CompFactory.DerivationFramework.DeltaRTool(name            = "TAUEH_DeltaRTool",
                                                                  ContainerName           = "Electrons",
                                                                  ObjectRequirements      =  el_sel,
                                                                  SecondContainerName     = "TauJets",
                                                                  SecondObjectRequirements= tau_sel,
                                                                  StoreGateEntryName      = "TAUEH_DeltaR"))
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("TAUEH_DeltaRAug", AugmentationTools = [tauLH_DeltaRTool]))

    # The main kernel algo
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("DRAW_TAULHKernelAug", AugmentationTools = [tauLH_DeltaRTool]), sequenceName='DRAW_TAULHSequence')
    DRAW_TAULHKernel = CompFactory.DerivationFramework.DerivationKernel(
        name='DRAW_TAULHKernel',
        doChronoStat=(flags.Concurrency.NumThreads <= 1),
        SkimmingTools=[combTool])
    acc.addEventAlgo(DRAW_TAULHKernel, sequenceName='DRAW_TAULHSequence')



    return acc


def DRAW_TAULHCfg(flags):
    """Main config fragment for DRAW_TAULH"""
    acc = ComponentAccumulator()
    
    # Main algorithm (kernel)
    acc.merge(DRAW_TAULHKernelCfg(flags, name='DRAW_TAULHKernel'))
    acc.merge(DRAWCommonByteStreamCfg(flags,
                                      formatName='DRAW_TAULH',
                                      filename=flags.Output.DRAW_TAULHFileName))

    return acc


