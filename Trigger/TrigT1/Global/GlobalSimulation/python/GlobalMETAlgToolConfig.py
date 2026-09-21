# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GlobalMETAlgToolCfg(
        flags,
        name='GlobalMETAlgTool',
        gblCellTowersKey = "GlobalCellTowers",
        gblJet1JetsKey = "GlobalJet1Jets",
        gblMETKey = "GlobalMET",

        # Algorithm parameters. Defaults match TrigGepPerf's GepTotalMETAlgCfg, which
        # drives the same bitwise core from floating point input.
        #
        # The jet cone radius and the jet multiplicity are NOT settable here: both are
        # properties of the upstream WTACone/JET1 algorithm that produced the jets, so
        # MET takes what it is given rather than declaring its own.
        #
        # The E_T thresholds are left at 0 deliberately: the Et scale of the TOBs on this
        # path is not the 0.25 GeV LSB the digitization assumes (see the note in
        # GlobalMETAlgTool::initialize), so a non-zero threshold would cut at the wrong
        # energy. The tool warns if either is set.
        jetEtThresholdGeV=0.0,
        towerEtThresholdGeV=0.0,
        doJetTowerOverlapRemoval=False,
        towerScaleFactor=1.0,
        jetScaleFactor=1.0,
        maxTowersConsidered=4096,
        OutputLevel=None):
    """Configure GlobalMETAlgTool, the bitwise GEP total MET emulation.

    Takes TWO inputs -- the cell towers and the Jet1 (WTA cone) jets built from those
    same towers -- and emits one MET TOB per event in the Table 4.5 format.
    """

    cfg = ComponentAccumulator()
    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name)

    metAlgTool = CompFactory.GlobalSim.GlobalMETAlgTool(name)

    if OutputLevel is not None:
        metAlgTool.OutputLevel = OutputLevel

    metAlgTool.GlobalCellTowersKey = gblCellTowersKey
    metAlgTool.GlobalJet1JetsKey = gblJet1JetsKey
    metAlgTool.GlobalMETKey = gblMETKey

    metAlgTool.JetEtThresholdGeV = jetEtThresholdGeV
    metAlgTool.TowerEtThresholdGeV = towerEtThresholdGeV
    metAlgTool.DoJetTowerOverlapRemoval = doJetTowerOverlapRemoval
    metAlgTool.TowerScaleFactor = towerScaleFactor
    metAlgTool.JetScaleFactor = jetScaleFactor
    metAlgTool.MaxTowersConsidered = maxTowersConsidered

    alg.globalsim_algs = [metAlgTool]

    cfg.addEventAlgo(alg)

    return cfg
