# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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


def DRAW_TAULHKernelCfg(configFlags, name='DRAW_TAULHKernel', **kwargs):
    """Configure DRAW_TAULH kernel"""

    mlog = logging.getLogger(name)
    mlog.info('Start configuration')

    acc = ComponentAccumulator()
    acc.addSequence(seqAND('DRAW_TAULHSequence'))

    # The selections
    augmentationTools = []

    # trigger-based skimming
    # unprescaled - single electron and muon triggers
    triggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(
        name = "TAULH_TriggerSkimmingTool",
        TriggerListOR = ["HLT_e26_lhtight_ivarloose_L1eEM26M",
                         "HLT_e60_lhmedium_L1eEM26M",
                         "HLT_e140_lhloose_L1eEM26M",
                         "HLT_mu24_ivarmedium_L1MU14FCH",
                         "HLT_mu50_L1MU14FCH"] )

    acc.addPublicTool(triggerSkimmingTool)
    skimTool1 = triggerSkimmingTool 

    # The Ztautau Lep-Had skimming using AOD string skimming and delta-R tool
    # split into selections and requirements
    el_sel  = "(Electrons.pt > 27.0*GeV) && (abs(Electrons.eta) < 2.5)  && (Electrons.LHMedium)"
    elRequirement = '( count( ' + el_sel + '  ) == 1 )'
    mu_sel  = "(Muons.pt > 27.0*GeV) && (abs(Muons.eta) < 2.5) && (Muons.quality <= 1)" 
    muRequirement = '( count( ' + mu_sel + '  ) == 1 )'
    # Select JetRNNSigMedium, bit 30 of isTauFlags >= 107374182 (see TauDefs.h)
    tau_sel = "(TauJets.pt > 13*GeV) && (abs(TauJets.charge)==1.0) && (TauJets.isTauFlags >= 1073741824)" \
                     " && ((TauJets.nChargedTracks == 1) || (TauJets.nChargedTracks == 3))"
    tauRequirement = '( count( ' + tau_sel + '  ) >= 1 )'

    # additional delta-R requirement for electrons
    expression = "( ("+ elRequirement + " && (count (TAUEH_DeltaR > 0.5) >=1) ) || " + muRequirement + ") && " + tauRequirement

    # Add the deltaR tool for electrons
    tauLH_DeltaRTool = CompFactory.DerivationFramework.DeltaRTool(name            = "TAUEH_DeltaRTool",
                                                          ContainerName           = "Electrons",
                                                          ObjectRequirements      =  el_sel,
                                                          SecondContainerName     = "TauJets",
                                                          SecondObjectRequirements= tau_sel,
                                                          StoreGateEntryName      = "TAUEH_DeltaR")
    acc.addPublicTool(tauLH_DeltaRTool)
    augmentationTools.append(tauLH_DeltaRTool)

    stringSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
        name='TAULH_stringSkimmingTool',
        expression = expression)
    acc.addPublicTool(stringSkimmingTool)
    skimTool2 = stringSkimmingTool

    # require trigger and rec. selection requirements
    combTool = CompFactory.DerivationFramework.FilterCombinationAND(name="tauSkim", FilterList=[skimTool1,skimTool2])
    acc.addPublicTool(combTool,primary = True)

    # The main kernel algo
    DRAW_TAULHKernel = CompFactory.DerivationFramework.DerivationKernel(
        name='DRAW_TAULHKernel',
        doChronoStat=(configFlags.Concurrency.NumThreads <= 1),
        AugmentationTools=augmentationTools,
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
