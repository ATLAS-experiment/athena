#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# ===============================================================
#  __mistimedAlg(flags)__
# ===============================================================
def mistimedAlg(flags):

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.Enums import Format

    acc = ComponentAccumulator()

    type_names = [
        # ===== CPM ================================================================
        "xAOD::CPMTowerContainer/CPMTowers",
        "xAOD::CPMTowerAuxContainer/CPMTowersAux.",
        # ===== PPM ============================================================
        "xAOD::TriggerTowerContainer/xAODTriggerTowers",
        "xAOD::TriggerTowerAuxContainer/xAODTriggerTowersAux.",
        # ===== JETELEMENT =========================================================
        "xAOD::JetElementContainer/JetElements",
        "xAOD::JetElementAuxContainer/JetElementsAux."
    ]

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags, type_names=type_names))

    from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg
    acc.merge(L1TriggerByteStreamDecoderCfg(flags))

    from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfg
    acc.merge(TriggerRecoCfg(flags))

    #Decoder eFex TOBs
    from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import eFexByteStreamToolCfg
    acc.popToolsAndMerge(eFexByteStreamToolCfg(flags, 'eFexBSDecoder', xTOBs=True, multiSlice=True))

    #Decoder gFex TOBs
    from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import gFexByteStreamToolCfg
    acc.popToolsAndMerge(gFexByteStreamToolCfg(flags, 'gFexBSDecoder'))

    #Decoder jFex TOBs
    from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import jFexRoiByteStreamToolCfg
    acc.popToolsAndMerge(jFexRoiByteStreamToolCfg(flags, 'jFexBSDecoder'))

    #Decodes LATOME into SCell container
    from L1CaloFEXSim.L1CaloFEXSimCfg import ReadSCellFromByteStreamCfg
    acc.merge(ReadSCellFromByteStreamCfg(flags))

    #Decorator jFex towers
    from L1CaloFEXAlgos.L1CaloFEXAlgosConfig import L1CalojFEXDecoratorCfg
    acc.merge(L1CalojFEXDecoratorCfg(flags,ExtraInfo = False))

    #jFex emulated towers
    from L1CaloFEXAlgos.FexEmulatedTowersConfig import jFexEmulatedTowersCfg
    acc.merge(jFexEmulatedTowersCfg(flags,"jFexEmulatedTowerMaker", "L1_jFexEmulatedTowers"))    

    #mistimed algorithm 
    from TrigT1CaloMonitoring.MistimedStreamMonitorAlgorithm import MistimedStreamMonitorConfig
    MistimedStreamMonitorCfg = MistimedStreamMonitorConfig(flags)
    acc.merge(MistimedStreamMonitorCfg)

    MistimedStreamMonitorCfg.OutputLevel = 1 # 1/2 INFO/DEBUG

    # Return our accumulator
    return acc

# ===============================================================
#  __main__
# ===============================================================
if __name__ == "__main__": # typically not needed in top level script
    from optparse import OptionParser
    parser = OptionParser(usage = "usage: %prog arguments", version="%prog")
    parser.add_option("-r", dest="runNumber",type="string", help="Input raw data run number (default: %default)")
    parser.set_defaults(runNumber="00455857")
    (options,args) = parser.parse_args()

    import sys

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Trigger.triggerConfig='DB'
    flags.Exec.MaxEvents = -1
    flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00'
    flags.Trigger.EDMVersion = 3
    flags.Trigger.enableL1CaloPhase1 = True
    flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2023-01'
    
    import glob
    runNumber = options.runNumber
    flags.Input.Files = glob.glob("/eos/atlas/atlastier0/rucio/data23_13p6TeV/physics_Mistimed/"+runNumber+"/data23_13p6TeV."+runNumber+".physics_Mistimed.merge.RAW/data23_13p6TeV."+runNumber+".physics_Mistimed.merge.RAW._lb*._SFO-ALL._0001.1")
    
    flags.Trigger.DecisionMakerValidation.Execute=False
    flags.Trigger.DecisionMakerValidation.ErrorMode=False

    flags.Output.HISTFileName = "MistimedPhI_"+runNumber+".root"
    flags.lock()

    # create basic infrastructure
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    sysAcc = MainServicesCfg(flags)

    # debug printout
    sysAcc.printConfig(withDetails=True, summariseProps=True)

    # add the algorithm to the configuration
    sysAcc.merge(mistimedAlg(flags))

    # run the job
    status = sysAcc.run()

    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())
