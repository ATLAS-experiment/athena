# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
    
def MuonSegmentReaderCfg(configFlags, **kwargs):
    #setup the tools
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    from MuonConfig.MuonRecToolsConfig import MuonEDMHelperSvcCfg

    result=ComponentAccumulator()
    result.merge(MuonIdHelperSvcCfg(configFlags))
    result.merge(MuonEDMHelperSvcCfg(configFlags))

    if "PullCalculator" not in kwargs:
        from TrkConfig.TrkResidualPullCalculatorConfig import (
            ResidualPullCalculatorCfg)
        kwargs.setdefault("PullCalculator", result.popToolsAndMerge(
            ResidualPullCalculatorCfg(configFlags)))

    alg = CompFactory.MuonCalib.MuonSegmentReader(**kwargs)

    result.addEventAlgo(alg)
    return result

if __name__ == "__main__":
    """Run a functional test if module is executed"""

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Logging import logging

    log = logging.getLogger('CalibStreamConfig')
    flags = initConfigFlags()

    #flags.Exec.SkipEvents = 1000
    #flags.Exec.MaxEvents = 1000
    flags.Concurrency.NumThreads = 1

    ##############################################
    # define the input files
    inputdir = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/MdtCalibration/'
    flags.Input.Files = [f'{inputdir}data23_calib.00456729.calibration_MuonAll.daq.RAW.0000_0004-0051.data']  # BMG2/4/6A12 
    #flags.Input.Files = [f'{d}data23_calib.00456729.calibration_MuonAll.daq.RAW.0000_0004-0200.data']  # Endcap 
    #flags.Input.Files = [f'{d}data23_calib.00456729.calibration_MuonAll.daq.RAW.0000_0004-0120.data']  # BIS7A16
    #flags.Input.Files = [f'{d}data23_calib.00456729.calibration_MuonAll.daq.RAW.0000_0004-0113.data']  # BIS7A02
   
    flags.Input.TypedCollections = 'calibration_MuonAll'
    flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00'
    flags.IOVDb.GlobalTag = 'CONDBR2-ES1PA-2023-03'      # last update May 2024
    flags.Input.isMC = False
    flags.Input.ProjectName=flags.Input.Files[0].split('/')[-1].split('.')[0]
    data = flags.Input.Files[0].split('/')[-1].split('.')
    if data[1][2:].isdigit():
        flags.Input.RunNumbers = [int(data[1][2:])]
    else:
        flags.Input.RunNumbers = [0]  #bogus run number in case parsing filename failed
    ntuplename = flags.Input.Files[0].split('/')[-1][0:-4] + "ntuple.root"
    
    flags.Detector.GeometryMDT   = True 
    flags.Detector.GeometryTGC   = True
    flags.Detector.GeometryRPC   = True
    flags.Detector.GeometryCSC   = False    # CSC was removed after LS2
    flags.Detector.GeometrysTGC  = False    # no NSW data in calibration stream
    flags.Detector.GeometryMM    = False    # no NSW data in calibration stream
    flags.Detector.GeometryCalo  = False    # no Calo data in calibration stream
    flags.Detector.GeometryID    = False    # no ID data in calibration stream
    flags.Detector.GeometryPixel = False    # no ID data in calibration stream
    flags.Detector.GeometrySCT   = False    # no ID data in calibration stream

    flags.Muon.makePRDs          = True
    
    from AthenaConfiguration.Enums import BeamType, Format
    flags.Beam.Type            = BeamType.Collisions
    flags.Input.Collections = []
    flags.Input.Format = Format.BS
    # setup the ESD output to verify the containers 
    flags.Output.doWriteESD      = False
    if flags.Output.ESDFileName == '':
        flags.Output.ESDFileName='newESD.pool.root'
    else:
        print('ESD = ', flags.Output.ESDFileName )
    
    # setup the outputLevel 
    #flags.Exec.OutputLevel = 2 # DEBUG
    flags.Exec.OutputLevel = 3 # INFO

    flags.PerfMon.doFullMonMT = True
    flags.PerfMon.OutputJSON = flags.Input.Files[0].split('/')[-1][0:-4] + "perfmonmt.json"

    flags.lock()

    acc = MainServicesCfg(flags)
    histSvc = CompFactory.THistSvc(Output=["%s DATAFILE='%s', OPT='RECREATE'" % ("CALIBNTUPLESTREAM", ntuplename)], AutoFlush= -10000000, AutoSave= -10000000)
    acc.addService(histSvc, primary=True)

    #setup the cpu and memory monitoring
    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    acc.merge(PerfMonMTSvcCfg(flags))

    # setup calibrationtool
    from MuonConfig.MuonCalibrationConfig import MdtCalibrationToolCfg

    tool_kwargs = {}
    tool_kwargs["UseTwin"] = True
    tool_kwargs["CalibrationTool"] = acc.popToolsAndMerge(MdtCalibrationToolCfg(flags, TimeWindowSetting = 3, DoPropagationCorrection = True, DoSlewingCorrection = True, DoMagneticFieldCorrection = True))

    # configure the muoncalibration stream reading
    from MuonCalibStreamCnv.MuonCalibStreamCnvConfig import MuonCalibStreamReadCfg, MdtCalibRawDataProviderCfg, RpcCalibRawDataProviderCfg, TgcCalibRawDataProviderCfg

    read = MuonCalibStreamReadCfg(flags)
    acc.merge(read)

    if flags.Detector.GeometryMDT:
        mdtCalibRawDataProvider = MdtCalibRawDataProviderCfg(flags)
        acc.merge(mdtCalibRawDataProvider)

    # set the RDO to PRD tools for TGC and RPC (MDT doesn't need this step)
    from MuonConfig.MuonRdoDecodeConfig import RpcRDODecodeCfg,TgcRDODecodeCfg
    if flags.Detector.GeometryRPC:
        rpcCalibRawDataProvider = RpcCalibRawDataProviderCfg(flags)
        acc.merge(rpcCalibRawDataProvider)
        acc.merge(RpcRDODecodeCfg(flags))
    if flags.Detector.GeometryTGC:
        tgcCalibRawDataProvider = TgcCalibRawDataProviderCfg(flags)
        acc.merge(tgcCalibRawDataProvider)
        acc.merge(TgcRDODecodeCfg(flags))


    from MuonConfig.MuonReconstructionConfig import MuonSegmentFindingCfg, MuonTrackBuildingCfg, StandaloneMuonOutputCfg
    reco = MuonSegmentFindingCfg(flags, setup_bytestream=False)
    reco.merge(MuonTrackBuildingCfg(flags))
    if flags.Output.doWriteESD or flags.Output.doWriteAOD:
        reco.merge(StandaloneMuonOutputCfg(flags))
    acc.merge(reco)

    # configure testAlg (switch off)
    #from MuonCalibStreamCnv.MuonCalibStreamCnvConfig import MuonCalibStreamTestAlgCfg
    # testAlg = MuonCalibStreamTestAlgCfg(flags)
    # acc.merge(testAlg)

    # configure segment creator
    #from AthenaCommon.Constants import DEBUG,INFO
    reader = MuonSegmentReaderCfg(flags, CalibrationTool = acc.popToolsAndMerge(MdtCalibrationToolCfg(flags, TimeWindowSetting = 3, DoPropagationCorrection = True, DoSlewingCorrection = True, DoMagneticFieldCorrection = True)))
    acc.merge(reader)

    acc.getCondAlgo("MdtCalibDbAlg").ReadKeyDCS=""
    
    acc.getService('Athena::DelayedConditionsCleanerSvc').RingSize=2
    acc.getService('Athena::DelayedConditionsCleanerSvc').CleanDelay=1
    acc.getService('Athena::DelayedConditionsCleanerSvc').LookAhead=1 

    acc.printConfig(withDetails=True, summariseProps=True)

    log.info("Config OK")

    # save the config
    with open('MuonCalibByteStreamConfig.pkl', 'wb') as pkl:
        acc.store(pkl)

    # Print out the storceGate in the end 
    # acc.getService('StoreGateSvc').Dump = True

    import sys
    sys.exit(acc.run(200).isFailure())

