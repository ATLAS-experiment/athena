#
#   @file    RegSelToolConfig.py
#
#            configuration functions for the new RegSelTools 
#
#   @author  sutt 
#
#   @date    Sun  8 Mar 2020 03:27:57 GMT
#                 
#   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration#
#
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)
    
def _condAlgName(detector):
    return "RegSelCondAlg_"+detector

def _createRegSelCondAlg( detector,  CondAlgConstructor, useMdtDcsData, printTable=False ):
    """
    Creates conditions alg that provides data to a RegSel Tool
    """
    if detector == "TRT":
        condAlg = CondAlgConstructor( name = _condAlgName( detector ),
                                      PrintTable  = printTable,
                                      RegSelLUT = ("RegSelLUTCondData_"+detector) )
    else:
        condAlg = CondAlgConstructor( name = _condAlgName( detector ),
                                      ManagerName = detector,
                                      PrintTable  = printTable,
                                      RegSelLUT = ("RegSelLUTCondData_"+detector) )

    if detector == "MDT" and not useMdtDcsData:
         condAlg.Conditions = "" 
    elif detector == "Pixel":
        condAlg.DetEleCollKey = "PixelDetectorElementCollection"
        condAlg.PixelCablingCondData = "PixelCablingCondData"
    elif detector == "SCT":
        condAlg.DetEleCollKey = "SCT_DetectorElementCollection"
        condAlg.SCT_CablingData = "SCT_CablingData"
    elif detector == "ITkPixel":
        condAlg.DetEleCollKey = "ITkPixelDetectorElementCollection"
        # No cabling data for ITk
        condAlg.PixelCablingCondData = ""
    elif detector == "ITkStrip":
        condAlg.DetEleCollKey = "ITkStripDetectorElementCollection"
        # No cabling data for ITk
        condAlg.SCT_CablingData = ""
    return condAlg

def _createRegSelTool( detector, enable ):
    """
    Creates RegSelTool and corresponding cond tool that is needed for its function

    If the enable flag is set - the tool is properly configured, else it is configured NOT to provide the data.

    """

    
    tool = CompFactory.RegSelTool(name="RegSelTool_"+detector)

    # should we enable the look up table access for this subsystem ?

    if not enable:
        # detector not configured so don't enable
        # lookup table access
        tool.Initialised = False
        return tool
        
    # add the lookup table to retrieve
        
    tool.RegSelLUT = "RegSelLUTCondData_"+detector # has to match wiht appropriate RegSelCondAlg
    tool.Initialised = True
    return tool


def regSelToolCfg(flags, detector, algorithm, readout_geometry=None, conditions=None):
    ca = ComponentAccumulator()
    if readout_geometry:
        ca.merge(readout_geometry)
    if conditions:
        ca.merge(conditions)
    ca.setPrivateTools(_createRegSelTool(detector, True))

    # test if we have a PrintLUT flag ... 
    printLUT = False
    if flags.hasFlag("PrintLUT"):
        printLUT = flags.PrintLUT
        
    the_alg = _createRegSelCondAlg(detector, algorithm, flags.Muon.useMdtDcsData, printTable=printLUT )
    ca.addCondAlgo(the_alg)
    return ca


# inner detector
@AccumulatorCache
def regSelTool_Pixel_Cfg(flags):
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelCablingCondAlgCfg
    return regSelToolCfg(flags, "Pixel", CompFactory.SiRegSelCondAlg,
                         readout_geometry=PixelReadoutGeometryCfg(flags), conditions=PixelCablingCondAlgCfg(flags))

@AccumulatorCache
def regSelTool_SCT_Cfg(flags):
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    from SCT_Cabling.SCT_CablingConfig import SCT_CablingCondAlgCfg
    return regSelToolCfg(flags, "SCT", CompFactory.SiRegSelCondAlg,
                         readout_geometry=SCT_ReadoutGeometryCfg(flags), conditions=SCT_CablingCondAlgCfg(flags))

