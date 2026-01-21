#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def jFexDBConfig(flags, name="jFEXCondAlgo"):


    acc=ComponentAccumulator()
    DBCond = CompFactory.LVL1.jFEXCondAlgo(name)

    ModSettings_folder  = "/TRIGGER/L1Calo/V1/Calibration/JfexModuleSettings"
    NoiseCut_folder     = "/TRIGGER/L1Calo/V1/Calibration/JfexNoiseCuts"
    SysSettingst_folder = "/TRIGGER/L1Calo/V1/Calibration/JfexSystemSettings"

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    database = "TRIGGER_OFL" if flags.Input.isMC else "TRIGGER_ONL"
    acc.merge(addFolders(flags, ModSettings_folder , database, className="CondAttrListCollection"))
    acc.merge(addFolders(flags, NoiseCut_folder    , database, className="CondAttrListCollection"))
    acc.merge(addFolders(flags, SysSettingst_folder, database, className="CondAttrListCollection"))
    
    DBCond.JfexModuleSettings = ModSettings_folder
    DBCond.JfexNoiseCuts      = NoiseCut_folder
    DBCond.JfexSystemSettings = SysSettingst_folder

    DBCond.IsMC = flags.Input.isMC

    acc.addCondAlgo(DBCond)

    return acc


def gFexDBConfig(flags, name="gFEXCondAlgo"):

    acc=ComponentAccumulator()
    DBCond = CompFactory.LVL1.gFEXCondAlgo(name)
    NoiseCut_folder     = "/TRIGGER/L1Calo/V1/Calibration/GfexModuleSettings"
    database = "TRIGGER_OFL" if flags.Input.isMC else "TRIGGER_ONL"

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    acc.merge(addFolders(flags, NoiseCut_folder, database, className="CondAttrListCollection"))
    DBCond.GfexNoiseCuts = NoiseCut_folder

    DBCond.IsMC = flags.Input.isMC

    acc.addCondAlgo(DBCond)

    return acc
