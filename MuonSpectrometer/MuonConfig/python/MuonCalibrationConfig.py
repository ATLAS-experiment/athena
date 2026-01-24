# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Based on : https://gitlab.cern.ch/atlas/athena/blob/master/MuonSpectrometer/MuonCnv/MuonCnvExample/python/MuonCalibConfig.py

from MuonConfig.MuonCondAlgConfig import CscCondDbAlgCfg, NswCalibDbAlgCfg
from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType, LHCPeriod
from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg

from AthenaCommon.Logging import logging
log = logging.getLogger('MuonCalibConfig')

################################################################################
# CSC calibration
################################################################################

def CscCalibToolCfg(flags, name="CscCalibTool", **kwargs):
    """Return ComponentAccumulator configured for CSC calibration with CscCalibTool as PrivateTools"""

    acc = CscCondDbAlgCfg(flags)

    kwargs.setdefault("Slope", 0.19)
    kwargs.setdefault("Noise", 3.5)
    kwargs.setdefault("Pedestal", 2048.0)
    kwargs.setdefault("ReadFromDatabase", True)
    kwargs.setdefault("SlopeFromDatabase", False)
    kwargs.setdefault("integrationNumber", 12.0)
    kwargs.setdefault("integrationNumber2", 11.66)
    kwargs.setdefault("samplingTime", 50.0)
    kwargs.setdefault("signalWidth", 14.40922)
    kwargs.setdefault("timeOffset", 71.825)
    kwargs.setdefault("IsOnline", flags.Common.isOnline)
    kwargs.setdefault("Latency", 119)

    acc.setPrivateTools(CompFactory.CscCalibTool(name, **kwargs))

    return acc



################################################################################
# MDT calibration
################################################################################

def _setupMdtCondDB(flags):
    result=ComponentAccumulator()
    
    if flags.Muon.Calib.readMDTCalibFromBlob:
        mdt_folder_name_appendix = "BLOB" 
    else:
        mdt_folder_name_appendix=""
    
    online_folders = ['/MDT/Onl/RT'+ mdt_folder_name_appendix,'/MDT/Onl/T0' + mdt_folder_name_appendix]
    offline_folders = ['/MDT/RT' + mdt_folder_name_appendix, '/MDT/T0' + mdt_folder_name_appendix]

    if flags.Muon.Calib.mdtCalibrationSource=="MDT":
        if flags.GeoModel.Run is LHCPeriod.Run4:
            # TODO: temporary conditions override until we get a global tag
            from IOVDbSvc.IOVDbSvcConfig import addFolders
            # Ugly, but hopefully temporary hack
            if flags.GeoModel.AtlasVersion.startswith('ATLAS-P2-RUN4-01-00'):
                result.merge(addFolders(flags, '/MDT/RT' + mdt_folder_name_appendix, 'MDT_OFL', className='CondAttrListCollection', tag='MDTRT_Sim-Run4-01', db="OFLP200"))
                result.merge(addFolders(flags, '/MDT/T0' + mdt_folder_name_appendix, 'MDT_OFL', className='CondAttrListCollection', tag='MDTT0_Sim-Run4-01', db="OFLP200"))
            else:
                result.merge(addFolders(flags, '/MDT/RT' + mdt_folder_name_appendix, 'MDT_OFL', className='CondAttrListCollection', tag='MDTRT_Sim-R3SYM-04', db="OFLP200"))
                result.merge(addFolders(flags, '/MDT/T0' + mdt_folder_name_appendix, 'MDT_OFL', className='CondAttrListCollection', tag='MDTT0_Sim-R3SYM-03', db="OFLP200"))
        else:
            result.merge(addFoldersSplitOnline(flags, 'MDT', online_folders , offline_folders,
                                               className = 'CondAttrListCollection' ) )
    else:
        result.merge(addFoldersSplitOnline(flags, flags.Muon.Calib.mdtCalibrationSource, online_folders, offline_folders,
                                           className = 'CondAttrListCollection' ) )
        
    return result, mdt_folder_name_appendix
# end of function setupMdtCondDB()

@AccumulatorCache
def MdtCalibrationToolCfg(flags, name= "MdtCalibrationTool",  **kwargs):
    result=ComponentAccumulator()
    result.merge(MdtCalibDbAlgCfg(flags))

    kwargs.setdefault("DoSlewingCorrection", flags.Muon.Calib.correctMdtRtForTimeSlewing)
    kwargs.setdefault("DoTemperatureCorrection", flags.Muon.Calib.applyRtScaling)
    kwargs.setdefault("DoTofCorrection", flags.Beam.Type is BeamType.Collisions) # No TOF correction if not collisions
    kwargs.setdefault("DoPropagationTimeUncert", flags.Muon.Calib.applySigPropUncert)

    if flags.Beam.Type is BeamType.Collisions:
        from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MdtCalibWindowNumber
        kwargs.setdefault("TimeWindowSetting", MdtCalibWindowNumber('Collision_G4'))

    result.merge(AtlasFieldCacheCondAlgCfg(flags))

    mdt_calibration_tool = CompFactory.MdtCalibrationTool(name= name, **kwargs)
    result.setPrivateTools(mdt_calibration_tool)
    return result

