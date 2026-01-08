# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

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
        from MuonGeoModelR4.MuonGeoModelConfig import MuonAlignStoreCfg
        result.merge(MuonAlignStoreCfg(flags))
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
    kwargs.setdefault("UseAsciiConditionData", flags.Muon.enableAlignment)
    
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

    # here define if I-lines (CSC internal alignment) are enabled
    acc.merge(CscILineCondAlgCfg(flags))
    # here define if As-Built (MDT chamber alignment) are enabled
    if flags.Muon.Align.UseAsBuilt:
        acc.merge(MdtAsBuiltCondAlgCfg(flags))
        acc.merge(NswAsBuiltCondAlgCfg(flags))
        if(flags.Muon.Align.UsesTGCAsBuild):
            acc.merge(sTGCAsBuiltCondAlgCfg(flags))

    if not flags.Muon.Align.UseALines and not flags.Muon.Align.UseBLines:
        return acc
    from IOVDbSvc.IOVDbSvcConfig import addFolders    
    
    onl = "/Onl" if ((flags.Common.isOnline or flags.IOVDb.GlobalTag.startswith("CONDBR2-HLTP")) and not flags.Input.isMC) else ""
    ParlineFolders = [f"/MUONALIGN{onl}/MDT/BARREL", 
                      f"/MUONALIGN{onl}/MDT/ENDCAP/SIDEA",
                      f"/MUONALIGN{onl}/MDT/ENDCAP/SIDEC", 
                      f"/MUONALIGN{onl}/TGC/SIDEA",
                      f"/MUONALIGN{onl}/TGC/SIDEC"]
    acc.merge(addFolders( flags, ParlineFolders, 'MUONALIGN' if len(onl) else 'MUONALIGN_OFL',  className='CondAttrListCollection'))
 
    if len(onl):
        ParlineFolders = [ x[ :x.find(onl)] + x[x.find(onl) + len(onl): ] for x in ParlineFolders]

    kwargs.setdefault("LoadALines", flags.Muon.Align.UseALines)
    kwargs.setdefault("LoadBLines", flags.Muon.Align.UseBLines)
    
    kwargs.setdefault("ParlineFolders", ParlineFolders)
    MuonAlign = CompFactory.Muon.AlignmentCondAlg(name, **kwargs)
    acc.addCondAlgo(MuonAlign, primary = True)
    return acc


def MuonAlignmentErrorDbAlgCfg(flags):
    acc = ComponentAccumulator()
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    acc.merge(addFolders(flags, "/MUONALIGN/ERRS", "MUONALIGN_OFL", className="CondAttrListCollection"))
    acc.addCondAlgo(CompFactory.Muon.AlignmentErrorDbAlg("MuonAlignmentErrorDbAlg"))
    return acc

def NswAsBuiltCondAlgCfg(flags, name = "NswAsBuiltCondAlg", **kwargs):
    result = ComponentAccumulator()
    #### Do not apply the as-built correction if not activated
    if flags.GeoModel.Run < LHCPeriod.Run3:
        return result
    kwargs.setdefault("MicroMegaJSON","")

    kwargs.setdefault("ReadMmAsBuiltParamsKey","/MUONALIGN/ASBUILTPARAMS/MM")

    ##TODO: remove hard-coded tag once the global tag is ready
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if(not (kwargs["MicroMegaJSON"] or not kwargs["ReadMmAsBuiltParamsKey"]) ) : # no need to add the folder if we are reading a json file anyhow
        result.merge(addFolders( flags, kwargs["ReadMmAsBuiltParamsKey"]  , 'MUONALIGN_OFL', className='CondAttrListCollection'))
    the_alg = CompFactory.Muon.NswAsBuiltCondAlg(name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)     
    return result

