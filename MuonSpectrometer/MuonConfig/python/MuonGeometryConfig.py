# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.Enums import ProductionStep
from AtlasGeoModel.GeoModelConfig import GeoModelCfg
from AthenaConfiguration.Enums import LHCPeriod

def TrackingVolumesSvcCfg(flags, name="TrackingVolumesSvc", **kwargs):
    result = ComponentAccumulator()
    trackingVolSvc = CompFactory.Trk.TrackingVolumesSvc(name=name, **kwargs)
    result.addService(trackingVolSvc, primary=True)
    return result
def MuonIdHelperSvcCfg(flags):
    acc = ComponentAccumulator()
    acc.merge(GeoModelCfg(flags))
    acc.addService( CompFactory.Muon.MuonIdHelperSvc("MuonIdHelperSvc",
        HasCSC=flags.Detector.GeometryCSC,
        HasSTGC=flags.Detector.GeometrysTGC,
        HasMM=flags.Detector.GeometryMM,
        HasMDT=flags.Detector.GeometryMDT,
        HasRPC=flags.Detector.GeometryRPC,
        HasTGC=flags.Detector.GeometryTGC), primary=True )
    return acc

@AccumulatorCache
def MuonGeoModelCfg(flags):
    result = ComponentAccumulator()
    result.merge(MuonIdHelperSvcCfg(flags)) 
    if flags.Muon.usePhaseIIGeoSetup:
        from MuonGeoModelR4.MuonGeoModelConfig import MuonGeoModelCfg as MuonGeoModelCfgR4
        result.merge(MuonGeoModelCfgR4(flags))
        if flags.Common.ProductionStep != ProductionStep.Simulation:
            from MuonGeometryCnv.MuonReadoutGeomCnvCfg import MuonReadoutGeometryCnvAlgCfg
            result.merge(MuonReadoutGeometryCnvAlgCfg(flags))
        return result
    
    result.merge(MuonGeoModelToolCfg(flags))
    result.merge(MuonDetectorCondAlgCfg(flags))
    return result


