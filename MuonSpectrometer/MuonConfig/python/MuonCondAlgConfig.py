# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

## Configuration Access to OFFLINE DB (COMP200)

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFolders
from AthenaConfiguration.Enums import LHCPeriod


@AccumulatorCache
def MdtCondDbAlgCfg(flags, **kwargs):
    result  = ComponentAccumulator()
    folders = []
    if not flags.Muon.useMdtDcsData:
        return result ## avoid adding algo to the component accumulator
    else:
        kwargs["isOnline"] = False
        if flags.Input.isMC:
            kwargs['isData'] = False
            folders          = ["/MDT/DCS/DROPPEDCH", "/MDT/DCS/PSLVCHSTATE"]

            # TODO: probably will be used in the future but disable for now
            kwargs['ReadKey_MC_DE'] = ''
            kwargs['ReadKey_MC_DT'] = ''

            # disable the rest
            kwargs['ReadKey_DataR1_DC'] = ''
            kwargs['ReadKey_DataR1_HV'] = ''
            kwargs['ReadKey_DataR1_LV'] = ''
            kwargs['ReadKey_DataR1_V0'] = ''
            kwargs['ReadKey_DataR1_V1'] = ''
            kwargs['ReadKey_DataR2_HV'] = ''
            kwargs['ReadKey_DataR2_LV'] = ''
        else:
            kwargs['isData'] = True
            kwargs['isRun1'] = flags.IOVDb.DatabaseInstance == 'COMP200'
            kwargs['useRun1SetPoints'] = False
            if kwargs['isRun1'] and kwargs['useRun1SetPoints']:
                folders = ["/MDT/DCS/PSV0SETPOINTS", "/MDT/DCS/PSV1SETPOINTS"]

                # disable the rest
                kwargs['ReadKey_DataR1_DC'] = ''
                kwargs['ReadKey_DataR1_HV'] = ''
                kwargs['ReadKey_DataR1_LV'] = ''
                kwargs['ReadKey_DataR2_HV'] = ''
                kwargs['ReadKey_DataR2_LV'] = ''
            if kwargs['isRun1']:
                folders = ["/MDT/DCS/PSHVMLSTATE", "/MDT/DCS/PSLVCHSTATE", "/MDT/DCS/DROPPEDCH"]

                # disable the rest
                kwargs['ReadKey_DataR1_V0'] = ''
                kwargs['ReadKey_DataR1_V1'] = ''
                kwargs['ReadKey_DataR2_HV'] = ''
                kwargs['ReadKey_DataR2_LV'] = ''
            else:
                folders = ["/MDT/DCS/HV", "/MDT/DCS/LV"]

                # disable the rest
                kwargs['ReadKey_DataR1_DC'] = ''
                kwargs['ReadKey_DataR1_HV'] = ''
                kwargs['ReadKey_DataR1_LV'] = ''
                kwargs['ReadKey_DataR1_V0'] = ''
                kwargs['ReadKey_DataR1_V1'] = ''

            # disable MC folders
            kwargs['ReadKey_MC_DC'] = ''
            kwargs['ReadKey_MC_DE'] = ''
            kwargs['ReadKey_MC_DT'] = ''
            kwargs['ReadKey_MC_NC'] = ''
    
    alg = CompFactory.MdtCondDbAlg(**kwargs)
    result.merge( addFolders(flags, folders , detDb="DCS_OFL", className='CondAttrListCollection') )
    result.addCondAlgo(alg, primary = True)
    return result

def RpcCondDbAlgCfg(flags, **kwargs):
    result  = ComponentAccumulator()   
    if flags.Common.isOnline:
        return result ## avoid adding algo to the component accumulator

    kwargs['isData'] = not flags.Input.isMC
    alg = CompFactory.RpcCondDbAlg(**kwargs)

    result.merge( addFolders(flags, ["/RPC/DQMF/ELEMENT_STATUS"], detDb="RPC_OFL", className='CondAttrListCollection') )
    result.addCondAlgo(alg)
    return result

