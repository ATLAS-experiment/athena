# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
import ROOT


def EventSelectorAthenaPoolCfg(flags):
    result = ComponentAccumulator()
    evSel = CompFactory.EventSelectorAthenaPool("EventSelector",
                                                InputCollections=flags.Input.Files,
                                                SkipEvents=flags.Exec.SkipEvents,
                                                ConversionService="AthenaPoolSharedIOCnvSvc" if flags.MP.UseSharedReader or flags.MP.UseSharedWriter else "AthenaPoolCnvSvc")
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


def PoolReadCfg(flags):
    """
    Creates a ComponentAccumulator instance containing the
    athena services required for POOL file reading
    """

    result = ComponentAccumulator()

    from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolAddressProviderSvcCfg
    if flags.MP.UseSharedReader or flags.MP.UseSharedWriter:
        from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolSharedIOCnvSvcCfg
        result.merge(AthenaPoolSharedIOCnvSvcCfg(flags, InputPoolAttributes=["DatabaseName = '*'; ContainerName = 'CollectionTree'; TREE_CACHE = '-1'"]))
    else:
        from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolCnvSvcCfg
        result.merge(AthenaPoolCnvSvcCfg(flags, InputPoolAttributes=["DatabaseName = '*'; ContainerName = 'CollectionTree'; TREE_CACHE = '-1'"]))

    # Suppress ROOT warnings about old I/O classes.
    from AthenaServices.ROOTMessageFilterSvcConfig import ROOTMessageFilterSvcCfg
    result.merge(ROOTMessageFilterSvcCfg(flags,
                                         SuppressionRules=[('TClass::Init',
                                                            '.*DataHeader.*_p[12].*',
                                                            ROOT.kWarning),
                                                            ('TClass::Init',
                                                            '.*PoolToken_p1.*',
                                                            ROOT.kWarning)]))

    if flags.Input.SecondaryFiles:
        skipEventsPrimary = flags.Exec.SkipEvents
        skipEventsSecondary = flags.Exec.SkipEvents
        if flags.Overlay.SkipSecondaryEvents >= 0:
            skipEventsSecondary = flags.Overlay.SkipSecondaryEvents

        # Create DoubleEventSelector (universal for any seconday input type)
        evSel = CompFactory.DoubleEventSelectorAthenaPool("EventSelector",
                                                          InputCollections=flags.Input.Files)
        evSel.CollectionType = "RootCollection"

        if flags.Overlay.ByteStream:
            # In case of data overlay HITS are primary input
            evSel.SkipEvents = skipEventsPrimary

            # We have to check if we're running overlay with ByteStream input
            from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
            result.merge(ByteStreamReadCfg(flags))

            # We still have to add primary address provider
            result.merge(AthenaPoolAddressProviderSvcCfg(flags,
                                                         name="AthenaPoolAddressProviderSvcPrimary",
                                                         DataHeaderKey="EventSelector"))
        else:
            # In case of MC overlay RDOs are primary input
            evSel.SkipEvents = skipEventsSecondary
            # Do not process secondary input metadata
            evSel.ProcessMetadata = False

            # We have primary and secondary pool inputs, create two address providers
            result.merge(AthenaPoolAddressProviderSvcCfg(flags,
                                                         name="AthenaPoolAddressProviderSvcPrimary",
                                                         DataHeaderKey="EventSelector",
                                                         AttributeListKey="Input"))
            result.merge(AthenaPoolAddressProviderSvcCfg(flags,
                                                         name="AthenaPoolAddressProviderSvcSecondary",
                                                         DataHeaderKey="SecondaryEventSelector"))

            secondarySel = CompFactory.EventSelectorAthenaPool("SecondaryEventSelector",
                                                               IsSecondary=True,
                                                               InputCollections=flags.Input.SecondaryFiles,
                                                               SkipEvents=skipEventsPrimary)
            secondarySel.CollectionType = "RootCollection"
            result.addService(secondarySel)
        result.addService(evSel)
    else:
        # We have only primary inputs
        result.merge(AthenaPoolAddressProviderSvcCfg(flags))
        result.merge(EventSelectorAthenaPoolCfg(flags))
        evSel = result.getService("EventSelector")

        #Schedule a (potential) AODFix ...
        processingTags=flags.Input.ProcessingTags
        if "StreamAOD" in processingTags:
            try:
                from RecJobTransforms.AODFixConfig import AODFixCfg
                result.merge(AODFixCfg(flags))
            except ImportError:
                #Looks like running on AthSimulation or AthAnalysis ... ignore AODFix
                pass



    result.setAppProperty("EvtSel", evSel.getFullJobOptName())

    return result
