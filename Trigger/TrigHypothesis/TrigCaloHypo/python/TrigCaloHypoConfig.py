# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from IOVDbSvc.IOVDbSvcConfig import addFolders


def TrigLArNoiseBurstRecoAlgCfg(flags, cells_name):

    cfg = ComponentAccumulator()
    monTool = GenericMonitoringTool(flags)

    monTool.defineHistogram('bitWise_flags', type='TH1I', path='EXPERT',
                            title="LArNoiseBurst Cut Counter;Cut;Count",
                            xlabels=["Input","BadFEBFlaggedPartitions", "BadFEB_WFlaggedPartitions", "SatTightFlaggedPartitions", "MNBLooseFlaggedPartions", "MNBTightFlaggedPartions", "MNBTight_PsVetoFlaggedPartions", "Output"])
    monTool.defineHistogram('TIME_larnoisetool', type='TH1F', path='EXPERT', title="Time;time(ps)",
                            xbins=100, xmin=-100.0, xmax=15000)

    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    cfg.merge( LArOnOffIdMappingCfg(flags) )

    cfg.addEventAlgo( CompFactory.TrigLArNoiseBurstRecoAlg(
         CellContainerKey = cells_name,
         Tool = CompFactory.LArNoisyROTool(SaturatedCellTightCut=20, MNBLooseCut=5, MNBTightCut=17,
         BadChanPerFEB=15, DoHVflag=False), # check ATR-27613 for details
         MonTool = monTool) )

    cfg.addCondAlgo( CompFactory.LArBadFebCondAlg(
         "LArKnownBadFebAlg",
         ReadKey="/LAR/BadChannels/KnownBADFEBs" if not flags.Input.isMC else "",
         WriteKey="LArKnownBadFEBs") )

    cfg.addCondAlgo( CompFactory.LArBadFebCondAlg(
         "LArKnownMNBFebAlg",
         ReadKey="/LAR/BadChannels/KnownMNBFEBs" if not flags.Input.isMC else "",
         WriteKey="LArKnownMNBFEBs") )

    if not flags.Input.isMC:
        cfg.merge( addFolders(flags, ["/LAR/BadChannels/KnownBADFEBs", "/LAR/BadChannels/KnownMNBFEBs"],
                              "LAR_ONL", className="AthenaAttributeList") )

    return cfg


def TrigLArNoiseBurstHypoToolGen(flags, chainDict):
    return CompFactory.TrigLArNoiseBurstHypoTool(chainDict['chainName'])


def TrigL2CaloLayersHypo_PreS_080Cfg(name="TrigL2CaloLayersHypo_PreS_080"):
    return CompFactory.TrigL2CaloLayersHypoTool(
         name = name,
         EnergyFractionCut=[0.80,1.0,1.0,1.0],
         AcceptAll = False)


def TrigL2CaloLayersHypoToolGen(flags, chainDict):
    return TrigL2CaloLayersHypo_PreS_080Cfg(chainDict['chainName'])
