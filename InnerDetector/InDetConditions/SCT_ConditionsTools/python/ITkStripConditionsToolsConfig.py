# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
from AtlasGeoModel.GeoModelConfig import GeoModelCfg
from IOVDbSvc.IOVDbSvcConfig import addFolders, addFoldersSplitOnline
from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg


def ITkStripByteStreamErrorsToolCfg(flags, name="ITkStripByteStreamErrorsTool", **kwargs):
    acc = ITkStripConfigurationConditionsToolCfg(flags)
    kwargs.setdefault("ConfigTool", acc.popPrivateTools())
    acc.setPrivateTools(CompFactory.SCT_ByteStreamErrorsTool(name, **kwargs))
    return acc


def ITkStripConditionsSummaryToolCfg(flags, name="ITkStripConditionsSummaryTool", **kwargs):
    acc = ComponentAccumulator()

    ConditionsTools = []
    if flags.ITk.doStripModuleVeto:
        ConditionsTools += [ acc.popToolsAndMerge(ITkStripModuleVetoCfg(flags)) ]

    kwargs.setdefault("ConditionsTools", ConditionsTools)
    kwargs.setdefault("SCTDetEleCollKey", "ITkStripDetectorElementCollection")
    acc.setPrivateTools(CompFactory.SCT_ConditionsSummaryTool(name, **kwargs))
    return acc


def ITkStripDetectorElementStatusAddByteStreamErrorsToolCfg(flags, name="ITkStripDetectorElementStatusAddByteStreamErrorsTool", **kwargs):
    acc = ComponentAccumulator()
    if not flags.Input.isMC  and flags.Input.Format is Format.BS :
        if "ConditionsTools" not in kwargs :
            kwargs.setdefault("ConditionsTools", [ acc.popToolsAndMerge(ITkStripByteStreamErrorsToolCfg(flags)) ])

    kwargs.setdefault("SCTDetEleCollKey", "ITkStripDetectorElementCollection")
    acc.setPrivateTools(CompFactory.SCT_ConditionsSummaryTool(name, **kwargs))
    return acc


def ITkStripConfigurationConditionsToolCfg(flags, name="ITkStripConfigurationConditionsTool", **kwargs):
    # Load conditions configuration service and load folders and algorithm for it
    # Load folders that have to exist for both MC and Data
    from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import getITkStripDAQConfigFolder, ITkStripConfigurationCondAlgCfg
    folder_path = getITkStripDAQConfigFolder(flags)

    cond_kwargs = {}
    cond_kwargs["ChannelFolder"] = folder_path + ("Chip" if flags.IOVDb.DatabaseInstance == "COMP200" else "ChipSlim")
    cond_kwargs["ModuleFolder"] = f"{folder_path}Module"
    cond_kwargs["MurFolder"] = f"{folder_path}MUR"
    cond_kwargs["dbInstance"] = "ITkStrip"
    cond_kwargs["SCT_ConfigurationCondAlgName"] = "ITkStripConfigurationCondAlg"

    acc = ITkStripReadoutGeometryCfg(flags)

    if "ChannelFolderDB" not in cond_kwargs:
        acc.merge(addFoldersSplitOnline(flags,
                                        detDb=cond_kwargs["dbInstance"],
                                        onlineFolders=cond_kwargs["ChannelFolder"],
                                        offlineFolders=cond_kwargs["ChannelFolder"],
                                        className="CondAttrListVec",
                                        splitMC=True))
    else:
        acc.merge(addFolders(flags, [cond_kwargs["ChannelFolderDB"]], detDb=cond_kwargs["dbInstance"], className="CondAttrListVec"))
    if "ModuleFolderDB" not in cond_kwargs:
        acc.merge(addFoldersSplitOnline(flags,
                                        detDb=cond_kwargs["dbInstance"],
                                        onlineFolders=cond_kwargs["ModuleFolder"],
                                        offlineFolders=cond_kwargs["ModuleFolder"],
                                        className="CondAttrListVec",
                                        splitMC=True))
    else:
        acc.merge(addFolders(flags, [cond_kwargs["ModuleFolderDB"]], detDb=cond_kwargs["dbInstance"], className="CondAttrListVec"))
    if "MurFolderDB" not in cond_kwargs:
        acc.merge(addFoldersSplitOnline(flags,
                                        detDb=cond_kwargs["dbInstance"],
                                        onlineFolders=cond_kwargs["MurFolder"],
                                        offlineFolders=cond_kwargs["MurFolder"],
                                        className="CondAttrListVec",
                                        splitMC=True))
    else:
        acc.merge(addFolders(flags, [cond_kwargs["MurFolderDB"]], detDb=cond_kwargs["dbInstance"],  className="CondAttrListVec"))

    ConfigCondAlg_kwargs = {}
    ConfigCondAlg_kwargs["ReadKeyChannel"] = cond_kwargs["ChannelFolder"]
    ConfigCondAlg_kwargs["ReadKeyModule"]  = cond_kwargs["ModuleFolder"]
    ConfigCondAlg_kwargs["ReadKeyMur"]     = cond_kwargs["MurFolder"]
    acc.merge(ITkStripConfigurationCondAlgCfg(flags, name="ITkStripConfigurationCondAlg", **ConfigCondAlg_kwargs))

    acc.setPrivateTools(CompFactory.SCT_ConfigurationConditionsTool(name, **kwargs))
    return acc


