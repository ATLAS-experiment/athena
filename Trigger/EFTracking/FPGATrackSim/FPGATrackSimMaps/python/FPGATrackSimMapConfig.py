# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from PyJobTransforms.trfUtils import findFile
from PyJobTransforms.trfLogger import msg
import os


def findFileWithTest(datapath,filename):
    retv = findFile(datapath,filename)
    if retv is None:
        msg.info(datapath)
        raise OSError(2, "Couldn't find file", filename)
    return retv

def getRegionIndex(map_tag):
    '''
    Note the region member of the tag is a string
    '''
    try:
        return int(map_tag['region'])
    except ValueError:
        return map_tag['regionNames'].index(map_tag['region'])
 
   
def getRegionName(map_tag):
    return map_tag['regionNames'][getRegionIndex(map_tag)]

def getSampleType(map_tag):
    return map_tag['sampleType']

def getWithPU(map_tag):
    return map_tag['withPU']

def getNSubregions(tag):
    formats = {
            'region': getRegionIndex(tag),
            'regionName': getRegionName(tag),
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