def MdtCalibDbAlgR4Cfg(flags, name="MdtCalibDbAlg",**kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("PropagationSpeedBeta", flags.Muon.Calib.mdtPropagationSpeedBeta)
    kwargs.setdefault("CreateBFieldFunctions", flags.Muon.Calib.correctMdtRtForBField)
    kwargs.setdefault("CreateSlewingFunctions", flags.Muon.Calib.correctMdtRtForTimeSlewing)
    kwargs.setdefault("RtJSON","")
    kwargs.setdefault("TubeT0JSON","")

    if(kwargs["RtJSON"] or kwargs["TubeT0JSON"]):
        kwargs.setdefault("ReadKeyRt","") 
        kwargs.setdefault("ReadKeyTube","")
        kwargs.setdefault("dbPayloadType","")

    else:
        kwargs.setdefault("ReadKeyRt","/MDT/RTJSONS") 
        kwargs.setdefault("ReadKeyTube","/MDT/T0JSONS")
        kwargs.setdefault("dbPayloadType","TTree")
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        result.merge(addFolders(flags,[kwargs["ReadKeyRt"]], className='CondAttrListCollection', detDb="MDT_OFL", tag="MDTRTTREE-RUN4-02"))
        result.merge(addFolders(flags,[kwargs["ReadKeyTube"]], className='CondAttrListCollection', detDb="MDT_OFL", tag="MDTT0TREE-RUN4-02"))

    alg = CompFactory.MuonCalibR4.MdtCalibDbAlg(name, **kwargs)
    result.addCondAlgo (alg, primary = True)
    return result



def MdtCalibDbAlgCfg(flags,name="MdtCalibDbAlg",**kwargs):
    result = ComponentAccumulator()
    result.merge(MuonGeoModelCfg(flags))    
    # setup COOL folders
    if not flags.Muon.Calib.readMdtJSON:
        acc, mdt_folder_name_appendix = _setupMdtCondDB(flags)
        result.merge(acc)
    
    if not flags.Muon.useMdtDcsData:
        kwargs.setdefault("ReadKeyDCS", "" )
    else:
        from MuonConfig.MuonCondAlgConfig import MdtCondDbAlgCfg
        result.merge(MdtCondDbAlgCfg(flags))
    
    if flags.Muon.Calib.fitAnalyticRt:        
        from MuonCondAlgR4.ConditionsConfig import MdtAnalyticRtCalibAlgCfg
        kwargs.setdefault("WriteKey", "LookUpMdtCalibDb")
        result.merge(MdtAnalyticRtCalibAlgCfg(flags, ReadKey="LookUpMdtCalibDb"))
    if flags.Muon.Calib.readMdtJSON:
        result.merge(MdtCalibDbAlgR4Cfg(flags, name, **kwargs))
        return result

    # set some default proper ties
    if flags.Common.isOnline and not flags.Input.isMC:
       kwargs.setdefault("ReadKeyTube", "/MDT/T0")
       kwargs.setdefault("ReadKeyRt",  "/MDT/RT")
    else:
       kwargs.setdefault("ReadKeyTube", "/MDT/T0"+ mdt_folder_name_appendix)
       kwargs.setdefault("ReadKeyRt", "/MDT/RT"+ mdt_folder_name_appendix)
    if flags.Input.isMC is False: # Should be " if flags.Input.isMC=='data' " ?
        kwargs.setdefault("defaultT0", 40)
    else:
        kwargs.setdefault("defaultT0", 799)
    
    kwargs.setdefault("UseMLRt",  flags.Muon.Calib.useMLRt )
    kwargs.setdefault("TimeSlewingCorrection", flags.Muon.Calib.correctMdtRtForTimeSlewing)
    kwargs.setdefault("MeanCorrectionVsR", [ -5.45973, -4.57559, -3.71995, -3.45051, -3.4505, -3.4834, -3.59509, -3.74869, -3.92066, -4.10799, -4.35237, -4.61329, -4.84111, -5.14524 ])
    kwargs.setdefault("PropagationSpeedBeta", flags.Muon.Calib.mdtPropagationSpeedBeta)
    # the same as MdtCalibrationDbTool
    kwargs.setdefault("CreateBFieldFunctions", flags.Muon.Calib.correctMdtRtForBField)
    kwargs.setdefault("CreateSlewingFunctions", flags.Muon.Calib.correctMdtRtForTimeSlewing)
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("AthRNGSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))

    kwargs.setdefault("UseR4DetMgr", flags.Muon.usePhaseIIGeoSetup)
    alg = CompFactory.Muon.MdtCalibDbAlg (name, **kwargs)
    result.addCondAlgo (alg, primary = True)
    return result

def NSWCalibToolCfg(flags, name="NSWCalibTool", **kwargs):
    """Return ComponentAccumulator configured for NSW calibration with NSWCalibTool as PrivateTools"""
    result = ComponentAccumulator()
    result.merge(NswCalibDbAlgCfg(flags))
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    result.merge(AtlasFieldCacheCondAlgCfg(flags))
    kwargs.setdefault("isData", not flags.Input.isMC)
    kwargs.setdefault("mmPeakTime",200)
    kwargs.setdefault("sTgcPeakTime",0)
    kwargs.setdefault("applyMmT0Calib", flags.Muon.Calib.applyMmT0Correction)    
    kwargs.setdefault("applysTgcT0Calib", flags.Muon.Calib.applysTgcT0Correction)
    kwargs.setdefault("applyMmBFieldCalib",flags.Muon.Calib.applyMmBFieldCalib)    
    the_tool = CompFactory.Muon.NSWCalibTool(name,**kwargs)
    result.setPrivateTools(the_tool)
    return result

def MMCalibSmearingToolCfg(flags, name="MMCalibSmearingTool", **kwargs):
    """Return ComponentAccumulator configured for MM smearing with NSWCalibSmearing as PrivateTools"""
    result = ComponentAccumulator()
    kwargs.setdefault("EtaSectors", [True, True, True, True])
    the_tool = CompFactory.Muon.NSWCalibSmearingTool(name,**kwargs)
    result.setPrivateTools(the_tool)
    return result

def STgcCalibSmearingToolCfg(flags, name="STgcCalibSmearingTool", **kwargs):
    """Return ComponentAccumulator configured for sTGC smearing with NSWCalibSmearing as PrivateTools"""
    result = ComponentAccumulator()
    kwargs.setdefault("EtaSectors", [True, True, True, True, True, True])
    the_tool = CompFactory.Muon.NSWCalibSmearingTool(name,**kwargs)
    result.setPrivateTools(the_tool)
    return result

def NswErrorCalibDbAlgCfg(flags, name = "NswErrorCalibDbAlg", **kwargs):
    result = ComponentAccumulator()    
    folderNames = []
    if "readFromJSON" in kwargs:
        folderNames=[]

    elif flags.Common.isOnline:
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        folderNames+=["/MDT/Onl/MM/ClusterUncertainties/SIDEA",
                      "/MDT/Onl/MM/ClusterUncertainties/SIDEC",
                      "/TGC/Onl/NSW/ClusterUncertainties/SIDEA",
                      "/TGC/Onl/NSW/ClusterUncertainties/SIDEC",
                      ]
        result.merge(addFolders(flags,folderNames[:2], className='CondAttrListCollection', detDb="MDT_ONL"))
        result.merge(addFolders(flags,folderNames[2:], className='CondAttrListCollection', detDb="TGC_ONL"))

    else:
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        folderNames+=["/MDT/MM/ClusterUncertainties/SIDEA", 
                      "/MDT/MM/ClusterUncertainties/SIDEC",
                      "/TGC/NSW/ClusterUncertainties/SIDEA",
                      "/TGC/NSW/ClusterUncertainties/SIDEC"]
        result.merge(addFolders(flags,folderNames[:2], className='CondAttrListCollection', detDb="MDT_OFL"))
        result.merge(addFolders(flags,folderNames[2:], className='CondAttrListCollection', detDb="TGC_OFL"))
         
    kwargs.setdefault("ReadKeys", folderNames) 
    the_alg = CompFactory.Muon.NswUncertDbAlg(name = name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result

def MmCTPCondDbAlgCfg(flags, name = "MmCTPCondDbAlg", **kwargs):
    result = ComponentAccumulator()
    if "readFromJSON" in kwargs:
      kwargs.setdefault("ReadKey", "")
    else:
      from IOVDbSvc.IOVDbSvcConfig import addFolders
      kwargs.setdefault("ReadKey", "/MDT/MM/CTPSLOPE")
      result.merge(addFolders(flags, kwargs["ReadKey"], className='CondAttrListCollection', detDb="MDT_OFL")) 

    the_alg = CompFactory.Muon.MmCTPCondDbAlg(name = name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result