def CscCondDbAlgCfg(flags, **kwargs):
    result  = ComponentAccumulator()
    pslope_from_db = False
    folders = ["/CSC/FTHOLD", "/CSC/NOISE", "/CSC/PED", "/CSC/RMS", "/CSC/STAT", "/CSC/T0BASE", "/CSC/T0PHASE"]
    scheme  = "CSC_OFL"
    kwargs['ReadKey_HV'] = '' # Never used at present 
    if flags.Common.isOnline:
        kwargs["isOnline"  ] = True
        kwargs['isData'    ] = True
        kwargs['ReadKey_FT'] = '/CSC/FTHOLD' # 'ConditionsStore+' prefix not necessarily needed in ReadKey
        kwargs['ReadKey_NO'] = '/CSC/NOISE'
        kwargs['ReadKey_PD'] = '/CSC/PED'
        if pslope_from_db:
            kwargs['ReadKey_PS'] = '/CSC/PSLOPE'
        kwargs['ReadKey_RM'] = '/CSC/RMS'
        kwargs['ReadKey_ST'] = '/CSC/STAT'
        kwargs['ReadKey_TB'] = ''
        kwargs['ReadKey_TP'] = ''
        folders = ["/CSC/ONL/FTHOLD", "/CSC/ONL/NOISE", "/CSC/ONL/PED", "/CSC/ONL/RMS", "/CSC/ONL/STAT"]
        if pslope_from_db:
            folders.append("/CSC/PSLOPE")
        scheme  = "CSC_ONL"
    else:
        if pslope_from_db:
            folders.append("/CSC/PSLOPE")

        kwargs["isOnline"] = False
        if flags.Input.isMC:
            kwargs['isData'] = False
        else:
            kwargs['isData'] = True
            kwargs['isRun1'] = flags.IOVDb.DatabaseInstance == 'COMP200'
    alg = CompFactory.CscCondDbAlg(**kwargs)
    result.merge( addFolders(flags, folders , detDb=scheme, className='CondAttrListCollection') )
    result.addCondAlgo(alg)
    return result

