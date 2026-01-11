# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
This map specifies with menu to load from TriggerMenuMT/python/L1/Menu

The mapping takes precedence over the existence of the menu definition file in the above directory
The resolved name is also being used in the L1/Config/ItemDef.py and L1/Config/ThresholdDef*.py files

Prescale sets will be stripped from the menu name
"""

from enum import Enum
import re

from PyUtils.moduleExists import moduleExists
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


menuMap = {
    # pp
    "Physics_pp_run3_v1"                        : ["Physics_pp_run3_v1","Physics_pp_run3_v1_inputs","Physics_pp_run3_v1_inputs_legacy"],
    "PhysicsP1_pp_run3_v1"                      : ["Physics_pp_run3_v1","Physics_pp_run3_v1_inputs","Physics_pp_run3_v1_inputs_legacy"],
    "MC_pp_run3_v1"                             : ["MC_pp_run3_v1",     "MC_pp_run3_v1_inputs",     "Physics_pp_run3_v1_inputs_legacy"],
    "Dev_pp_run3_v1"                            : ["MC_pp_run3_v1",     "MC_pp_run3_v1_inputs",     "Physics_pp_run3_v1_inputs_legacy"],

    # pp for run 4
    "Physics_pp_run4_v1"                        : ["Physics_pp_run4_v1","Physics_pp_run4_v1_inputs","Physics_pp_run4_v1_inputs_legacy"],
    #"PhysicsP1_pp_run4_v1"                      : ["Physics_pp_run4_v1","Physics_pp_run4_v1_inputs","Physics_pp_run4_v1_inputs_legacy"],
    "MC_pp_run4_v1"                             : ["MC_pp_run4_v1",     "MC_pp_run4_v1_inputs",     "Physics_pp_run4_v1_inputs_legacy"],
    "Dev_pp_run4_v1"                            : ["MC_pp_run4_v1",     "MC_pp_run4_v1_inputs",     "Physics_pp_run4_v1_inputs_legacy"],

    # low mu
    "PhysicsP1_pp_lowMu_run3_v1"                : ["Physics_HI_run3_v1", "Physics_HI_run3_v1_inputs", "Physics_HI_run3_v1_inputs_legacy"],
    "Dev_pp_lowMu_run3_v1"                      : ["Physics_HI_run3_v1", "Physics_HI_run3_v1_inputs", "Physics_HI_run3_v1_inputs_legacy"],

    # cosmics
    "Cosmic_run3_v1"                            : ["Physics_pp_run3_v1","Physics_pp_run3_v1_inputs","Physics_pp_run3_v1_inputs_legacy"],

    # HI
    "PhysicsP1_HI_run3_v1"                      : ["Physics_HI_run3_v1", "Physics_HI_run3_v1_inputs", "Physics_HI_run3_v1_inputs_legacy"],
    "MC_HI_run3_v1"                             : ["MC_HI_run3_v1",      "Physics_HI_run3_v1_inputs", "Physics_HI_run3_v1_inputs_legacy"],
    "Dev_HI_run3_v1"                            : ["MC_HI_run3_v1",      "Physics_HI_run3_v1_inputs", "Physics_HI_run3_v1_inputs_legacy"],

    # Special dummy menus needed for compiling CTPIN switch matrix
    "AllCTPIn_pp_run3_v1"                       : ["AllCTPIn_run3_v1",  "Physics_pp_run3_v1_inputs", "Physics_pp_run3_v1_inputs_legacy"],
    "AllCTPIn_HI_run3_v1"                       : ["AllCTPIn_run3_v1",  "Physics_HI_run3_v1_inputs", "Physics_HI_run3_v1_inputs_legacy"],
}

defaultRun4 = "Physics_pp_run4_v2"

class MenuMetaInfo:
    class MenuType(Enum):
        PHYSICS = "Physics"
        MC = "MC"
        DEV = "Dev"
        HI = "HI"
        ALLCTPIN = "AllCTPIn"

    """Class to hold meta information about a menu, e.g. run, purpose (Phuysics, MC, Dev, HI, AllCTPIN)
    and input files.
    """
    def __init__(self, menuFullName: str, menuBaseName: str, menuFile: str, menuInputFile: str, menuLegacyInputFile: str, menuPurpose: MenuType, run: int):
        self.menuFullName: str = menuFullName
        self.menuBaseName: str = menuBaseName
        self.menuFile: str = menuFile
        self.menuInputFile: str = menuInputFile
        self.menuLegacyInputFile = menuLegacyInputFile
        self.menuPurpose: MenuMetaInfo.MenuType = menuPurpose
        self.run: int = run

    def __str__(self):
        return f"menuName = {self.menuFullName}, menuFile={self.menuFile}, menuInputFile={self.menuInputFile}, menuLegacyInputFile={self.menuLegacyInputFile}, menuPurpose={self.menuPurpose}, run={self.run}"

    def __repr__(self):
        return f"MenuMetaInfo(menuName={self.menuFullName}, menuFile={self.menuFile}, menuInputFile={self.menuInputFile}, menuLegacyInputFile={self.menuLegacyInputFile}, menuPurpose={self.menuPurpose}, run={self.run})"

def menuResolution(fullMenuName) -> MenuMetaInfo:
    """Resolve the menu name to the actual menu to be used based from the mapping above.
    """

    # remove prescale suffix
    menuBaseName = fullMenuName
    if fullMenuName.endswith('prescale'):
        # take everything upto _vXX
        if (m:=re.match(r'\w*_v\d*', fullMenuName)) is not None:
            menuBaseName = m.group(0)


    if (menuInfo := menuMap.get(menuBaseName)) is not None:
        menuFile, menuInputFile, menuLegacyInputFile = (menuInfo + ["",""])[:3]
    else:
        menuFile = menuBaseName
        menuInputFile = menuFile + "_inputs"
        menuLegacyInputFile = ""

    menuPurpose = MenuMetaInfo.MenuType.PHYSICS
    if "MC_" in menuBaseName:
        menuPurpose = MenuMetaInfo.MenuType.MC
    elif "Dev_" in menuBaseName:
        menuPurpose = MenuMetaInfo.MenuType.DEV
    elif "HI_" in menuBaseName:
        menuPurpose = MenuMetaInfo.MenuType.HI
    elif "AllCTPIn_" in menuBaseName:
        menuPurpose = MenuMetaInfo.MenuType.ALLCTPIN

    run = 3 if "run3" in menuBaseName else 4        
    
    # for run 4 fall back to default menu during LS3 development period
    moduleName = 'TriggerMenuMT.L1.Menu.Menu_%s' % menuFile
    if not moduleExists(moduleName):
        if run == 4:
            log.info("Could not find menu definition for %s (module %s does not exist)", fullMenuName, moduleName)
            log.info("Falling back to default menu '%s'.", defaultRun4)
            menuFile = defaultRun4
            menuInputFile = menuFile + "_inputs"
            menuLegacyInputFile = ""
        else:
            raise ModuleNotFoundError(f"Could not find menu definition for {fullMenuName}")

    menuMeta = MenuMetaInfo(fullMenuName, menuBaseName, menuFile, menuInputFile, menuLegacyInputFile, menuPurpose, run)

    return menuMeta