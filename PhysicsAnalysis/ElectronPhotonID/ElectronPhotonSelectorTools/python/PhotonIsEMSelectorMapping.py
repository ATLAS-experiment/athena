# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# =============================================================================
#  Name:        PhotonIsEMSelectorMapping.py
#
# Author:      Tulay Cuhadar Donszelmann, Jovan Mitrevski
# Created:     Dec 2011
#
# Description: Find mapping of mask and function for ID quality
# =============================================================================

#
import ElectronPhotonSelectorTools.PhotonIsEMMenuDefs as PhotonIsEMMenuDefs
from ElectronPhotonSelectorTools.EgammaPIDdefs import egammaPID

#
# Define here the mapping between a "menu" and the corresponding selectors
# The latter are configured in PhotonIsEMMenuDefs.py
#


class photonPIDmenu:
    offlineMC20 = 1
    offlineMC21 = 2

# format - key: (mask, function)

PhotonIsEMMapOfflineMC20 = {
    egammaPID.PhotonIDLoose:  (
        egammaPID.PhotonLoose,
        PhotonIsEMMenuDefs.PhotonIsEMLooseSelectorConfig),
    egammaPID.PhotonIDMedium: (
        egammaPID.PhotonMedium,
        PhotonIsEMMenuDefs.PhotonIsEMMediumSelectorConfig),
    egammaPID.PhotonIDTight:  (
        egammaPID.PhotonTight,
        PhotonIsEMMenuDefs.PhotonIsEMTightSelectorConfigMC20),

    egammaPID.PhotonIDLooseAR:  (
        egammaPID.PhotonLooseAR,
        PhotonIsEMMenuDefs.PhotonIsEMLooseSelectorConfig),
    egammaPID.PhotonIDMediumAR: (
        egammaPID.PhotonMediumAR,
        PhotonIsEMMenuDefs.PhotonIsEMMediumSelectorConfig),
    egammaPID.PhotonIDTightAR:  (
        egammaPID.PhotonTightAR,
        PhotonIsEMMenuDefs.PhotonIsEMTightSelectorConfigMC20),
    egammaPID.NoIDCut: (
        0,
        PhotonIsEMMenuDefs.PhotonIsEMLooseSelectorConfig)
}

PhotonIsEMMapOfflineMC21 = {
    egammaPID.PhotonIDLoose:  (
        egammaPID.PhotonLoose,
        PhotonIsEMMenuDefs.PhotonIsEMLooseSelectorConfig),
    egammaPID.PhotonIDMedium: (
        egammaPID.PhotonMedium,
        PhotonIsEMMenuDefs.PhotonIsEMMediumSelectorConfig),
    egammaPID.PhotonIDTight:  (
        egammaPID.PhotonTight,
        PhotonIsEMMenuDefs.PhotonIsEMTightSelectorConfigMC21),

    egammaPID.PhotonIDLooseAR:  (
        egammaPID.PhotonLooseAR,
        PhotonIsEMMenuDefs.PhotonIsEMLooseSelectorConfig),
    egammaPID.PhotonIDMediumAR: (
        egammaPID.PhotonMediumAR,
        PhotonIsEMMenuDefs.PhotonIsEMMediumSelectorConfig),
    egammaPID.PhotonIDTightAR:  (
        egammaPID.PhotonTightAR,
        PhotonIsEMMenuDefs.PhotonIsEMTightSelectorConfigMC21),
    egammaPID.NoIDCut: (
        0,
        PhotonIsEMMenuDefs.PhotonIsEMLooseSelectorConfig)
}


def PhotonIsEMMap(quality, menu):
    if menu == photonPIDmenu.offlineMC20 and quality in PhotonIsEMMapOfflineMC20.keys():
        return PhotonIsEMMapOfflineMC20[quality]
    elif menu == photonPIDmenu.offlineMC21 and quality in PhotonIsEMMapOfflineMC21.keys():
        return PhotonIsEMMapOfflineMC21[quality]
    else:
        raise ValueError("Requested menu is undefined: %d" % menu)
