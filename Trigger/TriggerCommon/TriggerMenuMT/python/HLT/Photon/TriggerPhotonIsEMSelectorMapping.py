# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# =============================================================================
#  Name:        TriggerPhotonIsEMSelectorMapping.py
#
# Description: Find mapping of mask and function for ID quality
# =============================================================================

#
import TriggerMenuMT.HLT.Photon.TriggerPhotonIsEMMenuDefs as TriggerPhotonIsEMMenuDefs
from ElectronPhotonSelectorTools.EgammaPIDdefs import egammaPID

#
# The "photonPIDmenu" used to store every menu in existence...
# now we will simply update the
# location of the conf file to the new path.
# Therefore, the CURRENT MENU is: menuCurrentCuts
# (corresponding to the PhotonIsEMMapCurrent dict)

class triggerPhotonPIDmenu:
    menuCurrentCuts = 0


# format - key: (mask, function)
TriggerPhotonIsEMMapCurrent = {
    egammaPID.PhotonIDLoose:  (
        egammaPID.PhotonLoose,
        TriggerPhotonIsEMMenuDefs.PhotonIsEMLooseSelectorConfig),
    egammaPID.PhotonIDMedium: (
        egammaPID.PhotonMedium,
        TriggerPhotonIsEMMenuDefs.PhotonIsEMMediumSelectorConfig),
    egammaPID.PhotonIDTight:  (
        egammaPID.PhotonTight,
        TriggerPhotonIsEMMenuDefs.PhotonIsEMTightSelectorConfig),
}

def TriggerPhotonIsEMMap(quality, menu):
    # These are the "current menus" (non-legacy)
    if menu == triggerPhotonPIDmenu.menuCurrentCuts and quality in TriggerPhotonIsEMMapCurrent:
        return TriggerPhotonIsEMMapCurrent[quality]
    else:
        raise ValueError("Requested menu is undefined: %d" % menu)

