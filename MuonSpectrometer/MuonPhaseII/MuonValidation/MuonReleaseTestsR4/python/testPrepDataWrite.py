# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def setupTestOutputCfg(flags,**kwargs):

    kwargs.setdefault("AcceptAlgs",[])
  
    result = ComponentAccumulator()
    ### Setup an xAOD Stream to test the size of the Mdt container
    # =============================
    # Define contents of the format
    # =============================
    container_items = ["xAOD::MdtDriftCircleContainer#*", "xAOD::MdtDriftCircleAuxContainer#*",
                       "xAOD::sTgcStripContainer#*", "xAOD::sTgcStripAuxContainer#*",
                       "xAOD::MMClusterContainer#*", "xAOD::MMClusterAuxContainer#*",
                       "xAOD::TgcStripContainer#*", "xAOD::TgcStripAuxContainer#*",
                       "xAOD::RpcStripContainer#*", "xAOD::RpcStripAuxContainer#*",
                       "xAOD::MdtTwinDriftCircleContainer#*", "xAOD::MdtDriftCircleAuxContainer#*",
                       "xAOD::RpcStrip2DContainer#*", "xAOD::RpcStrip2DAuxContainer#*",
                        "xAOD::sTgcPadContainer#*", "xAOD::sTgcPadAuxContainer#*",
                        "xAOD::sTgcStripContainer#*", "xAOD::sTgcStripAuxContainer#*",
                        "xAOD::sTgcWireContainer#*", "xAOD::sTgcWireAuxContainer#*", 
                         
                           ]


    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    result.merge(SetupMetaDataForStreamCfg(flags, kwargs["streamName"]))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    kwargs.setdefault("ItemList", container_items)
    result.merge(OutputStreamCfg(flags, **kwargs))
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.add_argument("--saveTestNtuple", help="Schedule the SimHits tester n-tuple", action='store_true',
                         default=False)
    parser.set_defaults(nEvents = 150)
    parser.set_defaults(outRootFile="MuonPrepDataTest.root")
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R4)
    parser.set_defaults(defaultGeoFile="RUN4")
    parser.set_defaults(eventPrintoutLevel = 50)
   
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()


    streamName = "MuonPrepDataTest"
    flags.addFlag(f"Output.{streamName}FileName", args.outRootFile)
    flags.addFlag(f"Output.doWrite{streamName}", True)

    flags, cfg = setupGeoR4TestCfg(args, flags)
    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))
 
    cfg.merge(setupTestOutputCfg(flags, streamName=streamName))  
 
    executeTest(cfg)
  
