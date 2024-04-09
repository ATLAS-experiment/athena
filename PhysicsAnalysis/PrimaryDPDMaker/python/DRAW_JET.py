# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
# ====================================================================
# DRAW_JET.py
# This defines DRAW_JET, a skimmed DRAW format
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import seqAND
from AthenaCommon.Logging import logging
from PrimaryDPDMaker.DRAWCommonByteStream import DRAWCommonByteStreamCfg


def DRAW_JETKernelCfg(configFlags, name='DRAW_JETKernel', **kwargs):
    """Configure DRAW_JET kernel"""

    mlog = logging.getLogger(name)
    mlog.info('Start configuration')

    acc = ComponentAccumulator()
    acc.addSequence(seqAND('DRAW_JETSequence'))

    # The selections
    augmentationTools = []

    # trigger-based skimming
    # unprescaled - single electron and muon triggers
    triggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(
        name = "JET_TriggerSkimmingTool",
        TriggerListOR = ["HLT_j85_L1J20",
                         "HLT_j85_a10t_lcw_nojcalib_L120",
                         "HLT_j60f_L1J20p31ETA49"] )

    acc.addPublicTool(triggerSkimmingTool)
    skimTool1 = triggerSkimmingTool 

    # selecting events with forward jets to enhance statistics
    forward_jet_selection = "(AntiKt4EMPFlowJets.pt > 75.0*GeV) && (abs(AntiKt4EMPFlowJets.eta) >= 3.2)"
    expression = "count(" + forward_jet_selection + ") >= 1"


    stringSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
        name='JET_stringSkimmingTool',
        expression = expression)
    acc.addPublicTool(stringSkimmingTool)
    skimTool2 = stringSkimmingTool

    # require trigger and rec. selection requirements
    combTool = CompFactory.DerivationFramework.FilterCombinationOR(name="jetSkim", FilterList=[skimTool1,skimTool2])
    acc.addPublicTool(combTool,primary = True)

    # The main kernel algo
    DRAW_JETKernel = CompFactory.DerivationFramework.DerivationKernel(
        name='DRAW_JETKernel',
        doChronoStat=(configFlags.Concurrency.NumThreads <= 1),
        AugmentationTools=augmentationTools,
        SkimmingTools=[combTool])

    acc.addEventAlgo(DRAW_JETKernel, sequenceName='DRAW_JETSequence')
    return acc


def DRAW_JETCfg(flags):
    """Main config fragment for DRAW_JET"""
    acc = ComponentAccumulator()
    
    # Main algorithm (kernel)
    acc.merge(DRAW_JETKernelCfg(flags, name='DRAW_JETKernel'))
    acc.merge(DRAWCommonByteStreamCfg(flags,
                                      formatName='DRAW_JET',
                                      filename=flags.Output.DRAW_JETFileName))

    return acc
