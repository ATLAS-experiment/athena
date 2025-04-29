# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
def CTPSimulationCfg(flags):
    from AthenaCommon.Logging import logging
    log = logging.getLogger("CTPMCSimulationCfg")
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.LVL1CTP.CTPSimulation("CTPSimulation",
                                                        DoL1Topo       = flags.Trigger.L1.doTopo, 
                                                        DoL1TopoLegacy = False,
                                                        #Using same as Phase1L1Topo for now, but it should be changed in the future
                                                        DoL1CaloLegacy = flags.Trigger.enableL1CaloLegacy,
                                                        #TODO enable when input are also simulatedDetectors (and remove message)
                                                        jFexJetInput = "",
                                                        jFexLJetInput = "",
                                                        gFexJetInput =  "",
                                                        gFexMETNCInput = "",
                                                        gFexMETRhoInput = "",
                                                        gFexMETJwoJInput = "",
                                                        eFexClusterInput = "",
                                                        eFexTauInput = "",
                                                        TopoInput = "L1TopoToCTPLocation",
                                                        LegacyTopoInput = "L1TopoLegacyToCTPLocation",
                                                        DoZDC = flags.Trigger.doZDC,
                                                        DoTRT = flags.Trigger.doTRT,
                                                        ForceBunchGroupPattern = False if flags.Beam.Type is BeamType.Cosmics else True #to allow simulation of cosmics triggers in MC
                                                        ))
    log.info("Not all part of CTP simulation are enabled yet")
    #Still needed for HLTSeeding, see ATR-29954
    roib = CompFactory.ROIB.RoIBuilder("RoIBuilder",
                                        DoCalo = flags.Trigger.enableL1CaloLegacy,
                                        DoMuon = False)   # not needed for L1MuonPhase1
    acc.addEventAlgo(roib)
    return acc