def ITkStripDCSConditionsCfg(flags, name="ITkStripDCSConditions", **kwargs):
    """Return a ComponentAccumulator configured for ITk Strip DCS Conditions"""
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_DCSConditionsCfg
    return SCT_DCSConditionsCfg(flags, name, ReturnHVTemp=False,      #do not use DCS conditions until they're ready for ITk
                                **kwargs)



def ITkStripFlaggedConditionToolCfg(flags, name="ITkStripFlaggedConditionTool", **kwargs):
    acc = ITkStripReadoutGeometryCfg(flags)
    acc.setPrivateTools(CompFactory.SCT_FlaggedConditionTool(name, **kwargs))
    return acc


def ITkStripModuleVetoCfg(flags, name="ITkStripModuleVeto", **kwargs):
    """Return a ComponentAccumulator configured for ITkStripModuleVetoTool
    """
    # For SCT_ID used in SCT_ModuleVetoTool
    acc = GeoModelCfg(flags)
    print("ITkStripModuleVetoCfg - (AB) -HELLO!")
    
    
    kwargs.setdefault("useDB", True) #(AB)
    kwargs.setdefault("BadModuleIdentifiers", [])

    if kwargs["useDB"]:
        #kwargs.setdefault("BadModuleIdentifiers", ["database"])
        kwargs["BadModuleIdentifiers"] = ["database"]
        #flags.IOVDb.DBConnection="sqlite://;schema=/afs/cern.ch/work/a/anburger/COOLAthena/project/mycool.db;dbname=CONDBR2"
        #flags.IOVDb.SqliteInput="/afs/cern.ch/work/a/anburger/COOLAthena/project/mycool.db"
        #flags.IOVDb.GlobalTag = ""
        #flags.IOVDb.DatabaseInstance = ""
        print("ADD MY PERSONAL DATABASE - (AB)")
        print("(AB) - BadModuleIdentifiers : {}".format(kwargs["BadModuleIdentifiers"]))
        # Condition folder
        acc.merge(addFolders(flags,
                             folderStrings="/ITk/Manual/BadModules",
                             detDb="/afs/cern.ch/work/a/anburger/COOLAthena/project/mycool.db", #(AB)
                             className="AthenaAttributeList", #AthenaAttributeList
                            # tag=kwargs["folderTag"],
                             tag="OFLCOND-MC15c-SDR-14-05",
                             extensible=True,
                             db="CONDBR2"
                             ))            
        # Condition algorithm
        acc.addCondAlgo(CompFactory.SCT_ModuleVetoCondAlg())

    # Condition tool
    toolArgs = {}
    toolArgs["JsonPath"] = flags.ITk.JsonPathStripModuleVeto
    toolArgs["BadModuleIdentifiers"] = kwargs["BadModuleIdentifiers"]
    acc.setPrivateTools(CompFactory.SCT_ModuleVetoTool(name=f"{name}Tool", **toolArgs))

    return acc


def ITkStripMonitorConditionsToolCfg(flags, name="ITkStripMonitorConditionsTool", cond_kwargs={}, **kwargs):
    cond_kwargs.setdefault("Folder", "/ITkStrip/Derived/Monitoring")
    cond_kwargs.setdefault("dbInstance", "ITkStrip_OFL")
    cond_kwargs.setdefault("MonitorCondAlgName", "ITkStripMonitorCondAlg")

    kwargs.setdefault("CondKey", "ITkStripMonitorCondData")

    acc = ITkStripReadoutGeometryCfg(flags)

    if "FolderDb" not in cond_kwargs:
        cond_kwargs["FolderDb"] = cond_kwargs["Folder"]
    acc.merge(addFolders(flags, cond_kwargs["FolderDb"], cond_kwargs["dbInstance"], className="CondAttrListCollection"))

    acc.addCondAlgo(CompFactory.SCT_MonitorCondAlg(name=cond_kwargs["MonitorCondAlgName"],
                                                   ReadKey=cond_kwargs["Folder"],
                                                   WriteKey=kwargs["CondKey"]))

    acc.setPrivateTools(CompFactory.SCT_MonitorConditionsTool(name, **kwargs))
    return acc


