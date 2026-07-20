# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthSegmentWriterCfg(flags, name="TruthSegmentWriter", 
                          outFile="MuonSpacePoints.root", **kwargs):
    result = ComponentAccumulator()
    if not flags.Muon.setupTruthAlgorithms:
        return result
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    result.merge(setupHistSvcCfg(flags, outFile=outFile,
                                    outStream="ActsMuonTruthDump"))

    the_alg = CompFactory.MuonValR4.TruthSegmentWriter(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result


def SpacePointWriterCfg(flags, name="SpacePointWriter", outFile="MuonSpacePoints.root", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    result.merge(setupHistSvcCfg(flags, outFile=outFile,
                                        outStream="ActsMuonSpacePointDump"))
    the_alg = CompFactory.MuonValR4.SpacePointWriter(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)

    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import executeTest
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.RDO_R3)

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)

    from ActsConfig.ActsGeometryConfig import ActsWriteTrackingGeometryCfg
    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(xAODUncalibMeasPrepCfg(flags))
    cfg.merge(MuonSpacePointFormationCfg(flags))
    
    cfg.merge(ActsWriteTrackingGeometryCfg(flags))
    cfg.merge(SpacePointWriterCfg(flags))
    cfg.merge(TruthSegmentWriterCfg(flags))
    executeTest(cfg)

