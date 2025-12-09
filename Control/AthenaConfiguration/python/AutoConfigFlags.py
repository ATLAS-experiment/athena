# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from PyUtils.MetaReader import read_metadata, lite_primary_keys_to_keep, lite_TagInfo_keys_to_keep
from AthenaCommon.Logging import logging
from functools import lru_cache

msg = logging.getLogger('AutoConfigFlags')

# Module level cache of file-metadata:
_fileMetaData = dict()

class DynamicallyLoadMetadata:
    def __init__(self, filename, maxLevel='peeker'):
        self.metadata = {}
        self.filename = filename
        self.currentAccessLevel = 'lite'
        self.maxAccessLevel = maxLevel
        thisFileMD = read_metadata(filename, None, 'lite')
        self.metadata.update(thisFileMD[self.filename])
        msg.debug("Loaded using 'lite' %s", str(self.metadata))

    def _loadMore(self, level):
        self.currentAccessLevel = level

        thisFileMD = read_metadata(self.filename, None, level)
        self.metadata.update(thisFileMD[self.filename])

    def get(self, key, default):
        if key in self.metadata:
            return self.metadata[key]
        if key in lite_primary_keys_to_keep or key in lite_TagInfo_keys_to_keep:
            # no need to load more
            return default
        
        if self.currentAccessLevel == self.maxAccessLevel:
            return default

        levels = []
        if self.currentAccessLevel == 'lite':
            levels = ['peeker', 'full'] if self.maxAccessLevel == 'full' else ['peeker']
        elif self.currentAccessLevel == 'peeker':
            levels = ['full']

        for level in levels:
            msg.info("Looking into the file in '%s' mode as the configuration requires more details: %s ", level, key)
            self._loadMore(level)
            if key in self.metadata:
                return self.metadata[key]

        return default

    def __contains__(self, key):
        self.get(key, None)
        return key in self.metadata

    def __getitem__(self, key):
        return self.get(key, None)

    def __repr__(self):
        return repr(self.metadata)

    def keys(self):
        return self.metadata.keys()

def GetFileMD(filenames, allowEmpty=True, maxLevel='peeker'):
    if not filenames:
        if allowEmpty:
            msg.info("Running an input-less job. Will have empty metadata.")
            return {}
        raise RuntimeError("Metadata can not be read in an input-less job.")
    if isinstance(filenames, str):
        filenames = [filenames]
    if '_ATHENA_GENERIC_INPUTFILE_NAME_' in filenames:
        raise RuntimeError('Input file name not set, instead _ATHENA_GENERIC_INPUTFILE_NAME_ found. Cannot read metadata.')
    for filename in filenames:
        if filename not in _fileMetaData:
            msg.info("Obtaining metadata of auto-configuration by peeking into '%s'", filename)
            _fileMetaData[filename] = DynamicallyLoadMetadata(filename, maxLevel)
        if _fileMetaData[filename].maxAccessLevel != maxLevel:
            _fileMetaData[filename].maxAccessLevel = maxLevel
        if _fileMetaData[filename]['nentries'] not in [None, 0]: 
            return _fileMetaData[filename]
        else:
            msg.info("The file: %s has no entries, going to the next one for harvesting the metadata", filename)
    msg.info("No file with events found, returning anyways metadata associated to the first file %s", filenames[0])
    return _fileMetaData[filenames[0]]

def _initializeGeometryParameters(geoTag,sqliteDB,sqliteDBFullPath):
    """Read geometry database for all detectors"""

    from AtlasGeoModel import CommonGeoDB
    from PixelGeoModel import PixelGeoDB
    from LArGeoAlgsNV import LArGeoDB
    from MuonGeoModel import MuonGeoDB

    if not sqliteDB:
        # Read parameters from Oracle/Frontier
        from AtlasGeoModel.AtlasGeoDBInterface import AtlasGeoDBInterface
        dbGeomCursor = AtlasGeoDBInterface(geoTag)
        dbGeomCursor.ConnectAndBrowseGeoDB()

        params = { 'Common' : CommonGeoDB.InitializeGeometryParameters(dbGeomCursor),
                   'Pixel' : PixelGeoDB.InitializeGeometryParameters(dbGeomCursor),
                   'LAr' : LArGeoDB.InitializeGeometryParameters(dbGeomCursor),
                   'Muon' : MuonGeoDB.InitializeGeometryParameters(dbGeomCursor),
                   'Luminosity' : CommonGeoDB.InitializeLuminosityDetectorParameters(dbGeomCursor),
               }

        msg.debug('Config parameters retrieved from Geometry DB (Frontier/Oracle):')
        for key in params.keys():
            msg.debug(f'{key} -> {params[key]}')
    else:
        # Read parameters from SQLite
        from AtlasGeoModel.AtlasGeoDBInterface import AtlasGeoDBInterface_SQLite
        sqliteReader = AtlasGeoDBInterface_SQLite(geoTag,sqliteDBFullPath)
        sqliteReader.ConnectToDB()

        params = { 'Common' : CommonGeoDB.InitializeGeometryParameters_SQLite(sqliteReader),
                   'Pixel' : PixelGeoDB.InitializeGeometryParameters_SQLite(sqliteReader),
                   'LAr' : LArGeoDB.InitializeGeometryParameters_SQLite(sqliteReader),
                   'Muon' : MuonGeoDB.InitializeGeometryParameters_SQLite(sqliteReader),
                   'Luminosity' : CommonGeoDB.InitializeLuminosityDetectorParameters_SQLite(sqliteReader),
               }

        msg.debug('Config parameters retrieved from Geometry DB (SQLite):')
        for key in params.keys():
            msg.debug(f'{key} -> {params[key]}')

    return params


