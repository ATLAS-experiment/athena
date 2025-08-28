# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


def binstr(value, width):
    """Convert integer into binary string of given (minimum) width"""
    return f"{value:0{width}b}"


def get_smk_psk_Name(menuName):
    import re
    if "mc_prescale" in menuName:
        form = "(.*)_(.*)_mc_prescale"
        m = re.match(form, menuName)
        (smkName, pskName) = m.groups()
        pskName = pskName+"_mc" 
    elif "prescale" in menuName:
        #eg lumi1e31_simpleL1Calib_no_prescale
        form = "(.*)_(.*)_prescale"
        m = re.match(form, menuName)
        (smkName, pskName) = m.groups()
    else:
        #eg lumi1e31 ps set name can be "default"
        smkName = menuName
        pskName = "default"

    smk_psk_Name = {"smkName": str(smkName),
                    "pskName": f"{smkName}_{pskName}_prescale"}

    return smk_psk_Name
