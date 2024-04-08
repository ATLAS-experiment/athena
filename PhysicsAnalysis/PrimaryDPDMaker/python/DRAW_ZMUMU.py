# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import seqAND
from DerivationFrameworkTools.DerivationFrameworkToolsConfig import InvariantMassToolCfg
from PrimaryDPDMaker.DRAWCommonByteStream import DRAWCommonByteStreamCfg


def DRAW_ZmumuKernelCfg(flags, name="DRAW_ZMUMUKernel", **kwargs):

    result = ComponentAccumulator()
    result.addSequence(seqAND("DRAWZMUMU_Sequence"))

    massEntryName = "DRZmumuMass"
    # Object selection strings
    sel_muon1 = 'Muons.pt > 25*GeV && Muons.ptcone40/Muons.pt < 0.3'
    sel_muon2 = 'Muons.pt > 20*GeV && Muons.ptcone40/Muons.pt < 0.3'
    # Ensure that it's scheduled after the Isolation builder
    kwargs.setdefault(
        "ExtraInputs", [('xAOD::MuonContainer', 'StoreGateSvc+Muons.ptcone40')])

    mass_tool = result.getPrimaryAndMerge(InvariantMassToolCfg(flags,
                                                               name="DRZmumuMassTool",
                                                               ContainerName="Muons",
                                                               ObjectRequirements=sel_muon1,
                                                               SecondObjectRequirements=sel_muon2,
                                                               MassHypothesis=105.66,
                                                               SecondMassHypothesis=105.66,
                                                               StoreGateEntryName=massEntryName))

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import AsgSelectionToolWrapperCfg
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    muon_sel_tool = result.popToolsAndMerge(MuonSelectionToolCfg(flags, name="DRAW_ZMUMU_MuonsSelector",
                                                                 MaxEta=3, MuQuality=2  # Use Medium muons
                                                                 ))
    result.addPublicTool(muon_sel_tool)
    muonSkimmingTool = result.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                                            name="DRAW_ZMUMU_GoodMuon_SkimmingTool",
                                                                            ContainerName="Muons",
                                                                            StoreGateEntryName="isMedium_DRAWZmumu",
                                                                            AsgSelectionTool=muon_sel_tool))

    dimuonMassString = "( count (  {mass} > 70*GeV   &&  {mass} < 110*GeV ) >= 1 )".format(
        mass=massEntryName)

    muonSkimmingString = "( count (Muons.pt > 20*GeV && Muons.isMedium_DRAWZmumu == 1) >= 1)"
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg
    dimuonMassSkimmingTool = result.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags,
                                                       name="DRAW_ZMUMU_DiMuonMass_SkimmingTool",
                                                       expression="{mass} && {muon} ".format(mass=dimuonMassString,
                                                                                             muon=muonSkimmingString)))

    # needed for dynamic determination of lowest-unprescaled single-muon and dimuon triggers
    from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
    from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType
    periods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future
    TriggerAPI.setConfigFlags(flags)
    allUnprescaledTriggers = TriggerAPI.getLowestUnprescaledAnyPeriod(
        periods, TriggerType.mu) + ["HLT_noalg_L1MU14FCH", "L1_MU14FCH"]
    print("DRAW_ZMUMU: will skim on an OR of the following muon triggers (list provided at run-time by the TriggerAPI):")
    for trig in allUnprescaledTriggers:
        print(" **** {trig}".format(trig=trig))

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import TriggerSkimmingToolCfg
    triggerSkimmingTool = result.getPrimaryAndMerge(TriggerSkimmingToolCfg(flags,
                                                                           name="DRAWZMUMUTriggerSkimmingTool",
                                                                           TriggerListOR=allUnprescaledTriggers))

    # Event selection tool
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import FilterCombinationANDCfg
    DRAW_ZMUMU_SkimmingTool = result.getPrimaryAndMerge(FilterCombinationANDCfg(flags,
                                                        name="DRAW_ZMUMU_FinalFilter",
                                                        FilterList=[dimuonMassSkimmingTool,
                                                                    triggerSkimmingTool]))

    kwargs.setdefault("AugmentationTools", [muonSkimmingTool, mass_tool])
    kwargs.setdefault("SkimmingTools", [DRAW_ZMUMU_SkimmingTool])
    kwargs.setdefault("doChronoStat", flags.Concurrency.NumThreads <= 1)
    the_alg = CompFactory.DerivationFramework.DerivationKernel(name, **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result


def DRAW_ZmumuCfg(flags):
    acc = ComponentAccumulator()

    # Main algorithm (kernel)
    acc.merge(DRAW_ZmumuKernelCfg(flags), sequenceName="DRAWZMUMU_Sequence")
    acc.merge(DRAWCommonByteStreamCfg(flags,
                                      formatName='DRAW_ZMUMU',
                                      filename=flags.Output.DRAW_ZmumuFileName))

    return acc