def TgcCondDbAlgCfg(flags,name="TgcCondDbAlg", **kwargs):
    result  = ComponentAccumulator()
    ### TO DO define the COOL folder
    the_alg = CompFactory.TgcCondDbAlg(name = name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result

def TgcEnergyThresholdCondAlgCfg(flags, name = "TgcEnergyThresholdCondAlg", **kwargs):
    result = ComponentAccumulator()
    ### TO DO define the COOL folder
    the_alg = CompFactory.TgcDigitEnergyThreshCondAlg(name= name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result

def TgcDigitJitterCondAlgCfg(flags, name = "TgcDigitJitterCondAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.TgcDigitJitterCondAlg(name = name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result

def TgcDigitASDposCondAlgCfg(flags, name="TgcDigitASDposCondAlg", **kwargs):
    result = ComponentAccumulator()
    result.merge(addFolders(flags, ["/TGC/DIGIT/ASDPOS"], detDb="TGC_OFL", db="OFLP200", className="CondAttrListCollection"))
    the_alg = CompFactory.TgcDigitASDposCondAlg(name = name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result

def TgcDigitTimeOffsetCondAlgCfg(flags, name = "TgcDigitTimeOffsetCondAlg", **kwargs):
    result = ComponentAccumulator()
    result.merge(addFolders(flags, ["/TGC/DIGIT/TOFFSET"], detDb="TGC_OFL", db="OFLP200", className="CondAttrListCollection"))
    the_alg = CompFactory.TgcDigitTimeOffsetCondAlg(name = name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result

def TgcDigitCrosstalkCondAlgCfg(flags, name = "TgcDigitCrosstalkCondAlg", **kwargs):
    result = ComponentAccumulator()
    result.merge(addFolders(flags, ["/TGC/DIGIT/XTALK"], detDb="TGC_OFL", db="OFLP200", className="CondAttrListCollection"))
    the_alg = CompFactory.TgcDigitCrosstalkCondAlg(name = name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)
    return result

def TgcDigitCondAlgCfg(flags):
    result  = ComponentAccumulator()
    result.merge(TgcDigitASDposCondAlgCfg(flags))    
    result.merge(TgcDigitTimeOffsetCondAlgCfg(flags))
    result.merge(TgcDigitCrosstalkCondAlgCfg(flags))
    return result
    result.merge(TgcCondDbAlgCfg(flags))
    result.merge(TgcEnergyThresholdCondAlgCfg(flags))
    result.merge(TgcDigitJitterCondAlgCfg(flags))
    return result


@AccumulatorCache
def NswCalibDbAlgCfg(flags, **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("MmT0FileName","")
    kwargs.setdefault("sTgcT0FileName","")

    kwargs.setdefault("loadMmT0Data",flags.Muon.Calib.applyMmT0Correction)
    kwargs.setdefault("loadsTgcT0Data", flags.Muon.Calib.applysTgcT0Correction)
    if(kwargs["loadMmT0Data"] and not kwargs['MmT0FileName'] ):
        kwargs.setdefault('ReadKey_MM_T0', "/MDT/MM/T0")
    kwargs.setdefault('ReadKey_MM_T0', "")
    
    if(kwargs["loadsTgcT0Data"] and not kwargs['sTgcT0FileName']):
        kwargs['ReadKey_STGC_T0'] = "/TGC/NSW/T0"
    kwargs.setdefault('ReadKey_STGC_T0', "")


    ## online scenario
    if flags.Common.isOnline:
        kwargs['isData'] = True

        ## MM folders
        scheme  = "MDT_ONL"
        
        # use specific folder tags for Run 4
        if flags.GeoModel.Run>=LHCPeriod.Run4:
            result.merge( addFolders(flags, ["/MDT/Onl/MM/TIME/SIDEA"  ], detDb=scheme, className='CondAttrListCollection' , tag="MmTdoSideA-Const-3p73") )
            result.merge( addFolders(flags, ["/MDT/Onl/MM/TIME/SIDEC"  ], detDb=scheme, className='CondAttrListCollection' , tag="MmTdoSideC-Const-3p73") )
            result.merge( addFolders(flags, ["/MDT/Onl/MM/CHARGE/SIDEA"], detDb=scheme, className='CondAttrListCollection' , tag="MmPdoSideA-Const-9p0" ) )
            result.merge( addFolders(flags, ["/MDT/Onl/MM/CHARGE/SIDEC"], detDb=scheme, className='CondAttrListCollection' , tag="MmPdoSideC-Const-9p0" ) )
            if(kwargs['ReadKey_MM_T0']):
                result.merge(addFolders(flags, [kwargs['ReadKey_MM_T0']], detDb=scheme, className='CondAttrListCollection' , tag="MmT0SideAc-Nov2023"))
        else:
            folders = ["/MDT/Onl/MM/TIME/SIDEA", "/MDT/Onl/MM/CHARGE/SIDEA", \
                       "/MDT/Onl/MM/TIME/SIDEC", "/MDT/Onl/MM/CHARGE/SIDEC"]
            result.merge( addFolders(flags, folders, detDb=scheme, className='CondAttrListCollection') )
            if(kwargs['ReadKey_MM_T0']):
                result.merge(addFolders(flags, [kwargs['ReadKey_MM_T0']], detDb=scheme, className='CondAttrListCollection' , tag="MmT0SideAc-Nov2023"))

        kwargs["ReadKey_MM_SIDEA_TDO"] = "/MDT/Onl/MM/TIME/SIDEA"
        kwargs["ReadKey_MM_SIDEC_TDO"] = "/MDT/Onl/MM/TIME/SIDEC"
        kwargs["ReadKey_MM_SIDEA_PDO"] = "/MDT/Onl/MM/CHARGE/SIDEA"
        kwargs["ReadKey_MM_SIDEC_PDO"] = "/MDT/Onl/MM/CHARGE/SIDEC"
        


        ## sTGC folders
        scheme  = "TGC_ONL"

        # use specific folder tags for Run 4
        if flags.GeoModel.Run>=LHCPeriod.Run4:
            result.merge( addFolders(flags, [ "/TGC/Onl/NSW/TIME/SIDEA"  ], detDb=scheme, className='CondAttrListCollection' , tag="sTgcTdoSideA-Const-3p73"))
            result.merge( addFolders(flags, [ "/TGC/Onl/NSW/TIME/SIDEC"  ], detDb=scheme, className='CondAttrListCollection' , tag="sTgcTdoSideC-Const-3p73"))
            result.merge( addFolders(flags, [ "/TGC/Onl/NSW/CHARGE/SIDEA"], detDb=scheme, className='CondAttrListCollection' , tag="sTgcPdoSideA-Const-0p78-icpt0"))
            result.merge( addFolders(flags, [ "/TGC/Onl/NSW/CHARGE/SIDEC"], detDb=scheme, className='CondAttrListCollection' , tag="sTgcPdoSideC-Const-0p78-icpt0"))
        else:
            folders = ["/TGC/Onl/NSW/TIME/SIDEA", "/TGC/Onl/NSW/CHARGE/SIDEA", \
                       "/TGC/Onl/NSW/TIME/SIDEC", "/TGC/Onl/NSW/CHARGE/SIDEC"]
            result.merge( addFolders(flags, folders , detDb=scheme, className='CondAttrListCollection') )

        kwargs["ReadKey_STGC_SIDEA_TDO"] = "/TGC/Onl/NSW/TIME/SIDEA"
        kwargs["ReadKey_STGC_SIDEC_TDO"] = "/TGC/Onl/NSW/TIME/SIDEC"
        kwargs["ReadKey_STGC_SIDEA_PDO"] = "/TGC/Onl/NSW/CHARGE/SIDEA"
        kwargs["ReadKey_STGC_SIDEC_PDO"] = "/TGC/Onl/NSW/CHARGE/SIDEC"
        

    elif flags.Input.isMC:
        kwargs['isData'] = False
        kwargs.setdefault('processThresholds', False)

        ## MM folders
        scheme  = "MDT_OFL"
        
        # use specific folder tags for Run 4
        if flags.GeoModel.Run>=LHCPeriod.Run4:
            result.merge( addFolders(flags, ["/MDT/MM/TIME/SIDEA"  ], detDb=scheme, className='CondAttrListCollection' , tag="MmTdoSideA-Const-3p73") )
            result.merge( addFolders(flags, ["/MDT/MM/TIME/SIDEC"  ], detDb=scheme, className='CondAttrListCollection' , tag="MmTdoSideC-Const-3p73") )
            result.merge( addFolders(flags, ["/MDT/MM/CHARGE/SIDEA"], detDb=scheme, className='CondAttrListCollection' , tag="MmPdoSideA-Const-9p0" ) )
            result.merge( addFolders(flags, ["/MDT/MM/CHARGE/SIDEC"], detDb=scheme, className='CondAttrListCollection' , tag="MmPdoSideC-Const-9p0" ) )
            if kwargs['processThresholds']:
                result.merge( addFolders(flags, ["/MDT/MM/THR/SIDEA"   ], detDb=scheme, className='CondAttrListCollection' , tag="MmThrSideA-Const-55p4") )
                result.merge( addFolders(flags, ["/MDT/MM/THR/SIDEC"   ], detDb=scheme, className='CondAttrListCollection' , tag="MmThrSideC-Const-55p4") )
            if(kwargs['ReadKey_MM_T0']):
                result.merge(addFolders(flags, [kwargs['ReadKey_MM_T0']], detDb=scheme, className='CondAttrListCollection' , tag="MmT0SideAc-Nov2023"))
        else:
            folders = ["/MDT/MM/TIME/SIDEA" , "/MDT/MM/CHARGE/SIDEA" , \
                       "/MDT/MM/TIME/SIDEC" , "/MDT/MM/CHARGE/SIDEC" ]
            if kwargs['processThresholds']:
                folders.append("/MDT/MM/THR/SIDEA")
                folders.append("/MDT/MM/THR/SIDEC")
            result.merge( addFolders(flags, folders, detDb=scheme, className='CondAttrListCollection') )
            if(kwargs['ReadKey_MM_T0']):
                result.merge(addFolders(flags, [kwargs['ReadKey_MM_T0']], detDb=scheme, className='CondAttrListCollection' , tag="MmT0SideAc-Nov2023"))

        ## sTGC folders
        scheme  = "TGC_OFL"

        # use specific folder tags for Run 4
        if flags.GeoModel.Run>=LHCPeriod.Run4:
            result.merge( addFolders(flags, [ "/TGC/NSW/TIME/SIDEA"  ], detDb=scheme, className='CondAttrListCollection' , tag="sTgcTdoSideA-Const-3p73"))
            result.merge( addFolders(flags, [ "/TGC/NSW/TIME/SIDEC"  ], detDb=scheme, className='CondAttrListCollection' , tag="sTgcTdoSideC-Const-3p73"))
            result.merge( addFolders(flags, [ "/TGC/NSW/CHARGE/SIDEA"], detDb=scheme, className='CondAttrListCollection' , tag="sTgcPdoSideA-Const-0p78-icpt0"))
            result.merge( addFolders(flags, [ "/TGC/NSW/CHARGE/SIDEC"], detDb=scheme, className='CondAttrListCollection' , tag="sTgcPdoSideC-Const-0p78-icpt0"))
            if kwargs['processThresholds']:
                result.merge( addFolders(flags, [ "/TGC/NSW/THR/SIDEA"   ], detDb=scheme, className='CondAttrListCollection' , tag="sTgcThrSideA-Const-15p0"))
                result.merge( addFolders(flags, [ "/TGC/NSW/THR/SIDEC"   ], detDb=scheme, className='CondAttrListCollection' , tag="sTgcThrSideC-Const-15p0"))
        else:
            folders = ["/TGC/NSW/TIME/SIDEA", "/TGC/NSW/CHARGE/SIDEA", \
                       "/TGC/NSW/TIME/SIDEC", "/TGC/NSW/CHARGE/SIDEC"]
            if kwargs['processThresholds']:
                folders.append("/TGC/NSW/THR/SIDEA")
                folders.append("/TGC/NSW/THR/SIDEC")
            result.merge( addFolders(flags, folders , detDb=scheme, className='CondAttrListCollection') )

    ## offline
    else:
        kwargs['isData'] = True

        ## MM folders
        scheme  = "MDT_OFL"
        
        # use specific folder tags for Run 4
        if flags.GeoModel.Run>=LHCPeriod.Run4:
            result.merge( addFolders(flags, ["/MDT/MM/TIME/SIDEA"  ], detDb=scheme, className='CondAttrListCollection' , tag="MmTdoSideA-Const-3p73") )
            result.merge( addFolders(flags, ["/MDT/MM/TIME/SIDEC"  ], detDb=scheme, className='CondAttrListCollection' , tag="MmTdoSideC-Const-3p73") )
            result.merge( addFolders(flags, ["/MDT/MM/CHARGE/SIDEA"], detDb=scheme, className='CondAttrListCollection' , tag="MmPdoSideA-Const-9p0" ) )
            result.merge( addFolders(flags, ["/MDT/MM/CHARGE/SIDEC"], detDb=scheme, className='CondAttrListCollection' , tag="MmPdoSideC-Const-9p0" ) )
            if(kwargs['ReadKey_MM_T0']):
                result.merge(addFolders(flags, [kwargs['ReadKey_MM_T0']], detDb=scheme, className='CondAttrListCollection' , tag="MmT0SideAc-Nov2023"))
        else:
            folders = ["/MDT/MM/TIME/SIDEA", "/MDT/MM/CHARGE/SIDEA", \
                       "/MDT/MM/TIME/SIDEC", "/MDT/MM/CHARGE/SIDEC"]
            result.merge( addFolders(flags, folders, detDb=scheme, className='CondAttrListCollection') ) 
            if(kwargs['ReadKey_MM_T0']):
                result.merge(addFolders(flags, [kwargs['ReadKey_MM_T0']], detDb=scheme, className='CondAttrListCollection' , tag="MmT0SideAC-Feb2025-withTDOCalib-v1"))
            
       
        ## sTGC folders
        scheme  = "TGC_OFL"

        # use specific folder tags for Run 4
        if flags.GeoModel.Run>=LHCPeriod.Run4:
            result.merge( addFolders(flags, [ "/TGC/NSW/TIME/SIDEA"  ], detDb=scheme, className='CondAttrListCollection' , tag="sTgcTdoSideA-Const-3p73"))
            result.merge( addFolders(flags, [ "/TGC/NSW/TIME/SIDEC"  ], detDb=scheme, className='CondAttrListCollection' , tag="sTgcTdoSideC-Const-3p73"))
            result.merge( addFolders(flags, [ "/TGC/NSW/CHARGE/SIDEA"], detDb=scheme, className='CondAttrListCollection' , tag="sTgcPdoSideA-Const-0p78-icpt0"))
            result.merge( addFolders(flags, [ "/TGC/NSW/CHARGE/SIDEC"], detDb=scheme, className='CondAttrListCollection' , tag="sTgcPdoSideC-Const-0p78-icpt0"))
        else:
            folders = ["/TGC/NSW/TIME/SIDEA", "/TGC/NSW/CHARGE/SIDEA", \
                       "/TGC/NSW/TIME/SIDEC", "/TGC/NSW/CHARGE/SIDEC"]
            result.merge( addFolders(flags, folders , detDb=scheme, className='CondAttrListCollection') )

            if kwargs["ReadKey_STGC_T0"]:
                result.merge(addFolders(flags, [kwargs["ReadKey_STGC_T0"]], detDb=scheme, className='CondAttrListCollection'))
    
    result.addCondAlgo(CompFactory.NswCalibDbAlg(**kwargs))
    return result

def NswPassivationDbAlgCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    # use specific folder tags for Run 4
    if flags.GeoModel.Run>=LHCPeriod.Run4:
        acc.merge(addFolders(flags, "/MDT/MM/PASSIVATION", "MDT_OFL", className="CondAttrListCollection", tag="MmPassiv2022May19"))
    else:
        acc.merge(addFolders(flags, "/MDT/MM/PASSIVATION", "MDT_OFL", className="CondAttrListCollection"))
    alg = CompFactory.NswPassivationDbAlg("NswPassivationDbAlg", **kwargs)
    acc.addCondAlgo(alg)
    return acc

def MuonStationIntersectCondAlgCfg(flags, name='MuonStationIntersectCondAlg',**kwargs):
    # Has dependency IdHelperTool (which we ignore for now)
    result = ComponentAccumulator()
    result.merge(MdtCondDbAlgCfg(flags))
    if not flags.Muon.useMdtDcsData: kwargs.setdefault("MdtCondKey","")
    muon_station_intersect_condalg = CompFactory.MuonStationIntersectCondAlg(name=name, **kwargs)
    result.addCondAlgo(muon_station_intersect_condalg, primary=True)
    return result

def NswDcsDbAlgCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    if flags.GeoModel.Run!=LHCPeriod.Run3: return acc
    if flags.Input.isMC: return acc
    kwargs.setdefault("LoadTdaq", False)
    kwargs.setdefault("LoadEltx", False) 
    acc.merge(addFolders(flags, "/MMG/DCS/HV", "DCS_OFL", className="CondAttrListCollection"))
    acc.merge(addFolders(flags, "/STG/DCS/HV", "DCS_OFL", className="CondAttrListCollection"))
    
    if(kwargs["LoadTdaq"]):
        kwargs.setdefault("ReadKey_MMG_TDAQ","/MDT/MM/ELinks")
        kwargs.setdefault("ReadKey_STG_TDAQ","/TGC/NSW/ELinks")
        acc.merge(addFolders(flags, "/MDT/MM/ELinks", detDb="MDT_OFL", className="CondAttrListCollection", tag="MmElinks2023-TEST-01"))
        acc.merge(addFolders(flags, "/TGC/NSW/ELinks", detDb="TGC_OFL", className="CondAttrListCollection", tag="sTgcElinks2023-TEST-01"))
    kwargs.setdefault("ReadKey_MMG_TDAQ","/MDT/MM/ELinks")
    kwargs.setdefault("ReadKey_STG_TDAQ","/TGC/NSW/Elinks")
    
    if(kwargs["LoadEltx"]):
        kwargs.setdefault("ReadKey_MMG_ELTX","/MMG/DCS/TSELTX")
        kwargs.setdefault("ReadKey_STG_ELTX","/STG/DCS/TSELTX")
        acc.merge(addFolders(flags, kwargs["ReadKey_MMG_ELTX"], "DCS_OFL", className="CondAttrListCollection"))
        acc.merge(addFolders(flags, kwargs["ReadKey_STG_ELTX"], "DCS_OFL", className="CondAttrListCollection"))
    kwargs.setdefault("ReadKey_MMG_ELTX","")
    kwargs.setdefault("ReadKey_STG_ELTX","")


    alg = CompFactory.NswDcsDbAlg("NswDcsDbAlg", **kwargs)
    acc.addCondAlgo(alg)
    return acc


def MmDigitEffiCondAlgCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    if not flags.Input.isMC: return acc

    if (kwargs["readFromJSON"]):
        kwargs["ReadKey"] = ""
    else:
        kwargs.setdefault("ReadKey","/MDT/MM/EFFMAP")
        acc.merge(addFolders(flags, kwargs["ReadKey"],"MDT_OFL", className="CondAttrListCollection", tag="MMEffMap_2024DataTillEndOfJuly"))

    alg = CompFactory.MmDigitEffiCondAlg("MmDigitEffiCondAlg", **kwargs)
    acc.addCondAlgo(alg)
    return acc

def sTgcDigitEffiCondAlgCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    if not flags.Input.isMC: return acc

    if (kwargs["readFromJSON"]):
        kwargs["ReadKey"] = ""
    else:
        kwargs.setdefault("ReadKey","/TGC/NSW/EFFMAP")
        acc.merge(addFolders(flags, kwargs["ReadKey"],"TGC_OFL", className="CondAttrListCollection",tag="sTGCEffMap_2024DataTillEndOfJuly"))

    alg = CompFactory.sTgcDigitEffiCondAlg("sTgcDigitEffiCondAlg", **kwargs)
    acc.addCondAlgo(alg)
    return acc
