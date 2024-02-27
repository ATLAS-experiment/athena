# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
'''
@author Riley Xu - rixu@cern.ch
@date March 3rd 2020
@brief This file declares functions to make and configure the map service.
'''

import os

from PyJobTransforms.trfUtils import findFile
from PyJobTransforms.trfLogger import msg

from FPGATrackSimMaps.FPGATrackSimMapsConf import FPGATrackSimMappingSvc, FPGATrackSimHitFilteringTool
import FPGATrackSimConfTools.FPGATrackSimConfigCompInit as FPGATrackSimConfig

def findFileWithTest(datapath,filename):
    retv = findFile(datapath,filename)
    if retv is None:
        msg.info(datapath)
        raise OSError(2, "Couldn't find file", filename)
    return retv

def addMapSvc(tag):
    '''
    Creates and returns a FPGATrackSimMapSvc object, configured with the specified tag.

    This function adds the returned map service instance to SvcMgr, and ALSO ADDS
    the EventSelectionSvc, which the map svc depends on
    '''
    FPGATrackSimConfig.addEvtSelSvc(tag)

    MyFPGATrackSimMappingSvc = FPGATrackSimMappingSvc()

    filepaths = [
            'pmap',
            'rmap',
            'modulemap',
            'subrmap',
            'NNmap',
    ]

    formats = {
            'region': FPGATrackSimConfig.getRegionIndex(tag),
            'regionName': FPGATrackSimConfig.getRegionName(tag),
    }

    for param in filepaths:
        if tag['formatted']:
            path = tag[param].format(**formats)
        else:
            path = tag[param]
        setattr(MyFPGATrackSimMappingSvc, param, path)

    MyFPGATrackSimMappingSvc.mappingType = tag['mappingType']
    MyFPGATrackSimMappingSvc.layerOverride = tag['layerOverride']
 
    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr += MyFPGATrackSimMappingSvc

    return MyFPGATrackSimMappingSvc



def addHitFilteringTool(tag):
    '''
    Creates and adds the hit filtering tool to the tool svc
    '''

    HitFilteringTool = FPGATrackSimHitFilteringTool()

    for param in HitFilteringTool.__slots__:
        if param in tag:
            setattr(HitFilteringTool, param, tag[param])

    from AthenaCommon.AppMgr import ToolSvc
    ToolSvc += HitFilteringTool

    return HitFilteringTool



def getNSubregions(tag):
    formats = {
            'region': FPGATrackSimConfig.getRegionIndex(tag),
            'regionName': FPGATrackSimConfig.getRegionName(tag),
    }

    if tag['formatted']:
        path = tag['subrmap'].format(**formats)
    else:
        path = tag['subrmap']
    path = findFile(os.environ['DATAPATH'], path)

    with open(path, 'r') as f:
        fields = f.readline().split()
        assert(fields[0] == 'towers')
        return int(fields[1])


def _applyTag(MyFPGATrackSimMappingSvc, tag):
    '''
    Helper function that sets the filepaths of the MapSvc using the supplied tag
    '''
