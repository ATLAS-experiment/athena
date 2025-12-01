#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.Enums import LHCPeriod
from AthenaCommon.Logging import logging
__log = logging.getLogger('HIJetRecUtilsCA')

def getHIClusterGeoWeightFile(flags):
    """Returns the correct cluster.geo.XXX.root file based on the year and data/MC"""

    # TODO: There should be cluster.geo.XXX.root files for each geometry version (flags.GeoModel.AtlasVersion)
    #       that would contain all the relevant runs. See also ATLHI-489

    if flags.HeavyIon.Jet.HIClusterGeoWeightFile != "auto":
        return flags.HeavyIon.Jet.HIClusterGeoWeightFile

    if flags.Input.isMC:
        return 'cluster.geo.HIJING_2018.root'
    else:
        if flags.GeoModel.Run in [LHCPeriod.Run1, LHCPeriod.Run2]:
            if flags.Input.DataYear == 2015:
                return 'cluster.geo.DATA_2015.root'
            elif flags.Input.DataYear == 2017:
                return 'cluster.geo.DATA_XeXe_2017_fixedForward.root'
            elif flags.Input.DataYear == 2018:
                return 'cluster.geo.DATA_PbPb_2018v2.root'
            else:
                __log.warning("Have no cluster.geo weight file for Input.DataYear == "+str(flags.Input.DataYear)+", using cluster.geo.DATA_PbPb_2018v2.root")
                return 'cluster.geo.DATA_PbPb_2018v2.root'
        else:
            if flags.Input.DataYear == 2022:
                return 'cluster.geo.DATA_PbPb_2022.root'
            elif flags.Input.DataYear == 2023:
                return 'cluster.geo.DATA_PbPb_2023.root'
            elif flags.Input.DataYear == 2024:
                return 'cluster.geo.DATA_PbPb_2024.root'
            else:
                __log.info("Have no cluster.geo weight file for Input.DataYear == "+str(flags.Input.DataYear)+", using cluster.geo.DATA_PbPb_2024.root")
                return 'cluster.geo.DATA_PbPb_2024.root'
