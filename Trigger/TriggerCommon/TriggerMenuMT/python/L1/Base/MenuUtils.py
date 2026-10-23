# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)
import re


def binstr(value, width):
    """Convert integer into binary string of given (minimum) width"""
    return f"{value:0{width}b}"


def get_smk_psk_Name(menuName):
    smkName = menuName
    pskName = "default"  # eg lumi1e31 ps set name can be "default"
    if "mc_prescale" in menuName:
        form = "(.*)_(.*)_mc_prescale"
        if (m := re.match(form, menuName)) is not None:
            (smkName, pskName) = m.groups()
            pskName = pskName+"_mc" 
    elif "prescale" in menuName:
        #eg lumi1e31_simpleL1Calib_no_prescale
        form = "(.*)_(.*)_prescale"
        if (m := re.match(form, menuName)) is not None:
            (smkName, pskName) = m.groups()

    smk_psk_Name = {"smkName": str(smkName),
                    "pskName": f"{smkName}_{pskName}_prescale"}

    return smk_psk_Name
