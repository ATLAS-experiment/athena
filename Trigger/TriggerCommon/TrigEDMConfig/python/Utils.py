# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from functools import lru_cache
from collections import defaultdict
import re
from GaudiKernel.DataHandle import DataHandle
from AthenaCommon.Logging import logging
log = logging.getLogger('TrigEDMConfig')

@lru_cache(maxsize=None)
def getEDMVersionFromBS(filename):
   """Determine Trigger EDM version based on the input ByteStream file.

   Run-3 EDM is indicated by HLT ROD version > 1.0. For Run 1 and 2 the
   HLT ROD version was 0.0 and the run number is used to disambiguate between them.
   """

   boundary_run12 = 230000
   boundary_run23 = 368000

   import eformat
   from libpyeformat_helper import SubDetector
   bs = eformat.istream(filename)

   rodVersionM = None
   rodVersionL = None
   # Find the first HLT ROBFragment in the first event
   for robf in bs[0]:
       if robf.rob_source_id().subdetector_id()==SubDetector.TDAQ_HLT:
           rodVersionM = robf.rod_minor_version() >> 8
           rodVersionL = robf.rod_minor_version() & 0xFF
           log.debug("HLT ROD minor version from input file is %d.%d", rodVersionM, rodVersionL)
           break

   # Case 1: failed to read ROD version
   if rodVersionM is None or rodVersionL is None:
       log.warning("Cannot determine HLT ROD version from input file, falling back to run-number-based decision")
   # Case 2: ROD version indicating Run 3
   elif rodVersionM >= 1:
       log.info("Determined EDMVersion to be 3, because running on BS file with HLT ROD version %d.%d",
                 rodVersionM, rodVersionL)
       return 3

   # Case 3: ROD version indicating Run 1 or 2 - use run number to disambiguate
   runNumber = bs[0].run_no()
   log.debug("Read run number %s", runNumber)

   if not runNumber or runNumber <= 0:
       log.warning("Cannot determine EDM version because run number %s is invalid. ", runNumber)
       return None
   elif runNumber < boundary_run12:
       # Run-1 data
       log.info("Determined EDMVersion to be 1 based on BS file run number (runNumber < %d)",
                 boundary_run12)
       return 1
   elif runNumber < boundary_run23:
       # Run-2 data
       log.info("Determined EDMVersion to be 2 based on BS file run number (%d < runNumber < %d)",
                 boundary_run12, boundary_run23)
       return 2
   else:
       # Run-3 data
       log.info("Determined EDMVersion to be 3 based on BS file run number (runNumber > %d)",
                 boundary_run23)
       return 3


def edmDictToList(edmDict):
    '''
    Convert EDM dictionary in the format:
        {'type1': ['key1','key2'], 'type2': ['key3']}
    to a flat list in the format:
        ['type1#key1', 'type1#key2', 'type2#key3']
    '''
    return [ f"{type}#{name}" for type, names in edmDict.items() for name in names ]


def edmListToDict(edmList):
    '''
    Convert EDM list in the format:
        ['type1#key1', 'type1#key2', 'type2#key3']
    to a dictionary in the format:
        {'type1': ['key1','key2'], 'type2': ['key3']}
    '''
    edmDict = defaultdict(list)
    for typeName in edmList:
        edmType, edmKey = typeName.split('#')
        edmDict[edmType].append(edmKey)
    return edmDict


def getEDMListFromWriteHandles(configurables):
    '''
    Build OutputStream ItemList from all WriteHandles in a list of components (configurables),
    for example a list of AlgTools. The output is in flat list format:
        ['type1#key1', 'type1#key2', 'type2#key3']
    '''

    def getWriteHandles(comp):
        properties = [getattr(comp,propName) for propName in comp.getDefaultProperties().keys()]
        return [prop for prop in properties if isinstance(prop,DataHandle) and prop.mode()=='W']

    def formatItem(containerType, containerKey):
        auxType = containerType.replace('Container','AuxContainer')
        return [f'{containerType}#{containerKey}',
                f'{auxType}#{containerKey}Aux.-']

    def containerTypedef(containerType):
        if containerType.startswith('xAOD::') and containerType.endswith('Container'):
            # Already the right typedef
            return containerType
        m = re.match(r'DataVector<xAOD::(\w+)_v[0-9]+,', containerType)
        if m and len(m.groups()) > 0:
            return f'xAOD::{m.group(1)}Container'
        raise RuntimeError(f'Failed to convert type {containerType} into a container typedef')

    def itemListFromConfigurable(comp):
        items = []
        for handle in getWriteHandles(comp):
            sgKey = handle.Path.replace('StoreGateSvc+','')
            if not sgKey:
                continue
            items += formatItem(containerTypedef(handle.Type), handle.Path)
        return items

    itemList = []
    for comp in configurables:
        itemList += itemListFromConfigurable(comp)
    return itemList
