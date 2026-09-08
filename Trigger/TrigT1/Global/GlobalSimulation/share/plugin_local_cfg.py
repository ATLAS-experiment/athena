# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# run this with: l1calo-ath-mon --postInclude GlobalSimulation/plugin_local_cfg.py --evtMax 10 --filesInput RAW_RUN3_DATA24
#cfg and flags are already defined

def setup(flags):
    flags.DQ.doMonitoring=False # turns off any L1Mon setup for l1calo-ath-mon
    if flags.Concurrency.NumThreads==0: flags.Concurrency.NumThreads = 1 # ensure running in MT mode


# Add algorithm to prepare LAr cells for Global                                                                                                                                                                                                                           
from  GlobalSimulation.LArCellPreparationAlgConfig import LArCellPreparationAlgCfg
gblLArCellContainerKey = "GlobalLArCells"

sequenceName = "GlobalSim"
cfg.addSequence(CompFactory.AthSequencer(sequenceName,StopOverride=True),parentName="AthAlgSeq")


cfg.merge(LArCellPreparationAlgCfg(flags,
                                   numberOfEnergyBits = 6,
                                   valueLeastSignificantBit = 40,
                                   valueGainFactor = 4,
                                   GlobalLArCellsKey = gblLArCellContainerKey), sequenceName=sequenceName)



# Add algorithm to simulate MUX input/output for LAr cells
from  GlobalSimulation.LArCellMuxAlgConfig import LArCellMuxAlgCfg
cfg.merge(LArCellMuxAlgCfg(flags,
                           GlobalLArCellsKey = gblLArCellContainerKey,
                           WriteMuxInputBitstreamToFile = True,
                           WriteMuxOutputBitstreamToFile = True), sequenceName=sequenceName)

# add in the Algorithm to build a  LArStrip Neighborhood container
from  GlobalSimulation.Egamma1_OnlineMapNbhoodConfig import Egamma1_OnlineMapNbhoodCfg
cfg.merge(Egamma1_OnlineMapNbhoodCfg(flags,
                                     dump=False,
                                     dumpTerse=False), sequenceName=sequenceName)

cfg.getEventAlgo("Egamma1_OnlineMapNbhood").roiAlgTool.etMin = 5000.
cfg.getEventAlgo("Egamma1_OnlineMapNbhood").roiAlgTool.etaMin = 0.0
cfg.getEventAlgo("Egamma1_OnlineMapNbhood").roiAlgTool.etaMax = 5.0

from GlobalSimulation.GlobalSimAlgCfg_local import GlobalSimulationAlgCfg
cfg.merge(GlobalSimulationAlgCfg(flags, dump=True), sequenceName=sequenceName)

# optional: scheduler the graph creator svc:
cfg.addService( CompFactory.GlobalSim.GraphSvc(SequenceNameFilter=sequenceName,OutputLevel=3), create=True )
