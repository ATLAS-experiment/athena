#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# ===============================================================
#  __mistimedAlg(flags)__
# ===============================================================
def mistimedAlg(flags, myflags):

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

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
        "xAOD::JetElementAuxContainer/JetElementsAux.",
        # ====== CTP ============================================================
        "CTP_RDO/CTP_RDO"
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
    MistimedStreamMonitorCfg = MistimedStreamMonitorConfig(flags, myflags)
    acc.merge(MistimedStreamMonitorCfg)

    MistimedStreamMonitorCfg.OutputLevel = 1 # 1/2 INFO/DEBUG

    # Return our accumulator
    return acc

# ===============================================================
#  __main__
# ===============================================================
if __name__ == "__main__": # typically not needed in top level script

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = [] # so that when no files given we can detect that

    flags.Trigger.triggerConfig='DB'
    flags.Exec.MaxEvents = -1
    flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00'
    flags.Trigger.EDMVersion = 3
    flags.Trigger.L1.doCTP = True
    flags.Trigger.enableL1CaloPhase1 = True
    flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2023-01'

    import argparse
    parser = flags.getArgumentParser()
    parser.add_argument('--systemVersion', default='phaseI', help="legacy or phaseI (default: %(default)s)")
    parser.add_argument('--streamName', default='physics_Mistimed', help="stream name (default: %(default)s)")
    parser.add_argument('--dataTag', default='data25_13p6TeV', help="data tag (default: %(default)s)")

    parser.add_argument('--efexItems', nargs='+', default=['L1_eEM26M'], help="eFex items in BC0 (default: %(default)s)")
    parser.add_argument('--jfexItems', nargs='+', default=['L1_jJ160', 'L1_jJ500'], help="jFex items in BC0 (default: %(default)s)")
    parser.add_argument('--gfexItems', nargs='+', default=['L1_gJ400p0ETA25', 'L1_gLJ140p0ETA25'], help="gFex items in BC0 (default: %(default)s)")

    requiredNamed = parser.add_argument_group('Required named arguments')
    requiredNamed.add_argument('--runNumber', default=None, help="8 digit run number (default: %(default)s)", required=True)
    args,unknown_args = flags.fillFromArgs(parser=parser,return_unknown=True)
    
    import glob
    runNumber = args.runNumber
    systemVersion = args.systemVersion
    streamName = args.streamName
    dataTag = args.dataTag

    # if no explicit file names is given, build it from arguments
    if len(flags.Input.Files)==0:
        flags.Input.Files = glob.glob("/eos/atlas/atlastier0/rucio/"+dataTag+"/"+streamName+"/"+runNumber+"/"+dataTag+"."+runNumber+"."+streamName+".merge.RAW/"+dataTag+"."+runNumber+"."+streamName+".merge.RAW._lb*._SFO-ALL._0001.1")

    # check that we have proper file name
    if len(flags.Input.Files)==0:
        exit('Error: input files do not exist')
    
    flags.Trigger.DecisionMakerValidation.Execute=False
    flags.Trigger.DecisionMakerValidation.ErrorMode=False

    flags.Output.HISTFileName = "MistimedPhI_"+runNumber+"_"+systemVersion+".root"
    flags.lock()

    # create basic infrastructure
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    sysAcc = MainServicesCfg(flags)

    # set heavy ions tag depending on data tag
    ionsTag=True if dataTag.endswith("_hi") else False

    # bundle all additional algorithm flags
    myflags = {}
    # MW: continue to maintain two independent flags (though atm only one active at a time)
    if (systemVersion=="legacy"):
        myflags["legacy"] = True
        myflags["phaseI"] = False
    else:
        myflags["legacy"] = False
        myflags["phaseI"] = True

    myflags["ions"] = ionsTag

    # trigger selection
    myflags["efex"] = args.efexItems
    myflags["jfex"] = args.jfexItems
    myflags["gfex"] = args.gfexItems

    # add the algorithm to the configuration
    sysAcc.merge( mistimedAlg(flags, myflags) )

    # debug printout
    sysAcc.printConfig(withDetails=True, summariseProps=True)

    # print all settings used by this script
    import os.path
    for k,v in sorted(vars(args).items()):
        print("{0}: {1}: {2}".format(os.path.basename(__file__),k,v))
    
    # run the job
    status = sysAcc.run()

    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())
