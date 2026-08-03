# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod


def LArHVScaleCfg(configFlags, **props):
    result=ComponentAccumulator()

    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg, LArBadFebCfg

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(configFlags))

    from LArCabling.LArHVCablingConfig import LArHVCablingCfg
    result.merge(LArHVCablingCfg(configFlags))

    result.merge(LArBadFebCfg(configFlags))
    
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    LArHVCondAlg=CompFactory.LArHVCondAlg

    if configFlags.Input.isMC:
        hvcond = LArHVCondAlg(doHV=False, doAffectedHV=False)
    elif not configFlags.Common.isOnline:
        result.merge(addFolders(configFlags,["/LAR/DCS/HV/BARREl/I16"], "DCS_OFL", className="CondAttrListCollection"))
        result.merge(addFolders(configFlags,["/LAR/DCS/HV/BARREL/I8"],  "DCS_OFL", className="CondAttrListCollection"))

        result.merge(addFolders(configFlags,["/LAR/HVPathologiesOfl/Pathologies"], "LAR_OFL", className="AthenaAttributeList"))
        if configFlags.GeoModel.Run is not LHCPeriod.Run1:
            result.merge(addFolders(configFlags,["/LAR/HVPathologiesOfl/Rvalues"], "LAR_OFL", className="AthenaAttributeList"))

        result.merge(LArBadChannelCfg(configFlags))

        LArHVPathologyDbCondAlg=CompFactory.LArHVPathologyDbCondAlg
        hvpath = LArHVPathologyDbCondAlg(PathologyFolder="/LAR/HVPathologiesOfl/Pathologies",
                                         HVMappingKey="LArHVIdMap",
                                         HVPAthologyKey="LArHVPathology")
        result.addCondAlgo(hvpath)

        from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
        result.merge(LArElecCalibDBCfg(configFlags,["HVScaleCorr",]))

        if configFlags.GeoModel.Run is not LHCPeriod.Run1:
           hvcond = LArHVCondAlg(HVPathologies="LArHVPathology")
        else:
          hvcond = LArHVCondAlg(HVPathologies="LArHVPathology",doR=False)
        hvcond.keyOutputResidualCorr = props.pop("keyOutputResidualCorr", "LArHVScaleCorrRecomputed")
        hvcond.keyOutputFullCorr = props.pop("keyOutputFullCorr", "")
    hvcond.fixHV = props.pop("fixHV", [])
    hvcond.fixCurrent = props.pop("fixCurrent", [])
    if len(props) > 0: raise ValueError(f"tool properties not modified: {','.join(props.keys())}")
    result.addCondAlgo(hvcond)

    return result


def addHvScaleParserArgs(parser):
    # used by LArCalib_HVCorrConfig,py and CaloNoiseConfig.py
    parser.add_argument('-V','--voltage',type=str, default=[], action='append', nargs=1,
                        help="use -V \"<ID> <HV>\" to set the voltage for this line instead of reading it from DCS")
    parser.add_argument('-I','--current',type=str, default=[], action='append', nargs=1,
                        help="use -I \"<ID> <current>\" to set the current for this line instead of reading it from DCS")


def buildHvScaleProps(args):
    from itertools import chain
    from re import match
    v = list(chain.from_iterable(args.voltage))
    i = list(chain.from_iterable(args.current))
    for x in v + i:
        if match(r"\d+\s+\d+\.?\d*", x) is None:
            raise ValueError("ERROR: invalid voltage/current specification, should be of the form \"<line ID> <HV>\"")
    return {"fixHV": v, "fixCurrent": i}


def setConsistentHVScaleOutputKeys(ca1, ca2):
    """
    When two CAs configured a LArHVCondAlg, make an attempt at harmonizing the respective 
    alg properties such that CA deduplication won't fail, as long as the resulting 
    behaviour is unchanged; e.g. enabling extra outputs. 
    Properties for which invariance isn't guaranteed are left unchanged.
    """
    alg1 = ca1.getCondAlgo("LArHVCondAlg")
    alg2 = ca2.getCondAlgo("LArHVCondAlg")
    if alg1 and alg2:
        for x, y in ((alg1, alg2), (alg2, alg1)):
            if x.keyOutputResidualCorr == "": x.keyOutputResidualCorr = y.keyOutputResidualCorr
            if x.keyOutputFullCorr == "": x.keyOutputFullCorr = y.keyOutputFullCorr


if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags=initConfigFlags()

    nThreads=1
    flags.Concurrency.NumThreads = nThreads
    if nThreads>0:
        flags.Scheduler.ShowDataDeps = True
        flags.Scheduler.ShowDataFlow = True
        flags.Scheduler.ShowControlFlow = True
        flags.Concurrency.NumConcurrentEvents = nThreads

    flags.Input.Files = ["myESD-data.pool.root"]
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg=MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    cfg.merge( LArHVScaleCfg(flags) )

    cfg.run(10)