def sTGCAsBuiltCondAlgCfg(flags, name = "sTGCAsBuiltCondAlg", **kwargs):
    result = ComponentAccumulator()
    #### Do not apply the as-built correction if not activated
    if flags.GeoModel.Run < LHCPeriod.Run3 or not flags.Muon.Align.UsesTGCAsBuild:
        return result
    kwargs.setdefault("readFromJSON","")
    if not kwargs["readFromJSON"]:
        kwargs.setdefault("ReadKey","/MUONALIGN/ASBUILTPARAMS/STGC")
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        result.merge(addFolders( flags, kwargs["ReadKey"], 'MUONALIGN_OFL', className='CondAttrListCollection', tag = 'MUONALIGN_STG_IntAl_alCons_noQL3_v01'))
    the_alg = CompFactory.Muon.sTGCAsBuiltCondAlg(name,**kwargs)
    result.addCondAlgo(the_alg, primary=True)
    return result

# since the MM as built 2 corrections are not supossed to run in standart reco job just a fragment to activate them in a postExex is provided
def activateMmAsBuilt2PostExec(flags, cfg, **kwargs):
    #### Do not apply the as-built correction if not activated
    name = "MmAsBuilt2CondAlg"
    if flags.GeoModel.Run != LHCPeriod.Run3:
        return
    kwargs.setdefault("readFromJSON","")
    kwargs.setdefault("WriteKey","MmAsBuilt2")
    the_alg = CompFactory.Muon.sTGCAsBuiltCondAlg(name,**kwargs)
    cfg.addCondAlgo(the_alg, primary=True)

    cfg.getCondAlgo("MuonDetectorCondAlg").applyMmAsBuilt2=True
    cfg.getCondAlgo("MuonDetectorCondAlg").ReadMmAsBuilt2Key = kwargs["WriteKey"] 
        
        


def MdtAsBuiltCondAlgCfg(flags, name="MdtAsBuiltCondAlg", **kwargs):
    result = ComponentAccumulator()
    if not flags.Detector.GeometryMDT:
        return result
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if "readFromJSON" not in kwargs or not kwargs["readFromJSON"]:
        result.merge(addFolders( flags, '/MUONALIGN/MDT/ASBUILTPARAMS' , 'MUONALIGN_OFL', className='CondAttrListCollection'))
    the_alg = CompFactory.Muon.MdtAsBuiltCondAlg(name = name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)    
    return result

def CscILineCondAlgCfg(flags, name="CscILinesCondAlg", **kwargs):
    result = ComponentAccumulator()
    if not flags.Muon.Align.UseILines: 
        return result
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if (flags.Common.isOnline and not flags.Input.isMC):
        result.merge(addFolders( flags, ['/MUONALIGN/Onl/CSC/ILINES'], 'MUONALIGN', className='CondAttrListCollection'))
    else:
        result.merge(addFolders( flags, ['/MUONALIGN/CSC/ILINES'], 'MUONALIGN_OFL', className='CondAttrListCollection'))
    
    the_alg = CompFactory.Muon.CscILinesCondAlg(name, **kwargs)
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
    kwargs.setdefault("applyALines", flags.Muon.Align.UseALines)
    kwargs.setdefault("applyBLines", flags.Muon.Align.UseBLines)
    kwargs.setdefault("applyILines", flags.Muon.Align.UseILines)
    kwargs.setdefault("applyNswAsBuilt", len([alg for alg in result.getCondAlgos() if alg.name == "NswAsBuiltCondAlg"])>0)
    kwargs.setdefault("applysTGCAsBuilt", len([alg for alg in result.getCondAlgos() if alg.name == "sTGCAsBuiltCondAlg"])>0)
    kwargs.setdefault("applyMdtAsBuilt", len([alg for alg in result.getCondAlgos() if alg.name == "MdtAsBuiltCondAlg"])>0)

   
    
    MuonDetectorManagerCond = CompFactory.MuonDetectorCondAlg(name, **kwargs)

    result.addCondAlgo(MuonDetectorManagerCond, primary = True)
    return result


def MuonGeoModelToolCfg(flags):
    result = ComponentAccumulator()
    geoModelSvc = result.getPrimaryAndMerge(GeoModelCfg(flags))
    geoModelSvc.DetectorTools+= [result.popToolsAndMerge(MuonDetectorToolCfg(flags))]
    return result
