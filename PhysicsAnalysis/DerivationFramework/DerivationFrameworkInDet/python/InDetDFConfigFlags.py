# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# configuration flags for the ID derivations

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from Campaigns.Utils import Campaign


def createInDetDFConfigFlags():
    iddcf = AthConfigFlags()
    # schedule track systematics alg
    # can only run if recommendations available
    iddcf.addFlag("Derivation.InDet.doTrackSystematics", lambda prevFlags:
                  prevFlags.Input.isMC and
                  prevFlags.Input.MCCampaign not in [Campaign.MC23g])
    return iddcf