def ITkStripReadCalibDataToolCfg(flags, name="ITkStripReadCalibDataTool", cond_kwargs={}, **kwargs):
    acc = ITkStripReadoutGeometryCfg(flags)

    cond_kwargs.setdefault("NoiseFolder","/ITkStrip/DAQ/Calibration/NoiseOccupancyDefects")
    cond_kwargs.setdefault("GainFolder","/ITkStrip/DAQ/Calibration/NPtGainDefects")
    cond_kwargs.setdefault("ReadCalibDataCondAlgName","ITkStripReadCalibDataCondAlg")

    acc.merge(addFoldersSplitOnline(flags,
                                    detDb="ITkStrip",
                                    onlineFolders=cond_kwargs["NoiseFolder"],
                                    offlineFolders=cond_kwargs["NoiseFolder"],
                                    className="CondAttrListCollection",
                                    splitMC=True))
    acc.merge(addFoldersSplitOnline(flags,
                                    detDb="ITkStrip",
                                    onlineFolders=cond_kwargs["GainFolder"],
                                    offlineFolders=cond_kwargs["GainFolder"],
                                    className="CondAttrListCollection",
                                    splitMC=True))

    acc.addCondAlgo(CompFactory.SCT_ReadCalibDataCondAlg(name=cond_kwargs["ReadCalibDataCondAlgName"],
                                                         ReadKeyGain=cond_kwargs["GainFolder"],
                                                         ReadKeyNoise=cond_kwargs["NoiseFolder"]))

    from ITkStripCabling.ITkStripCablingConfig import ITkStripCablingToolCfg
    kwargs.setdefault("SCT_CablingTool", acc.popToolsAndMerge(ITkStripCablingToolCfg(flags)))

    acc.setPrivateTools(CompFactory.SCT_ReadCalibDataTool(name, **kwargs))
    return acc


def ITkStripReadoutToolCfg(flags, name="ITkStripReadoutTool", **kwargs):
    from ITkStripCabling.ITkStripCablingConfig import ITkStripCablingToolCfg
    acc = ITkStripCablingToolCfg(flags)
    kwargs.setdefault("SCT_CablingTool", acc.popPrivateTools())
    acc.setPrivateTools(CompFactory.SCT_ReadoutTool(name, **kwargs))
    return acc


def ITkStripSiliconConditionsCfg(flags, name="ITkStripSilicon", **kwargs):
    """Return a ComponentAccumulator configured for ITk Strip SiliconConditions"""
    acc = ComponentAccumulator()
    
    kwargs["DCSConditionsTool"] = acc.popToolsAndMerge(ITkStripDCSConditionsCfg(flags))

    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_SiliconConditionsCfg
    acc.setPrivateTools(acc.popToolsAndMerge(SCT_SiliconConditionsCfg(flags, name, useDCS = False, **kwargs)))
    return acc


def ITkStripTdaqEnabledToolCfg(flags, name="ITkStripTdaqEnabledTool", **kwargs):
    # For SCT_ID used in SCT_TdaqEnabledTool
    acc = GeoModelCfg(flags)

    # Folder
    # FIXME - is there a better way to do this?
    folder = "/TDAQ/Resources/ATLAS/ITkStrip/Robins" if flags.IOVDb.DatabaseInstance == "CONDBR2" else "/TDAQ/EnabledResources/ATLAS/ITkStrip/Robins"
    acc.merge(addFolders(flags, [folder], detDb="TDAQ", className="CondAttrListCollection"))

    # Algorithm
    from ITkStripCabling.ITkStripCablingConfig import ITkStripCablingToolCfg
    kwargs.setdefault("SCT_CablingTool", acc.popToolsAndMerge(ITkStripCablingToolCfg(flags)))
    acc.addCondAlgo(CompFactory.SCT_TdaqEnabledCondAlg(**kwargs))

    # Tool
    acc.setPrivateTools(CompFactory.SCT_TdaqEnabledTool(name))
    return acc
