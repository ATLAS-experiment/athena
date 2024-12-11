# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep

# based on https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/Control/AthenaServices/python/Configurables.py#0247
def add_modifier(run_nbr=None, evt_nbr=None, time_stamp=None, lbk_nbr=None, nevts=1):

    if run_nbr is None:
        modify_run_nbr = 0
        run_nbr = 0
    else:
        modify_run_nbr = 1

    if evt_nbr is None:
        modify_evt_nbr = 0
        evt_nbr = 0
    else:
        modify_evt_nbr = 1

    if time_stamp is None:
        modify_time_stamp = 0
        time_stamp = 0
    else:
        modify_time_stamp = 1

    if lbk_nbr is None:
        modify_lbk_nbr = 0
        lbk_nbr = 0
    else:
        modify_lbk_nbr = 1

    mod_bit = int(0b0000
                  | (modify_run_nbr << 0)
                  | (modify_evt_nbr << 1)
                  | (modify_time_stamp << 2)
                  | (modify_lbk_nbr << 3))

    return [run_nbr, evt_nbr, time_stamp, lbk_nbr, nevts, mod_bit]


def buildListOfModifiers(flags):
    # migrated from RunDMCFlags.py
    Modifiers = []
    pDicts = flags.Input.RunAndLumiOverrideList
    DataRunNumber = flags.Input.ConditionsRunNumber

    if pDicts:
        for el in pDicts:
            evt_nbr = el.get("evt_nbr", None)
            Modifiers += add_modifier(run_nbr=el["run"], evt_nbr=evt_nbr, time_stamp=el["starttstamp"], lbk_nbr=el["lb"], nevts=el["evts"])
    elif DataRunNumber>0:
        assert DataRunNumber >= 0, (
            "flags.Input.ConditionsRunNumber %d is negative. "
            "Use a real run number from data." % DataRunNumber)

        # Using event numbers to avoid "some very large number" setting
        totalNumber = 1000000
        if flags.Exec.MaxEvents > 0:
            totalNumber = flags.Exec.MaxEvents + 1
        if flags.Exec.SkipEvents > 0:
            totalNumber += flags.Exec.SkipEvents

        InitialTimeStamp = flags.IOVDb.RunToTimestampDict.get(DataRunNumber, 1) # TODO fix repeated configuration

        FirstLB = 1
        Modifiers += add_modifier(run_nbr=DataRunNumber, lbk_nbr=FirstLB, time_stamp=InitialTimeStamp, nevts=totalNumber)
    elif flags.Input.RunNumbers:
        # Behaviour for Simulation jobs. For standard Simulation we
        # override the run number once per job. TODO Still need to deal with the specific case of DataOverlay
        myRunNumber = flags.Input.RunNumbers[0]
        assert myRunNumber >= 0, (
            "flags.Input.RunNumbers[0] %d is negative. "
            "Use a real run number from data." % myRunNumber)
        myFirstLB = flags.Input.LumiBlockNumbers[0]
        myInitialTimeStamp = flags.Input.TimeStamps[0]

        # Using event numbers to avoid "some very large number" setting
        totalNumber = 1000000
        if flags.Exec.MaxEvents > 0:
            totalNumber = flags.Exec.MaxEvents + 1
        if flags.Exec.SkipEvents > 0:
            totalNumber += flags.Exec.SkipEvents
        Modifiers += add_modifier(run_nbr=myRunNumber, lbk_nbr=myFirstLB, time_stamp=myInitialTimeStamp, nevts=totalNumber)
    return Modifiers


def getFirstLumiBlock(flags, run):
    pDicts = flags.Input.RunAndLumiOverrideList
    if pDicts:
        allLBs = [1]
        for el in pDicts:
            if el["run"] == run:
                allLBs += [el["lb"]]
        return min(allLBs) + 0
    else:
        return flags.Input.LumiBlockNumbers[0]


def getMinMaxRunNumbers(flags):
    """Get a pair (firstrun,lastrun + 1) for setting ranges in IOVMetaData """
    mini = 1
    maxi = 2147483647
    if flags.Input.RunAndLumiOverrideList:
        # Behaviour for Digitization jobs using RunAndLumiOverrideList
        allruns = [element['run'] for element in flags.Input.RunAndLumiOverrideList]
        mini = min(allruns) + 0
        maxi = max(allruns) + 1
    elif flags.Input.ConditionsRunNumber > 0:
        # Behaviour for Digitization jobs using DataRunNumber
        assert flags.Input.ConditionsRunNumber >= 0, (
            "flags.Input.ConditionsRunNumber %d is negative. "
            "Use a real run number from data." % flags.Input.ConditionsRunNumber)
        mini = flags.Input.ConditionsRunNumber
        maxi = mini + 1
    elif flags.Input.RunNumbers:
        # Behaviour for Simulation jobs
        assert flags.Input.RunNumbers[0] >= 0, (
            "flags.Input.RunNumbers[0] %d is negative. "
            "Use a real run number from data." % flags.Input.RunNumbers[0])
        if flags.Input.isMC:
            mini = flags.Input.RunNumbers[0]
            maxi = 2147483647
        else:
            mini = min(flags.Input.RunNumbers) + 0
            maxi = max(flags.Input.RunNumbers) + 1
    return (mini, maxi)


def IOVDbMetaDataToolWithRunNumberOverrideCfg(flags):
    """Metadata override for overlay"""
    tool = CompFactory.IOVDbMetaDataTool()
    tool.MinMaxRunNumbers = getMinMaxRunNumbers(flags)
    acc = ComponentAccumulator()
    acc.addPublicTool(tool)
    return acc


def EvtIdModifierSvcCfg(flags, name="EvtIdModifierSvc", **kwargs):
    acc = ComponentAccumulator()
    isMT = flags.Concurrency.NumThreads > 0
    pileUp = flags.Common.ProductionStep in [ProductionStep.Digitization, ProductionStep.PileUpPresampling, ProductionStep.FastChain] \
                and flags.Digitization.PileUp
    if pileUp and not isMT:
        kwargs.setdefault("EvtStoreName", "OriginalEvent_SG")
    elif pileUp:
        kwargs.setdefault("SkippedEvents", flags.Exec.SkipEvents)
    else:
        kwargs.setdefault("EvtStoreName", "StoreGateSvc")

    Modifiers = buildListOfModifiers(flags)
    if len(Modifiers) > 0:
        kwargs.setdefault("Modifiers", Modifiers)

    acc.merge(IOVDbMetaDataToolWithRunNumberOverrideCfg(flags))

    acc.addService(CompFactory.EvtIdModifierSvc(name, **kwargs), create=True, primary=True)
    return acc
