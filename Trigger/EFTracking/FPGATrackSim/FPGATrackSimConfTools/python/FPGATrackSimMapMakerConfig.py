# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def getFirstStagePlanes(flags):
    """ Default logic for selecting logical layer / plane configuration for first stage.
        This used to be hardcoded in the map maker header file. Now it's hardcoded here."""
    planes = getSecondStagePlanes(flags)

    # The first stage planes should always be either the first 5, 9, or 8 layers.
    # For additional algorithms more permutations here could be added.
    # This could also fire based off of the "pipeline" setting if we want.
    if flags.doInsideOut:
        return planes[:5]
    elif flags.Trigger.FPGATrackSim.spacePoints:
        return planes[:9]
    else:
        return planes[:8]


def getSecondStagePlanes(flags):
    """ Default logic for selecting logical layer / plane configuration for first stage.
        This used to be hardcoded in the map maker header file. Now it's hardcoded here."""
    if (not flags.Trigger.FPGATrackSim.oldRegionDefs) or flags.doInsideOut:
        # Use a very generic assignment that just maps all modules to a layer to ensure they are turned on.
        # NOTE: this will *not* work in the forward region. There we need to be more subtle about what pixel layers
        # are "first stage" vs "second stage". Everywhere else though it's sufficient to assume strip == second stage, pixel == first stage.
        # (for the inside out algorithm).
        # Numbers here are taken from boundaries in FPGATrackSimModuleRelabel.h
        return [
            ["pb0"] + [f"pe{x}+" for x in range(0, 21)],
            ["pb1"] + [f"pe{x}+" for x in range(21, 44)],
            ["pb2"] + [f"pe{x}+" for x in range(44, 61)],
            ["pb3"] + [f"pe{x}+" for x in range(61, 77)],
            ["pb4"] + [f"pe{x}+" for x in range(77, 95)],
            ["sb0", "se4+", "se0+"],
            ["sb1", "se5+", "se1+"],
            ["sb2", "se6+", "se2+"],
            ["sb3", "se7+", "se3+"],
            ["sb4", "se8+"],
            ["sb5", "se9+"],
            ["sb6", "se10+"],
            ["sb7", "se11+"]
        ]
    else:
        # Use layer assignments for the old regions for the Hough-like algorithms.
        if flags.Trigger.FPGATrackSim.spacePoints:
            # New as of Python 3.10! (We are using 3.11)
            match flags.Trigger.FPGATrackSim.region:
                case 0 | 1 | 5 | 6 | 7:
                    return [["pb4"],["sb0"],["sb1"],["sb2"],["sb3"],["sb4"],["sb5"],["sb6"],["sb7"],["pb0"],["pb1"],["pb2"],["pb3"]]
                case 3:
                    return [["pb4","pe83+","pe84+","pe85+","pe86+","pe87+","pe88+","pe89+"],["se4+"],["se5+"],["se6+"],["se7+"],["se8+"],["se9+"],["se10+"],["se11+"],["pb2"],["pb3","pe58+"],["se2+"],["se3+"]]
                case 2 | 4 | _:
                    return [["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"]]
        else:
            match flags.Trigger.FPGATrackSim.region:
                case 0 | 1 | 5 | 6 | 7:
                    return [["pb4"],["sb0"],["sb2"],["sb3"],["sb4"],["sb5"],["sb6"],["sb7"],["pb0"],["pb1"],["pb2"],["pb3"], ["sb1"]]
                case 3:
                    return [["pb4","pe83+","pe84+","pe85+","pe86+","pe87+","pe88+","pe89+"],["se5+"],["se6+"],["se7+"],["se8+"],["se9+"],["se10+"],["se11+"],["pb2"],["pb3","pe58+"],["se2+"],["se3+"], ["se4+"]]
                case 2 | 4 | _:
                    return [["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"],["-1"]]

def FPGATrackSimMapMakerCfg(flags):
    acc = ComponentAccumulator()
    from FPGATrackSimConfTools.FPGATrackSimDataPrepConfig import FPGATrackSimReadInputCfg, FPGATrackSimEventSelectionCfg
    alg = CompFactory.FPGATrackSimMapMakerAlg(
        GeometryVersion=flags.GeoModel.AtlasVersion,
        OutFileName=flags.OutFileName,
        KeyString=flags.KeyString,
        nSlices=flags.nSlices,
        region=flags.Trigger.FPGATrackSim.region,
        trim=flags.trim,
        globalTrim=flags.globalTrim,
        InputTool = acc.getPrimaryAndMerge(FPGATrackSimReadInputCfg(flags)),
        eventSelector = acc.getPrimaryAndMerge(FPGATrackSimEventSelectionCfg(flags)),
        planes = getFirstStagePlanes(flags),
        planes2 = getSecondStagePlanes(flags)
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