def MuonDetectorToolCfg(flags, name = "MuonDetectorTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("HasCSC", flags.Detector.GeometryCSC)
    kwargs.setdefault("HasSTgc", flags.Detector.GeometrysTGC)
    kwargs.setdefault("HasMM", flags.Detector.GeometryMM)

   
    kwargs.setdefault("UseConditionDb", flags.Muon.enableAlignment)
    # call fill cache of MuonDetectorTool such that all MdtReadoutElement caches are filled
    # already during initialize() -> this will increase memory -> needs to be measured
    kwargs.setdefault("FillCacheInitTime", flags.Common.ProductionStep != ProductionStep.Simulation or \
                                           flags.Muon.enableAlignment) 

    kwargs.setdefault("UseAsciiConditionData", flags.Muon.enableAlignment)
    # turn on/off caching of MdtReadoutElement surfaces
    kwargs.setdefault("CachingFlag", 1)
    
    UseIlinesFromGM = False
    EnableCscInternalAlignment = False

    if flags.Muon.enableAlignment:
        if not flags.Input.isMC:
            kwargs.setdefault("BEENoShift", True)
        # here define if I-lines (CSC internal alignment) are enabled
        if flags.Muon.Align.UseILines and flags.Detector.GeometryCSC:
            if 'HLT' in flags.IOVDb.GlobalTag:
                #logMuon.info("Reading CSC I-Lines from layout - special configuration for COMP200 in HLT setup.")
                UseIlinesFromGM = True
                EnableCscInternalAlignment = False
            else :
                #logMuon.info("Reading CSC I-Lines from conditions database.")
                if (flags.Common.isOnline and not flags.Input.isMC):
                    EnableCscInternalAlignment = True
                else:
                    UseIlinesFromGM = False
                    EnableCscInternalAlignment =  True
   
    kwargs.setdefault("UseIlinesFromGM", UseIlinesFromGM)
    kwargs.setdefault("EnableCscInternalAlignment", EnableCscInternalAlignment)
   
    if not flags.GeoModel.SQLiteDB:
        ## Additional material in the muon system
        AGDD2Geo = CompFactory.AGDDtoGeoSvc()
        muonAGDDTool = CompFactory.MuonAGDDTool("MuonSpectrometer", BuildNSW=False)
        AGDD2Geo.Builders += [ muonAGDDTool ]
        if (flags.Detector.GeometrysTGC and flags.Detector.GeometryMM):
            nswAGDDTool = CompFactory.NSWAGDDTool("NewSmallWheel", Locked=False)
            nswAGDDTool.Volumes = ["NewSmallWheel"]
            nswAGDDTool.DefaultDetector = "Muon"
            AGDD2Geo.Builders += [ nswAGDDTool ]

        #create=True is needed for the service to be initialised in the new style
        acc.addService(AGDD2Geo, create=True)


    acc.merge(MuonIdHelperSvcCfg(flags))
    
    detTool = CompFactory.MuonDetectorTool(name, **kwargs)
    acc.setPrivateTools(detTool)
    return acc


def MuonAlignmentCondAlgCfg(flags, name="MuonAlignmentCondAlg", **kwargs):
    acc = ComponentAccumulator()
    from IOVDbSvc.IOVDbSvcConfig import addFolders    
    if (flags.Common.isOnline and not flags.Input.isMC):
        acc.merge(addFolders( flags, ['/MUONALIGN/Onl/MDT/BARREL'], 'MUONALIGN', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/Onl/MDT/ENDCAP/SIDEA'], 'MUONALIGN', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/Onl/MDT/ENDCAP/SIDEC'], 'MUONALIGN', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/Onl/TGC/SIDEA'], 'MUONALIGN', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/Onl/TGC/SIDEC'], 'MUONALIGN', className='CondAttrListCollection'))
    else:
        acc.merge(addFolders( flags, ['/MUONALIGN/MDT/BARREL'], 'MUONALIGN_OFL', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/MDT/ENDCAP/SIDEA'], 'MUONALIGN_OFL', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/MDT/ENDCAP/SIDEC'], 'MUONALIGN_OFL', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/TGC/SIDEA'], 'MUONALIGN_OFL', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/TGC/SIDEC'], 'MUONALIGN_OFL', className='CondAttrListCollection'))


    ParlineFolders = ["/MUONALIGN/MDT/BARREL",
                      "/MUONALIGN/MDT/ENDCAP/SIDEA",
                      "/MUONALIGN/MDT/ENDCAP/SIDEC",
                      "/MUONALIGN/TGC/SIDEA",
                      "/MUONALIGN/TGC/SIDEC"]

    # here define if I-lines (CSC internal alignment) are enabled
    acc.merge(CscILineCondAlgCfg(flags))
    # here define if As-Built (MDT chamber alignment) are enabled
    if flags.Muon.Align.UseAsBuilt:
        if flags.IOVDb.DatabaseInstance == 'COMP200' or \
                'HLT' in flags.IOVDb.GlobalTag or flags.Common.isOnline :           
            pass
        else :            
            acc.merge(MdtAsBuiltCondAlgCfg(flags))
            acc.merge(NswAsBuiltCondAlgCfg(flags))
    
    kwargs.setdefault("ParlineFolders", ParlineFolders)
    MuonAlign = CompFactory.MuonAlignmentCondAlg(name, **kwargs)
    acc.addCondAlgo(MuonAlign, primary = True)
    return acc


def MuonAlignmentErrorDbAlgCfg(flags):
    acc = ComponentAccumulator()
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    acc.merge(addFolders(flags, "/MUONALIGN/ERRS", "MUONALIGN_OFL", className="CondAttrListCollection"))
    acc.addCondAlgo(CompFactory.MuonAlignmentErrorDbAlg("MuonAlignmentErrorDbAlg"))
    return acc

