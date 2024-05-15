# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

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


# Current cut-based tight menu. When updating tight, simply change the config-file below
# (and please add a meaningful description of the conf file)
# Note: keep this conf file up to date with the PhotonCutPointToConfFile map in Root/EGSelectorConfigurationMapping.h


def PhotonIsEMTightSelectorConfig(theTool):
    '''
    These are the photon isEM definitions for Tight menu
    '''

    #
    # Tight (10/05/24 - see ATLASG-2708)
    #
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc20_20240510/PhotonIsEMTightSelectorCutDefs_pTdep_mc20_smooth.conf"

#
# Pt-inclusive tight ID menu, derived in January 2018.
# Needs to be kept around until the new menu is fully supported,
# but can be removed when it becomes obsolete.
#


def PhotonIsEMTightSelectorConfigPtInclJan2018(theTool):
    '''
    These are the photon isEM definitions for Tight menu
    '''

    #
    # PHOTON tight cuts, now with January 2018 re-optimization
    #
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/20180116/PhotonIsEMTightSelectorCutDefs.conf"

#
# mu-dependend tight ID menu, derived in January 2022 for Run3
# Needs to be kept around for commissioning until it will replace the Run2
# tight menu
#

def PhotonIsEMTightSelectorConfigMuDependent2022(theTool):
    '''
    These are the photon isEM definitions for the mu-dependent Tight menu
    '''

    #
    # PHOTON tight cuts, with mu-dependent January 2022 re-optimization
    #
    theTool.ConfigFile = "ElectronPhotonSelectorTools/offline/mc16_20220621/PhotonIsEMTightSelectorCutDefs.conf"

