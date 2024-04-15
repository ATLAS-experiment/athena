#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# ===============================================================
#  main()
# ===============================================================
def main():
    from optparse import OptionParser
    parser = OptionParser(usage = "usage: %prog arguments", version="%prog")
    parser.add_option("-r", dest="runNumber",type="string", help="Input raw data run number (default: %default)")
    parser.set_defaults(runNumber="00436354")
    (options,args) = parser.parse_args()
    
    import sys 

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.MaxEvents = -1
    flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2022-02'
    import glob
    runNumber = options.runNumber
    flags.Input.Files = glob.glob("/eos/atlas/atlastier0/rucio/data22_13p6TeV/physics_Mistimed/"+runNumber+"/data22_13p6TeV."+runNumber+".physics_Mistimed.merge.RAW/data22_13p6TeV."+runNumber+".physics_Mistimed.merge.RAW._lb*._SFO-ALL._0001.1")

    
    flags.Trigger.triggerConfig = 'DB'
    flags.Trigger.DecisionMakerValidation.Execute=False
    flags.Output.HISTFileName = "ExampleMonitorOutput_LVL1_"+runNumber+".root"
    
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
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
        
    ]
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg 
    from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg
    from TrigT1CaloByteStream.LVL1CaloRun2ByteStreamConfig import LVL1CaloRun2ReadBSCfg

    acc.merge(ByteStreamReadCfg(flags, type_names=type_names))
    acc.merge( L1TriggerByteStreamDecoderCfg(flags) )
    acc.merge( LVL1CaloRun2ReadBSCfg(flags) )
    
    
    from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfg
    acc.merge(TriggerRecoCfg(flags))
    
    

    from TrigT1CaloMonitoring.MistimedStreamMonitorAlgorithm import MistimedStreamMonitorConfig
    MistimedStreamMonitorCfg = MistimedStreamMonitorConfig(flags)
    acc.merge(MistimedStreamMonitorCfg)

    
    sys.exit(acc.run().isFailure())

# ===============================================================
#  __main__
# ===============================================================
if __name__ == '__main__':
    main()