def NswAsBuiltCondAlgCfg(flags, name = "NswAsBuiltCondAlg", **kwargs):
    result = ComponentAccumulator()
    #### Do not apply the as-built correction if not activated
    if flags.GeoModel.Run < LHCPeriod.Run3:
        return result
    kwargs.setdefault("MicroMegaJSON","")
    kwargs.setdefault("sTgcJSON","")

    kwargs.setdefault("ReadMmAsBuiltParamsKey","/MUONALIGN/ASBUILTPARAMS/MM")
    #kwargs.setdefault("ReadSTgcAsBuiltParamsKey","/MUONALIGN/ASBUILTPARAMS/STGC") # This is the folder that sould be used once the as builts are validated, so keep it here but commented out
    kwargs.setdefault("ReadSTgcAsBuiltParamsKey","") # for now diable the reading of sTGC as build from the conditions database
    
    ##TODO: remove hard-coded tag once the global tag is ready
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if(not (kwargs["MicroMegaJSON"] or not kwargs["ReadMmAsBuiltParamsKey"]) ) : # no need to add the folder if we are reading a json file anyhow
        result.merge(addFolders( flags, kwargs["ReadMmAsBuiltParamsKey"]  , 'MUONALIGN_OFL', className='CondAttrListCollection', tag='MuonAlignAsBuiltParamsMm-RUN3-01-00'))
    ### Disable the STGC as-built parameters (Keep the path if we want to add later fully validated As-built)
    if(not (kwargs["sTgcJSON"] or not kwargs["ReadSTgcAsBuiltParamsKey"])): # no need to add the folder if we are reading a json file anyhow
        result.merge(addFolders( flags, kwargs["ReadSTgcAsBuiltParamsKey"], 'MUONALIGN_OFL', className='CondAttrListCollection', tag='MUONALIGN_STG_ASBUILT-001-03'))
    the_alg = CompFactory.NswAsBuiltCondAlg(name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)     
    return result

def MdtAsBuiltCondAlgCfg(flags, name="MdtAsBuiltCondAlg", **kwargs):
    result = ComponentAccumulator()
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if "readFromJSON" not in kwargs or not kwargs["readFromJSON"]:
        result.merge(addFolders( flags, '/MUONALIGN/MDT/ASBUILTPARAMS' , 'MUONALIGN_OFL', className='CondAttrListCollection'))
    the_alg = CompFactory.MdtAsBuiltCondAlg(name = name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)    
    return result

def CscILineCondAlgCfg(flags, name="CscILinesCondAlg", **kwargs):
    result = ComponentAccumulator()
    if not flags.Muon.Align.UseILines or not flags.Detector.GeometryCSC or 'HLT' in flags.IOVDb.GlobalTag: 
        return result
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if (flags.Common.isOnline and not flags.Input.isMC):
        result.merge(addFolders( flags, ['/MUONALIGN/Onl/CSC/ILINES'], 'MUONALIGN', className='CondAttrListCollection'))
    else:
        result.merge(addFolders( flags, ['/MUONALIGN/CSC/ILINES'], 'MUONALIGN_OFL', className='CondAttrListCollection'))
    
    the_alg = CompFactory.CscILinesCondAlg(name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result


@AccumulatorCache
def MuonDetectorCondAlgCfg(flags, name = "MuonDetectorCondAlg", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("MuonDetectorTool", result.popToolsAndMerge(MuonDetectorToolCfg(flags)))
    kwargs.setdefault("applyMmPassivation", flags.Muon.applyMMPassivation)

    if kwargs["applyMmPassivation"]:
        from MuonConfig.MuonCondAlgConfig import NswPassivationDbAlgCfg
        result.merge(NswPassivationDbAlgCfg(flags))
    if flags.Muon.enableAlignment:
        result.merge(MuonAlignmentCondAlgCfg(flags))
    kwargs.setdefault("applyALines", len([alg for alg in result.getCondAlgos() if alg.name == "MuonAlignmentCondAlg"])>0)
    kwargs.setdefault("applyBLines", len([alg for alg in result.getCondAlgos() if alg.name == "MuonAlignmentCondAlg"])>0)
    kwargs.setdefault("applyILines", len([alg for alg in result.getCondAlgos() if alg.name == "CscILinesCondAlg"])>0)
    kwargs.setdefault("applyNswAsBuilt", len([alg for alg in result.getCondAlgos() if alg.name == "NswAsBuiltCondAlg"])>0)
    kwargs.setdefault("applyMdtAsBuilt", len([alg for alg in result.getCondAlgos() if alg.name == "MdtAsBuiltCondAlg"])>0)

   
    
    MuonDetectorManagerCond = CompFactory.MuonDetectorCondAlg(name, **kwargs)

    result.addCondAlgo(MuonDetectorManagerCond, primary = True)
    return result


def MuonGeoModelToolCfg(flags):
    result = ComponentAccumulator()
    geoModelSvc = result.getPrimaryAndMerge(GeoModelCfg(flags))
    geoModelSvc.DetectorTools+= [result.popToolsAndMerge(MuonDetectorToolCfg(flags, FillCacheInitTime = 0))]
    return result
