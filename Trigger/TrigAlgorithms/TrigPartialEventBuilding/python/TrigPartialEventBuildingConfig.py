#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Configurable import ConfigurableCABehavior
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from libpyeformat_helper import SourceIdentifier, SubDetector
from RegionSelector import RegSelToolConfig

from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)


def getRegSelTools(flags, detNames):
    '''
    Get a list of RegionSelector tools for given detector look-up tables to build list of ROBs
    in these detectors that intersect with the RoI. Special value 'All' can be also given
    in the detNames list to include all detectors available in RegionSelector.
    '''
    # To check Detector flags before adding RegSel tool configs, we need to map RegSel det names to Detector flag names
    _regSelToDetFlagMap = {
        # Calo
        'TTEM': 'Calo',
        'TTHEC': 'Calo',
        'FCALEM': 'LAr',
        'FCALHAD': 'LAr',
        'TILE': 'Tile',
    }
    # ID
    _regSelToDetFlagMap |= dict([(d,d) for d in ['Pixel', 'SCT', 'TRT']])
    # Muon
    _regSelToDetFlagMap |= dict([(d,d) for d in ['MDT', 'RPC', 'TGC', 'CSC', 'MM']])
    _regSelToDetFlagMap['STGC'] = 'sTGC'  # inconsistent capitalisation, regSelTool_STGC_Cfg should be regSelTool_sTGC_Cfg
    if 'All' in detNames:
        detNames = _regSelToDetFlagMap.keys()

    acc = ComponentAccumulator()
    regSelTools = []
    for det in detNames:
        if det=='sTGC':
            det='STGC'  # inconsistent capitalisation, regSelTool_STGC_Cfg should be regSelTool_sTGC_Cfg
        if det not in _regSelToDetFlagMap:
            raise RuntimeError('Cannot add detector "' + det + '" because it is not in _regSelToDetFlagMap')
        detFlag = 'Enable'+_regSelToDetFlagMap[det]
        detFlagCont = getattr(flags, 'Detector')
        detEnabled = getattr(detFlagCont, detFlag)
        if not detEnabled:
            _log.debug('addRegSelDets: skip adding detector "%s" because the flag Detector.%s is False', det, detFlag)
            continue
        funcName = f'regSelTool_{det}_Cfg'
        if not hasattr(RegSelToolConfig, funcName):
            raise RuntimeError('Cannot add detector "' + det + '", RegSelToolConfig does not have a function ' + funcName)
        func = getattr(RegSelToolConfig, funcName)
        if not callable(func):
            raise RuntimeError('Cannot add detector "' + det + '", RegSelToolConfig.' + funcName + ' is not callable')
        with ConfigurableCABehavior():
            regSelTools += [acc.popToolsAndMerge(func(flags))]

    acc.setPrivateTools(regSelTools)
    return acc


def RoIPEBInfoWriterToolCfg(flags, name='RoIPEBInfoWriterTool',
                            regSelDets : list[str] = [],
                            ROBs: list[SourceIdentifier] = [],
                            subDets: list[SubDetector] = [],
                            **kwargs):
    """Configure the RoIPEBInfoWriterTool"""

    acc = ComponentAccumulator()
    acc_regsel = getRegSelTools(flags, regSelDets)

    tool = CompFactory.RoIPEBInfoWriterTool(
        name,
        RegionSelectorTools = acc_regsel.popPrivateTools(),
        ExtraROBs = [int(robid) for robid in ROBs],
        ExtraSubDets = [int(detid) for detid in subDets],
        **kwargs )

    acc.merge(acc_regsel)
    acc.setPrivateTools(tool)
    return acc


def StaticPEBInfoWriterToolCfg(flags, name='StaticPEBInfoWriterTool',
                               ROBs: list[SourceIdentifier] = [],
                               subDets : list[SubDetector] = [],
                               **kwargs):
    """Configure the StaticPEBInfoWriterTool"""

    acc = ComponentAccumulator()
    tool = CompFactory.StaticPEBInfoWriterTool(
        name,
        ROBList = [int(robid) for robid in ROBs],
        SubDetList = [int(detid) for detid in subDets],
        **kwargs )

    acc.setPrivateTools(tool)
    return acc


if __name__ == '__main__':
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    flags.lock()

    cfg = ComponentAccumulator()
    acc = RoIPEBInfoWriterToolCfg(flags,
                                  regSelDets = ['Pixel', 'SCT', 'TRT'],
                                  subDets = [SubDetector.TDAQ_CTP] )
    acc.popPrivateTools()
    cfg.merge(acc)

    acc = StaticPEBInfoWriterToolCfg(flags,
                                     subDets = [SubDetector.TDAQ_HLT],
                                     ROBs = [SourceIdentifier(SubDetector.TDAQ_CTP, 0)])
    acc.popPrivateTools()
    cfg.merge(acc)

    cfg.wasMerged()