@AccumulatorCache
def regSelTool_TRT_Cfg(flags):
    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    return regSelToolCfg(flags, "TRT", CompFactory.TRT_RegSelCondAlg,
                         readout_geometry=TRT_ReadoutGeometryCfg(flags))

# ITk
@AccumulatorCache
def regSelTool_ITkPixel_Cfg(flags):
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    return regSelToolCfg(flags, "ITkPixel", CompFactory.SiRegSelCondAlg,
                         readout_geometry=ITkPixelReadoutGeometryCfg(flags))

@AccumulatorCache
def regSelTool_ITkStrip_Cfg(flags):
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    return regSelToolCfg(flags, "ITkStrip", CompFactory.SiRegSelCondAlg,
                         readout_geometry=ITkStripReadoutGeometryCfg(flags))


# muon spectrometer
@AccumulatorCache
def regSelTool_MDT_Cfg(flags):
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    from MuonConfig.MuonCondAlgConfig import MdtCondDbAlgCfg
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg

    conditions = ComponentAccumulator()
    conditions.merge(MuonGeoModelCfg(flags))
    conditions.merge(MDTCablingConfigCfg(flags))
    if flags.Muon.useMdtDcsData: #false for online and MDT calibration stream processing 
        conditions.merge(MdtCondDbAlgCfg(flags))

    return regSelToolCfg(flags, "MDT", CompFactory.MDT_RegSelCondAlg,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_RPC_Cfg(flags):
    from MuonConfig.MuonCablingConfig import RPCCablingConfigCfg
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg

    conditions = ComponentAccumulator()
    conditions.merge(MuonGeoModelCfg(flags))
    conditions.merge(RPCCablingConfigCfg(flags))

    return regSelToolCfg(flags, "RPC", CompFactory.RPC_RegSelCondAlg,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_TGC_Cfg(flags):
    from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg

    conditions = ComponentAccumulator()
    conditions.merge(MuonGeoModelCfg(flags))
    conditions.merge(TGCCablingConfigCfg(flags))

    return regSelToolCfg(flags, "TGC", CompFactory.TGC_RegSelCondAlg,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_CSC_Cfg(flags):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    from MuonConfig.MuonCablingConfig import CSCCablingConfigCfg

    conditions = ComponentAccumulator()
    conditions.merge(MuonGeoModelCfg(flags))
    conditions.merge(CSCCablingConfigCfg(flags))

    return regSelToolCfg(flags, "CSC", CompFactory.CSC_RegSelCondAlg,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_STGC_Cfg(flags):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    return regSelToolCfg(flags, "sTGC", CompFactory.sTGC_RegSelCondAlg,
                         conditions=MuonGeoModelCfg(flags))

@AccumulatorCache
def regSelTool_MM_Cfg(flags):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    return regSelToolCfg(flags, "MM", CompFactory.MM_RegSelCondAlg,
                         conditions=MuonGeoModelCfg(flags))


# calo
@AccumulatorCache
def regSelTool_TTEM_Cfg(flags):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from LArRecUtils.LArRecUtilsConfig import LArRoIMapCondAlgCfg

    conditions = ComponentAccumulator()
    conditions.merge(LArGMCfg(flags))
    conditions.merge(LArRoIMapCondAlgCfg(flags))

    return regSelToolCfg(flags, "TTEM", CompFactory.RegSelCondAlg_LAr,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_TTHEC_Cfg(flags):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from LArRecUtils.LArRecUtilsConfig import LArRoIMapCondAlgCfg

    conditions = ComponentAccumulator()
    conditions.merge(LArGMCfg(flags))
    conditions.merge(LArRoIMapCondAlgCfg(flags))

    return regSelToolCfg(flags, "TTHEC", CompFactory.RegSelCondAlg_LAr,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_FCALEM_Cfg(flags):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from LArRecUtils.LArRecUtilsConfig import LArRoIMapCondAlgCfg

    conditions = ComponentAccumulator()
    conditions.merge(LArGMCfg(flags))
    conditions.merge(LArRoIMapCondAlgCfg(flags))

    return regSelToolCfg(flags, "FCALEM", CompFactory.RegSelCondAlg_LAr,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_FCALHAD_Cfg(flags):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from LArRecUtils.LArRecUtilsConfig import LArRoIMapCondAlgCfg

    conditions = ComponentAccumulator()
    conditions.merge(LArGMCfg(flags))
    conditions.merge(LArRoIMapCondAlgCfg(flags))

    return regSelToolCfg(flags, "FCALHAD", CompFactory.RegSelCondAlg_LAr,
                         conditions=conditions)

@AccumulatorCache
def regSelTool_TILE_Cfg(flags):
    from TileByteStream.TileHid2RESrcIDConfig import TileHid2RESrcIDCondAlgCfg
    return regSelToolCfg(flags, "TILE", CompFactory.RegSelCondAlg_Tile,
                         conditions=TileHid2RESrcIDCondAlgCfg(flags, ForHLT=True))


def regSelToolsCfg( flags, detNames ):
    '''
    Get a list of RegionSelector tools for given detector look-up tables if the corresponding Detector flags are enabled
    '''
    acc = ComponentAccumulator()
    regSelTools = []
    for det in detNames:
        flagName = det
        if det in ['TTEM', 'TTHEC', 'FCALEM', 'FCALHAD']:
            flagName = 'LAr'
        elif det == 'TILE':
            flagName = 'Tile'
        elif det == 'STGC':
            flagName = 'sTGC'
        detFlag = f'Enable{flagName}'
        detEnabled = getattr(flags.Detector, detFlag)
        if not detEnabled:
            _log.debug('regSelToolsCfg: skip adding detector "%s" because the flag Detector.%s is False', det, detFlag)
            continue
        funcName = f'regSelTool_{det}_Cfg'
        func = globals().get(funcName, None)
        if func is None:
            raise RuntimeError('regSelToolsCfg: cannot add detector "' + det + '", RegSelToolConfig does not have a function ' + funcName)
        regSelTools += [acc.popToolsAndMerge(func(flags))]
    acc.setPrivateTools(regSelTools)
    return acc


# unit test
if __name__=='__main__':
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration import DetectorConfigFlags, TestDefaults
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    import sys

    flags = initConfigFlags()
    flags.Input.Files = TestDefaults.defaultTestFiles.RAW_RUN3
    flags.GeoModel.AtlasVersion = TestDefaults.defaultGeometryTags.RUN3
    flags.IOVDb.GlobalTag = TestDefaults.defaultConditionsTags.RUN3_DATA23
    flags.Exec.MaxEvents = 1
    flags.Concurrency.NumThreads = 1

    # Strict dependency checking
    flags.Input.FailOnUnknownCollections = True
    flags.Scheduler.AutoLoadUnmetDependencies = False

    # have to add a printLUT=True flag, so it can be tested in the regSelToolCgf
    # and passed in to _createRe=gSelTool woithout having to add it to *every* tool config

    flags.addFlag( "PrintLUT", True )
    
    detNames = sys.argv[1:]
    
    # Toggle detectors. Note that we are not toggling the geometry because this would
    # result in un-physical configurations that we are not supporting in reco anyway.

    DetectorConfigFlags.disableDetectors(flags, DetectorConfigFlags.allDetectors +
                                         list(DetectorConfigFlags.allGroups.keys()), toggle_geometry=False)

    DetectorConfigFlags.enableDetectors(flags, detNames, toggle_geometry=False)

    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(ByteStreamReadCfg(flags))

    if 'LAr' in detNames:
        detNames.remove('LAr')
        detNames += ['TTEM', 'TTHEC', 'FCALEM', 'FCALHAD']
    if 'Tile' in detNames:
        detNames.remove('Tile')
        detNames.append('TILE')
    if 'sTGC' in detNames:
        detNames.remove('sTGC')
        detNames.append('STGC')

    toolsCfg = regSelToolsCfg(flags, detNames)

    alg = CompFactory.RegSelToolTester(
        RegionSelectorTools = acc.popToolsAndMerge(toolsCfg) )
    
    acc.addEventAlgo(alg, sequenceName='AthAlgSeq')

    sys.exit(acc.run().isFailure())