@lru_cache(maxsize=4)  # maxsize=1 should be enough for most jobs
def DetDescrInfo(geoTag, sqliteDB, sqliteDBFullPath):
    """Query geometry DB for detector description. Returns dictionary with
    detector description. Queries DB for each tag only once.

    geoTag: geometry tag (e.g. ATLAS-R2-2016-01-00-01)
    """
    if not geoTag:
        raise ValueError("No geometry tag specified")

    detDescrInfo = _initializeGeometryParameters(geoTag,sqliteDB,sqliteDBFullPath)
    detDescrInfo["geomTag"] = geoTag
    return detDescrInfo


@lru_cache(maxsize=4)  # maxsize=1 should be enough for most jobs
def getDefaultDetectors(geoTag, sqliteDB, sqliteDBFullPath, includeForward=False):
    """Query geometry DB for detector description.
    Returns a set of detectors used in a geometry tag.

    geoTag: geometry tag (e.g. ATLAS-R2-2016-01-00-01)
    """
    detectors = set()
    detectors.add('Bpipe')

    if DetDescrInfo(geoTag,sqliteDB,sqliteDBFullPath)['Common']['Run'] not in ['RUN1', 'RUN2', 'RUN3']: # RUN4 and beyond
        detectors.add('ITkPixel')
        detectors.add('ITkStrip')
        if DetDescrInfo(geoTag,sqliteDB,sqliteDBFullPath)['Luminosity']['BCMPrime']:
            pass  # keep disabled for now
        if DetDescrInfo(geoTag,sqliteDB,sqliteDBFullPath)['Luminosity']['PLR']:
            detectors.add('PLR')
    else:
        detectors.add('Pixel')
        detectors.add('SCT')
        detectors.add('TRT')
        detectors.add('BCM')
    # TODO: wait for special table in the geo DB
    # if DetDescrInfo(geoTag)['Common']['Run'] == 'RUN4':
    #     detectors.add('BCMPrime')

    if DetDescrInfo(geoTag,sqliteDB,sqliteDBFullPath)['Common']['Run'] not in ['RUN1', 'RUN2', 'RUN3']: # RUN4 and beyond
        detectors.add('HGTD')

    detectors.add('LAr')
    detectors.add('Tile')
    if DetDescrInfo(geoTag,sqliteDB,sqliteDBFullPath)['Common']['Run'] in ['RUN1', 'RUN2', 'RUN3']:
        detectors.add('MBTS')

    if DetDescrInfo(geoTag,sqliteDB,sqliteDBFullPath)['Muon']['HasMDT']:
        detectors.add('MDT')
    if DetDescrInfo(geoTag,sqliteDB,sqliteDBFullPath)['Muon']['HasRPC']:
         detectors.add('RPC')
    if DetDescrInfo(geoTag,sqliteDB,sqliteDBFullPath)['Muon']['HasTGC']:
        detectors.add('TGC')
    if DetDescrInfo(geoTag,sqliteDB,sqliteDBFullPath)['Muon']['HasCSC']:
        detectors.add('CSC')
    if DetDescrInfo(geoTag,sqliteDB,sqliteDBFullPath)['Muon']['HasSTGC']:
        detectors.add('sTGC')
    if DetDescrInfo(geoTag,sqliteDB,sqliteDBFullPath)['Muon']['HasMM']:
        detectors.add('MM')

    if includeForward:
        detectors.add('Lucid')
        if DetDescrInfo(geoTag,sqliteDB,sqliteDBFullPath)['Common']['Run'] not in ['RUN1']:
            detectors.add('AFP')
        detectors.add('ZDC')
        detectors.add('ALFA')
        detectors.add('FwdRegion')

    return detectors


# Based on RunDMCFlags.py
def getRunToTimestampDict():
    # this wrapper is intended to avoid an initial import
    from .RunToTimestampData import RunToTimestampDict
    return RunToTimestampDict


def getInitialTimeStampsFromRunNumbers(runNumbers):
    """This is used to hold a dictionary of the form
    {152166:1269948352889940910, ...} to allow the
    timestamp to be determined from the run.
    """
    run2timestampDict =  getRunToTimestampDict()
    timeStamps = [run2timestampDict.get(runNumber,1) for runNumber in runNumbers] # Add protection here?
    return timeStamps


