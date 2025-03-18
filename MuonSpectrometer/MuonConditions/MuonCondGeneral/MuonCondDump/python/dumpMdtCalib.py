# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def MdtCalibJsonDumpAlgCfg(flags, name= "MdtCalibJsonDumpAlg", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonCalibrationConfig import MdtCalibDbAlgCfg
    result.merge(MdtCalibDbAlgCfg(flags))
    kwargs.setdefault("JsonPerIOV",False and not flags.Input.isMC)
    the_alg = CompFactory.Muon.MdtCalibJsonDumpAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser
    parser = SetupArgParser()
    parser.add_argument("--outRtJSON" , default="RtConstants.json")
    parser.add_argument("--outT0JSON" , default="T0Constants.json")
    parser.set_defaults(outRootFile="MdtCalib.root")
    
    #parser.set_defaults(inputFile = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/ESD/data23_cos.00448208.express_express.recon.ESD.x721/73events.data23_cos.00448208.express_express.recon.ESD.x721._lb0003._SFO-ALL._0001.1"])
    #parser.set_defaults(condTag="CONDBR2-BLKPA-2023-02")
    args = parser.parse_args()
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, executeTest, setupHistSvcCfg
    flags = initConfigFlags()
    flags.Muon.Calib.fitAnalyticRt = True

    flags, cfg = setupGeoR4TestCfg(args, flags)
    cfg.merge(MdtCalibJsonDumpAlgCfg(flags,RtJSON = args.outRtJSON, TubeT0JSON = args.outT0JSON))
    cfg.merge(setupHistSvcCfg(flags, outFile = "{rootFile}.root".format(rootFile = args.outRtJSON[: args.outRtJSON.rfind(".")]), outStream="MDTRTCALIBDUMP"))
    cfg.merge(setupHistSvcCfg(flags, outFile = "{rootFile}.root".format(rootFile = args.outT0JSON[: args.outT0JSON.rfind(".")]), outStream="MDTT0CALIBDUMP"))

    from MuonCondAlgR4.ConditionsConfig import MdtAnalyticRtCalibAlgCfg
    cfg.merge(MdtAnalyticRtCalibAlgCfg(flags))
    cfg.getCondAlgo("MdtCalibDbAlg").checkTubes = False
    executeTest(cfg)
