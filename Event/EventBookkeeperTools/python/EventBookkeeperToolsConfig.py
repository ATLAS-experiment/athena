"""Define functions for event bookkeeping configuration using ComponentAccumulator

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg


def BookkeeperToolCfg(flags, name='BookkeeperTool', output_name='CutBookkeepers'):
    """BookkeeperTool config"""
    tool = CompFactory.BookkeeperTool(name,
                                      InputCollName = output_name,
                                      OutputCollName = output_name)
    acc = MetaDataSvcCfg(flags, tools=[tool])
    acc.addPublicTool(tool)
    return acc


def BookkeeperDumperToolCfg(flags):
    """BookkeeperDumperTool configuration"""
    return MetaDataSvcCfg(flags, toolNames=['BookkeeperDumperTool'])


def CutFlowSvcCfg(flags, **kwargs):
    """CutFlowSvc configuration"""
    acc = BookkeeperToolCfg(flags)

    kwargs.setdefault('Configured', True)
    # Determine current input stream name
    kwargs.setdefault('InputStream', flags.Input.ProcessingTags[-1] if flags.Input.ProcessingTags else 'N/A')
    # Configure skimming cycle
    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    kwargs.setdefault('SkimmingCycle', GetFileMD(flags.Input.Files).get('currentCutCycle', -1) + 1)

    # Init the service
    acc.addService(CompFactory.CutFlowSvc(**kwargs))

    # TODO: different sequence?
    acc.addEventAlgo(CompFactory.AllExecutedEventsCounterAlg())

    return acc


def AllWrittenEventsCounterAlgCfg(flags):
    """AllWrittenEventsCounterAlg configuration"""
    acc = ComponentAccumulator(CompFactory.AthSequencer("AthOutSeq", StopOverride=True))
    acc.addEventAlgo(CompFactory.AllWrittenEventsCounterAlg())
    return acc


def CutFlowOutputList(flags, output_name='CutBookkeepers'):
    """CutFlow output metadata list"""
    return [
        'xAOD::CutBookkeeperContainer#' + output_name + '*',
        'xAOD::CutBookkeeperAuxContainer#' + output_name + '*Aux.*',
        'xAOD::CutBookkeeperContainer#Incomplete' + output_name + '*',
        'xAOD::CutBookkeeperAuxContainer#Incomplete' + output_name + '*Aux.*'
    ]