def getGeneratorsInfo(flags):
    """Read in GeneratorsInfo from the input file
    """
    from AthenaConfiguration.Enums import ProductionStep
    inputFiles = flags.Input.Files
    if flags.Common.ProductionStep in [ProductionStep.Overlay, ProductionStep.FastChain] and flags.Input.SecondaryFiles and not flags.Overlay.ByteStream:
        # Do something special for MC Overlay
        inputFiles = flags.Input.SecondaryFiles
    generatorsString = ""
    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    if inputFiles:
        generatorsString = GetFileMD(inputFiles).get("generators", "")
    from GeneratorConfig.Versioning import generatorsGetFromMetadata
    return generatorsGetFromMetadata( generatorsString )


def getSpecialConfigurationMetadata(flags):
    """Read in special simulation job option fragments based on metadata
    passed by the evgen stage
    """
    specialConfigDict = dict()
    legacyPreIncludeToCAPostInclude = { 'SimulationJobOptions/preInclude.AMSB.py' : 'Charginos.CharginosConfig.AMSB_Cfg',
                                        'SimulationJobOptions/preInclude.Monopole.py' :  'Monopole.MonopoleConfig.MonopoleCfg',
                                        'SimulationJobOptions/preInclude.Quirks.py' : 'Quirks.QuirksConfig.QuirksCfg',
                                        'SimulationJobOptions/preInclude.SleptonsLLP.py' : 'Sleptons.SleptonsConfig.SleptonsLLPCfg',
                                        'SimulationJobOptions/preInclude.GMSB.py' : 'Sleptons.SleptonsConfig.GMSB_Cfg',
                                        'SimulationJobOptions/preInclude.Qball.py' : 'Monopole.MonopoleConfig.QballCfg',
                                        'SimulationJobOptions/preInclude.RHadronsPythia8.py' : 'RHadrons.RHadronsConfig.RHadronsCfg',
                                        'SimulationJobOptions/preInclude.fcp.py' : 'Monopole.MonopoleConfig.fcpCfg',
                                        'SimulationJobOptions/preInclude.Dyon.py' : 'Monopole.MonopoleConfig.DyonCfg' }
    legacyPreIncludeToCAPreInclude = { 'SimulationJobOptions/preInclude.AMSB.py' : None,
                                       'SimulationJobOptions/preInclude.Monopole.py' :  'Monopole.MonopoleConfig.MonopolePreInclude',
                                       'SimulationJobOptions/preInclude.Quirks.py' : None,
                                       'SimulationJobOptions/preInclude.SleptonsLLP.py' : None,
                                       'SimulationJobOptions/preInclude.GMSB.py' : None,
                                       'SimulationJobOptions/preInclude.Qball.py' : 'Monopole.MonopoleConfig.QballPreInclude',
                                       'SimulationJobOptions/preInclude.RHadronsPythia8.py' : 'RHadrons.RHadronsConfig.RHadronsPreInclude',
                                       'SimulationJobOptions/preInclude.fcp.py' : 'Monopole.MonopoleConfig.fcpPreInclude',
                                       'SimulationJobOptions/preInclude.Dyon.py' : 'Monopole.MonopoleConfig.DyonPreInclude' }
    specialConfigString = ''
    from AthenaConfiguration.Enums import ProductionStep
    inputFiles = flags.Input.Files
    secondaryInputFiles = flags.Input.SecondaryFiles
    if flags.Common.ProductionStep in [ProductionStep.Overlay, ProductionStep.FastChain] and not flags.Overlay.DataOverlay and flags.Input.SecondaryFiles:
        # Do something special for MC Overlay
        inputFiles = flags.Input.SecondaryFiles
        secondaryInputFiles = flags.Input.Files
    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    if len(inputFiles)>0:
        specialConfigString = GetFileMD(inputFiles).get('specialConfiguration', '')
    if (not len(specialConfigString) or specialConfigString == 'NONE') and len(secondaryInputFiles)>0:
        # If there is no specialConfiguration metadata in the primary
        # input try the secondary inputs (MC Overlay case)
        specialConfigString = GetFileMD(secondaryInputFiles).get('specialConfiguration', '')
    if len(specialConfigString)>0:
        ## Parse the specialConfiguration string
        ## Format is 'key1=value1;key2=value2;...'. or just '
        spcitems = specialConfigString.split(";")
        for spcitem in spcitems:
            #print spcitem
            ## Ignore empty or "NONE" substrings, e.g. from consecutive or trailing semicolons
            if not spcitem or spcitem.upper() == "NONE":
                continue
            ## If not in key=value format, treat as v, with k="preInclude"
            if "=" not in spcitem:
                spcitem = "preInclude=" + spcitem
            ## Handle k=v directives
            k, v = spcitem.split("=")
            if k == "preInclude" and v.endswith('.py'): # Translate old preIncludes into CA-based versions.
                if v == 'SimulationJobOptions/preInclude.RhadronsPythia8.py':
                    v = 'SimulationJobOptions/preInclude.RHadronsPythia8.py' # ATLASSIM-6687 Fixup for older EVNT files
                v1 = legacyPreIncludeToCAPreInclude[v]
                if v1 is not None:
                    specialConfigDict[k] = v1
                v2 = legacyPreIncludeToCAPostInclude[v]
                if v2 is not None:
                    specialConfigDict['postInclude'] = v2
            else:
                specialConfigDict[k] = v
    return specialConfigDict

