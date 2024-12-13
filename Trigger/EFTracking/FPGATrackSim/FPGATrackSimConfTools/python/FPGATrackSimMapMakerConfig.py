# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def FPGATrackSimMapMakerCfg(flags):
    acc = ComponentAccumulator()
    from FPGATrackSimConfTools.FPGATrackSimDataPrepConfig import FPGATrackSimReadInputCfg
    alg = CompFactory.FPGATrackSimMapMakerAlg(
        GeometryVersion=flags.GeoModel.AtlasVersion,
        OutFileName=flags.OutFileName,
        KeyString=flags.KeyString,
        nSlices=flags.nSlices,
        region=flags.Trigger.FPGATrackSim.region,
        trim=flags.trim,
        globalTrim=flags.globalTrim,
        doSpacePoints=flags.Trigger.FPGATrackSim.spacePoints,
        doInsideOut=flags.doInsideOut,
        InputTool = acc.getPrimaryAndMerge(FPGATrackSimReadInputCfg(flags))        
        )

    acc.addEventAlgo(alg)
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.addFlag("OutFileName", "MMTest")
    flags.addFlag("KeyString", "strip,barrel,0")
    flags.addFlag("nSlices", 6)
    flags.addFlag("trim", 0.1)
    flags.addFlag("globalTrim", 0)
    flags.addFlag('doInsideOut', False)
    
    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)

    flags.fillFromArgs()
    if not flags.Trigger.FPGATrackSim.wrapperFileName and flags.Input.Files:
        flags.Trigger.FPGATrackSim.wrapperFileName = flags.Input.Files
        log.info("Taken wrapper input files from Input.Files(set via cmd line --filesInput option) property: %s", str(flags.Trigger.FPGATrackSim.wrapperFileName))
    flags.lock()

    acc=MainServicesCfg(flags)
    acc.store(open('FPGATrackSimMapMakerConfig.pkl','wb'))
    acc.merge(FPGATrackSimMapMakerCfg(flags))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    statusCode = acc.run()
    assert statusCode.isSuccess() is True, "Application execution did not succeed"

