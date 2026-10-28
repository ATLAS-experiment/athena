# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep


def EventSelectorAthenaPoolCfg(flags):
    result = ComponentAccumulator()
    # Secondary input needs a double selector and a separate secondary selector.
    # Configure both and return before the standard single-input setup below.
    if flags.Input.SecondaryFiles:
        skipEventsSecondary = flags.Exec.SkipEvents
        if flags.Overlay.SkipSecondaryEvents >= 0:
            skipEventsSecondary = flags.Overlay.SkipSecondaryEvents

        evSel = CompFactory.DoubleEventSelectorAthenaPool(
            "EventSelector",
            InputCollections=flags.Input.Files,
            SkipEvents=skipEventsSecondary,
            ProcessMetadata=False)
        if flags.Common.isOverlay and flags.Overlay.DataOverlay:
            evSel.OverrideRunNumber = True
            evSel.RunNumber = flags.Input.RunNumbers[0]
            evSel.FirstLB = flags.Input.LumiBlockNumbers[0]
            evSel.InitialTimeStamp = flags.Input.TimeStamps[0]

        secondarySel = CompFactory.EventSelectorAthenaPool(
            "SecondaryEventSelector",
            IsSecondary=True,
            InputCollections=flags.Input.SecondaryFiles,
            SkipEvents=flags.Exec.SkipEvents)
        result.addService(secondarySel)
        result.addService(evSel)
        return result

    # Standard single-input path, including shared I/O and run-number overrides.
    if flags.MP.UseSharedReader:
        evSel = CompFactory.EventSelectorAthenaPoolSharedIO("EventSelector",
                                                            InputCollections=flags.Input.Files,
                                                            SkipEvents=flags.Exec.SkipEvents,
                                                            ConversionService="AthenaPoolSharedIOCnvSvc")
    else:
        evSel = CompFactory.EventSelectorAthenaPool("EventSelector",
                                                    InputCollections=flags.Input.Files,
                                                    SkipEvents=flags.Exec.SkipEvents,
                                                    ConversionService="AthenaPoolSharedIOCnvSvc" if flags.MP.UseSharedWriter else "AthenaPoolCnvSvc")
    if flags.Input.OverrideRunNumber:
        if not flags.Input.RunAndLumiOverrideList:
            DataRunNumber = -1
            FirstLB = 1
            InitialTimeStamp = 1
            OldRunNumber = -1
            if flags.Input.ConditionsRunNumber > 0:
                # Behaviour for Digitization jobs using DataRunNumber
                DataRunNumber = flags.Input.ConditionsRunNumber
                FirstLB = 1
                InitialTimeStamp = flags.IOVDb.RunToTimestampDict.get(DataRunNumber, 1) # TODO fix repeated configuration
                if not flags.Sim.DoFullChain:
                    OldRunNumber = flags.Input.RunNumbers[0] # CHECK this should be the Run Number from the HITS file
            elif flags.Input.RunNumbers:
                # Behaviour for Simulation jobs
                DataRunNumber = flags.Input.RunNumbers[0]
                FirstLB = flags.Input.LumiBlockNumbers[0]
                InitialTimeStamp = flags.Input.TimeStamps[0]
            assert DataRunNumber >= 0, (
                "flags.Input.OverrideRunNumber was True, but provided DataRunNumber (%d) is negative. "
                "Use a real run number from data." % DataRunNumber)
            evSel.OverrideRunNumber = True
            evSel.RunNumber = DataRunNumber
            evSel.FirstLB = FirstLB
            evSel.InitialTimeStamp = InitialTimeStamp # Necessary to avoid a crash
            if hasattr(evSel, "OverrideRunNumberFromInput"):
                evSel.OverrideRunNumberFromInput = True
            if OldRunNumber > 0:
                evSel.OldRunNumber = OldRunNumber
        elif flags.Common.ProductionStep in [ProductionStep.Simulation, ProductionStep.FastChain]:
            # Behaviour for Simulation and FastChain jobs using RunAndLumiOverrideList
            from AthenaKernel.EventIdOverrideConfig import getMinMaxRunNumbers, getFirstLumiBlock
            minMax = getMinMaxRunNumbers(flags)
            evSel.OverrideRunNumber = True
            evSel.RunNumber = minMax[0]
            evSel.FirstLB = getFirstLumiBlock(flags, minMax[0])
            evSel.InitialTimeStamp = flags.IOVDb.RunToTimestampDict.get(minMax[0], 1) # TODO fix repeated configuration
            if hasattr(evSel, "OverrideRunNumberFromInput"):
                evSel.OverrideRunNumberFromInput = True
        else:
            # Behaviour for Digitization jobs using RunAndLumiOverrideList
            pass
        from AthenaKernel.EventIdOverrideConfig import EvtIdModifierSvcCfg
        result.merge(EvtIdModifierSvcCfg(flags))
    elif flags.Common.ProductionStep in [ProductionStep.Simulation] and len(flags.Input.RunNumbers) and flags.Sim.ISF.ReSimulation:
        # ReSimulation case
        evSel.OverrideRunNumber = True
        evSel.RunNumber = flags.Input.RunNumbers[0]
        if flags.Input.LumiBlockNumbers:
            evSel.FirstLB = flags.Input.LumiBlockNumbers[0]
        evSel.InitialTimeStamp = flags.IOVDb.RunToTimestampDict.get(flags.Input.RunNumbers[0], 1)

    result.addService(evSel)
    return result
