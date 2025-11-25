# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# default configuration of the PhotonIsEMSelectorCutDefs



def PhotonIsEMLooseSelectorConfig(theTool):
    '''
    These are the photon isEM definitions *DC14* Loose
    '''

    #
    # PHOTON Loose cuts, with updated using *DC14*.
    #
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc15_20150712/PhotonIsEMLooseSelectorCutDefs.conf"


def PhotonIsEMMediumSelectorConfig(theTool):
    '''
    These are the photon isEM definitions from *DC14*
    '''

    # MEDIUM (10/05/24 - see ATLASG-2708)
    #  Coming from Fer
    #
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20240510/PhotonIsEMMediumSelectorCutDefs_pTdep_smooth.conf"


# Cut-based tight menu for MC20 / Run2
# Note: keep this conf file up to date with the PhotonCutPointToConfFile map in Root/EGSelectorConfigurationMapping.h
def PhotonIsEMTightSelectorConfigMC20(theTool):
    '''
    These are the photon isEM definitions for Tight menu for MC20 / Run2
    '''

    #
    # Tight (10/05/24 - see ATLASG-2708)
    #
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20240510/PhotonIsEMTightSelectorCutDefs_pTdep_mc20_smooth.conf"


# Cut-based tight menu for MC21 / MC23 / Run3
# Note: keep this conf file up to date with the PhotonCutPointToConfFile map in Root/EGSelectorConfigurationMapping.h
def PhotonIsEMTightSelectorConfigMC21(theTool):
    '''
    These are the photon isEM definitions for Tight menu for MC21 / Run3
    '''

    #
    # Tight (same as Run2, to be updated)
    #
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20240510/PhotonIsEMTightSelectorCutDefs_pTdep_mc20_smooth.conf"
