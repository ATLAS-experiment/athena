# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.AutoConfigFlags import getDefaultDetectors
# This module is based upon Control/AthenaCommon/python/DetFlags.py
# Only some flags have been migrated. A full list of what the old
# DetFlags provided is given for reference below:
# detectors : ID = Bpipe Pixel SCT TRT BCM
#             ITk = Bpipe ITkPixel ITkStrip BCMPrime PLR
#             HGTD
#             Forward = Lucid ZDC ALFA AFP FwdRegion
#             Calo = LAr Tile MBTS
#             Muon = MDT CSC TGC RPC sTGC MM
# tasks:
#   geometry : setup the geometry
#   dcs : DCS information is available
#   simulate : simulate
#   simulateLVL1 : LVL1 simulation
#   detdescr : setup detector description (for anything which is not geant)
#   pileup   : collect hits from physics and min bias events
#   digitize : hit -> RDO (raw data objects)
#   makeRIO  : RDO -> RIO (Reconstruction Input Objects)
#   writeBS  : write RDO byte stream
#   readRDOBS : read RDO from byte stream
#   readRDOPool : read RDO from pool
#   readRIOBS : read RIO directly from BS
#   writeRDOPool : write RDO in pool
#   readRIOPool  : read RIO from pool
#   writeRIOPool : write RIO in pool
#   overlay : overlay setup


# all detectors - used in helper functions
allDetectors = [
    'Bpipe',
    'BCM', 'Pixel', 'SCT', 'TRT',
    'BCMPrime', 'PLR', 'ITkPixel', 'ITkStrip', 'HGTD',
    'LAr', 'Tile', 'MBTS',
    'CSC', 'MDT', 'RPC', 'TGC', 'sTGC', 'MM',
    'Lucid', 'ZDC', 'ALFA', 'AFP', 'FwdRegion',
    'Cavern',
]
# all detector groups - used in helper functions
allGroups = {
    'ID': ['BCM', 'Pixel', 'SCT', 'TRT'],
    'ITk': ['BCMPrime', 'ITkPixel', 'ITkStrip', 'PLR'],
    'Calo': ['LAr', 'Tile', 'MBTS'],
    'Muon': ['CSC', 'MDT', 'RPC', 'TGC', 'sTGC', 'MM'],
    'Forward': ['Lucid', 'ZDC', 'ALFA', 'AFP', 'FwdRegion'],
}


def createDetectorConfigFlags():
    dcf = AthConfigFlags()

    ## Detector.Geometry* flags represent the default full geometry,
    ## autoconfigured from the geometry tag
    dcf.addFlag('Detector.GeometryBpipe', True)  # always enabled by default

    # Inner Detector
    dcf.addFlag('Detector.GeometryBCM',   lambda prevFlags : 'BCM' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryPixel', lambda prevFlags : 'Pixel' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometrySCT',   lambda prevFlags : 'SCT' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryTRT',   lambda prevFlags : 'TRT' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryID',    lambda prevFlags : (prevFlags.Detector.GeometryBCM
                                                              or prevFlags.Detector.GeometryPixel or prevFlags.Detector.GeometrySCT
                                                              or prevFlags.Detector.GeometryTRT))

    # Upgrade ITk Inner Tracker is a separate and parallel detector
    dcf.addFlag('Detector.GeometryPLR',   lambda prevFlags : 'PLR' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryBCMPrime', lambda prevFlags : 'BCMPrime' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryITkPixel', lambda prevFlags : 'ITkPixel' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryITkStrip', lambda prevFlags : 'ITkStrip' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryITk',      lambda prevFlags : (prevFlags.Detector.GeometryBCMPrime
                                                                 or prevFlags.Detector.GeometryITkPixel
                                                                 or prevFlags.Detector.GeometryITkStrip
                                                                 or prevFlags.Detector.GeometryPLR))
    # HGTD
    dcf.addFlag('Detector.GeometryHGTD', lambda prevFlags : 'HGTD' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))

    # Calorimeters
    dcf.addFlag('Detector.GeometryLAr',  lambda prevFlags : 'LAr' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath)) # Add separate em HEC and FCAL flags?
    dcf.addFlag('Detector.GeometryTile', lambda prevFlags : 'Tile' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryMBTS', lambda prevFlags : (prevFlags.Detector.GeometryLAr and 'MBTS' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath)))
    dcf.addFlag('Detector.GeometryCalo', lambda prevFlags : (prevFlags.Detector.GeometryLAr or prevFlags.Detector.GeometryTile))

    # Muon Spectrometer
    dcf.addFlag('Detector.GeometryCSC',  lambda prevFlags : 'CSC' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryMDT',  lambda prevFlags : 'MDT' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryRPC',  lambda prevFlags : 'RPC' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryTGC',  lambda prevFlags : 'TGC' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometrysTGC', lambda prevFlags : 'sTGC' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryMM',   lambda prevFlags : 'MM' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB,prevFlags.GeoModel.SQLiteDBFullPath))
    dcf.addFlag('Detector.GeometryMuon', lambda prevFlags : (prevFlags.Detector.GeometryCSC or prevFlags.Detector.GeometryMDT
                                                             or prevFlags.Detector.GeometryRPC or prevFlags.Detector.GeometryTGC
                                                             or prevFlags.Detector.GeometrysTGC or prevFlags.Detector.GeometryMM))

    # Forward detectors (disabled by default)
    dcf.addFlag('Detector.GeometryLucid',     lambda prevFlags : 'Lucid' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB, prevFlags.GeoModel.SQLiteDBFullPath, includeForward=not prevFlags.Input.isMC))
    dcf.addFlag('Detector.GeometryZDC',       lambda prevFlags : 'ZDC' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB, prevFlags.GeoModel.SQLiteDBFullPath, includeForward=not prevFlags.Input.isMC))
    dcf.addFlag('Detector.GeometryALFA',      lambda prevFlags : 'ALFA' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB, prevFlags.GeoModel.SQLiteDBFullPath, includeForward=not prevFlags.Input.isMC))
    dcf.addFlag('Detector.GeometryAFP',       lambda prevFlags : 'AFP' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB, prevFlags.GeoModel.SQLiteDBFullPath, includeForward=not prevFlags.Input.isMC))
    dcf.addFlag('Detector.GeometryFwdRegion', lambda prevFlags : 'FwdRegion' in getDefaultDetectors(prevFlags.GeoModel.AtlasVersion,prevFlags.GeoModel.SQLiteDB, prevFlags.GeoModel.SQLiteDBFullPath, includeForward=not prevFlags.Input.isMC))
    dcf.addFlag('Detector.GeometryForward',   lambda prevFlags : (prevFlags.Detector.GeometryLucid or prevFlags.Detector.GeometryZDC
                                                                  or prevFlags.Detector.GeometryALFA or prevFlags.Detector.GeometryAFP
                                                                  or prevFlags.Detector.GeometryFwdRegion))

    # Cavern (disabled by default)
    dcf.addFlag('Detector.GeometryCavern', False)


    ## Detector.Enable* flags represent the default full geometry,
    ## autoconfigured from geometry flags
    # Inner Detector
    dcf.addFlag('Detector.EnableBCM',   lambda prevFlags : prevFlags.Detector.GeometryBCM)
    dcf.addFlag('Detector.EnablePixel', lambda prevFlags : prevFlags.Detector.GeometryPixel)
    dcf.addFlag('Detector.EnableSCT',   lambda prevFlags : prevFlags.Detector.GeometrySCT)
    dcf.addFlag('Detector.EnableTRT',   lambda prevFlags : prevFlags.Detector.GeometryTRT)
    dcf.addFlag('Detector.EnableID',    lambda prevFlags : prevFlags.Detector.GeometryID and
                                                           (prevFlags.Detector.EnableBCM
                                                            or prevFlags.Detector.EnablePixel or prevFlags.Detector.EnableSCT
                                                            or prevFlags.Detector.EnableTRT))

    # Upgrade ITk Inner Tracker is a separate and parallel detector
    dcf.addFlag('Detector.EnablePLR',   lambda prevFlags : prevFlags.Detector.GeometryPLR)
    dcf.addFlag('Detector.EnableBCMPrime', lambda prevFlags : prevFlags.Detector.GeometryBCMPrime)
    dcf.addFlag('Detector.EnableITkPixel', lambda prevFlags : prevFlags.Detector.GeometryITkPixel)
    dcf.addFlag('Detector.EnableITkStrip', lambda prevFlags : prevFlags.Detector.GeometryITkStrip)
    dcf.addFlag('Detector.EnableITk',      lambda prevFlags : prevFlags.Detector.GeometryITk and
                                                              (prevFlags.Detector.EnableBCMPrime
                                                               or prevFlags.Detector.EnableITkPixel
                                                               or prevFlags.Detector.EnableITkStrip
                                                               or prevFlags.Detector.EnablePLR))
    # HGTD
    dcf.addFlag('Detector.EnableHGTD', lambda prevFlags : prevFlags.Detector.GeometryHGTD)

    # Calorimeters
    dcf.addFlag('Detector.EnableLAr',    lambda prevFlags : prevFlags.Detector.GeometryLAr)
    dcf.addFlag('Detector.EnableTile',   lambda prevFlags : prevFlags.Detector.GeometryTile)
    dcf.addFlag('Detector.EnableMBTS',   lambda prevFlags : prevFlags.Detector.GeometryMBTS)
    dcf.addFlag('Detector.EnableL1Calo', lambda prevFlags : (prevFlags.Detector.EnableLAr or prevFlags.Detector.EnableTile))
    dcf.addFlag('Detector.EnableCalo',   lambda prevFlags : prevFlags.Detector.GeometryCalo and
                                                            (prevFlags.Detector.EnableLAr or prevFlags.Detector.EnableTile))

    # Muon Spectrometer
    dcf.addFlag('Detector.EnableCSC',  lambda prevFlags : prevFlags.Detector.GeometryCSC)
    dcf.addFlag('Detector.EnableMDT',  lambda prevFlags : prevFlags.Detector.GeometryMDT)
    dcf.addFlag('Detector.EnableRPC',  lambda prevFlags : prevFlags.Detector.GeometryRPC)
    dcf.addFlag('Detector.EnableTGC',  lambda prevFlags : prevFlags.Detector.GeometryTGC)
    dcf.addFlag('Detector.EnablesTGC', lambda prevFlags : prevFlags.Detector.GeometrysTGC)
    dcf.addFlag('Detector.EnableMM',   lambda prevFlags : prevFlags.Detector.GeometryMM)
    dcf.addFlag('Detector.EnableMuon', lambda prevFlags : prevFlags.Detector.GeometryMuon and
                                                          (prevFlags.Detector.EnableCSC or prevFlags.Detector.EnableMDT
                                                           or prevFlags.Detector.EnableRPC or prevFlags.Detector.EnableTGC
                                                           or prevFlags.Detector.EnablesTGC or prevFlags.Detector.EnableMM))

    # Forward detectors (disabled by default)
    dcf.addFlag('Detector.EnableLucid',     lambda prevFlags : prevFlags.Detector.GeometryLucid)
    dcf.addFlag('Detector.EnableZDC',       lambda prevFlags : prevFlags.Detector.GeometryZDC)
    dcf.addFlag('Detector.EnableALFA',      lambda prevFlags : prevFlags.Detector.GeometryALFA)
    dcf.addFlag('Detector.EnableAFP',       lambda prevFlags : prevFlags.Detector.GeometryAFP)
    dcf.addFlag('Detector.EnableFwdRegion', lambda prevFlags : prevFlags.Detector.GeometryFwdRegion)
    dcf.addFlag('Detector.EnableForward',   lambda prevFlags : prevFlags.Detector.GeometryForward and
                                                               (prevFlags.Detector.EnableLucid or prevFlags.Detector.EnableZDC
                                                                or prevFlags.Detector.EnableALFA or prevFlags.Detector.EnableAFP
                                                                or prevFlags.Detector.EnableFwdRegion))

    return dcf


def getEnabledDetectors(flags, geometry=False):
    # load flags
    flags._loadDynaFlags('Detector')
    values = []
    for d in allDetectors:
        # test whether each detector is on
        name = f'Detector.Geometry{d}' if geometry else f'Detector.Enable{d}'
        if flags.hasFlag(name):
            if flags(name) is not False:
                values.append(d)
    return values


def _parseDetectorsList(flags, detectors):
    # setup logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('DetectorConfigFlags')
    # load flags
    flags._loadDynaFlags('Detector')
    # compatibility
    ignored_detectors = ['None', 'Truth', 'DBM']
    def compatibility_layer(detector):
        if detector == 'pixel':
            return 'Pixel'
        return detector

    # first check if we have groups
    groups = {d for d in detectors if d in allGroups.keys()}
    detectors = {compatibility_layer(d) for d in detectors if d not in allGroups.keys()}
    for g in groups:
        log.debug("Evaluating detector group '%s'", g)
        if g == 'Forward':
            # forward detectors are special
            for d in allGroups[g]:
                log.debug("Appending detector '%s'", d)
                detectors.add(d)
        else:
            # in case of groups only enable defaults
            for d in allGroups[g]:
                if d in getDefaultDetectors(flags.GeoModel.AtlasVersion,flags.GeoModel.SQLiteDB,flags.GeoModel.SQLiteDBFullPath):
                    log.debug("Appending detector '%s'", d)
                    detectors.add(d)

    # check if valid detectors are required
    for d in detectors:
        if d not in allDetectors and d not in ignored_detectors:
            log.warning("Unknown detector '%s'", d)

    # sort the detectors
    return [d for d in allDetectors if d in detectors]


def _loadDetectorsFromMetaData(flags, key, keep_beampipe=False):
    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    detectors = eval(GetFileMD(flags.Input.Files).get(key, '[]'))
    if not detectors:
        return []

    # MBTS compatibility check
    from AthenaConfiguration.Enums import LHCPeriod
    if (flags.GeoModel.Run < LHCPeriod.Run4 and 'LAr' in detectors  # built as part of LAr
        or 'TileHitVector#MBTSHits' in flags.Input.TypedCollections
        or 'TileTTL1Container#TileTTL1MBTS' in flags.Input.TypedCollections):
        detectors.append('MBTS')

    if keep_beampipe and flags.Detector.GeometryBpipe:
        detectors.append('Bpipe')

    return _parseDetectorsList(flags, detectors)


def _getDetectorFlagStatus(flags, list=None, geometry=False):
    # load flags
    flags._loadDynaFlags('Detector')

    values = []
    for d in allDetectors:
        # test whether each detector is on
        name = f'Detector.Geometry{d}' if geometry else f'Detector.Enable{d}'
        if flags.hasFlag(name):
            if list is not None:
                if d in list:
                    values.append('ON')
                else:
                    values.append('--')
            else:
                if flags(name) is not False:
                    values.append('ON')
                else:
                    values.append('--')
        else:
            values.append('n/a')
    return values


def _printDetectorFlags(flags, info_dict):
    # setup logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('DetectorConfigFlags')
    # generate common formatting string
    item_len = 7
    # initial is always there
    types = ['Initial']
    subtypes = ['Geo.', 'Enbl.']
    format_header = f'%-{item_len * 2}s'
    if 'metadata_simulated' in info_dict.keys() or 'metadata_digitized' in info_dict.keys():
        metadata_len = 0
        if 'metadata_simulated' in info_dict.keys():
            subtypes.append('Sim.')
            metadata_len += item_len
        if 'metadata_digitized' in info_dict.keys():
            subtypes.append('Digi.')
            metadata_len += item_len
        types.append('Metadata' if item_len > 8 else 'Meta.')
        format_header += f'%-{metadata_len}s'
    if 'requested_geometry' in info_dict.keys():
        types.append('Requested')
        subtypes += ['Geo.', 'Enbl.']
        format_header += f'%-{item_len * 2}s'
    # final is always there ?
    types.append('Final')
    subtypes += ['Geo.', 'Enbl.']
    format_header += f'%-{item_len * 2}s'

    detector_len = 7
    for d in allDetectors:
        detector_len = max(detector_len, len(d) + 2)

    format_header = f'%{detector_len}s   ' + format_header
    format_subheader = f'%{detector_len}s   ' + len(subtypes) * f'%-{item_len}s'
    format = f'%{detector_len}s : ' + len(subtypes) * f'%-{item_len}s'
    data = [allDetectors] + [v for _, v in info_dict.items()]
    data = list(map(list, zip(*data)))

    # print header rows
    log.info(format_header, *([''] + types))
    log.info(format_subheader, *([''] + subtypes))
    # print data
    for row in data:
        log.info(format, *row)


def setupDetectorFlags(flags, custom_list=None, use_metadata=False, toggle_geometry=False, validate_only=False, keep_beampipe=False):
    """Setup detector flags from metadata or a list"""
    info_dict = {}
    # setup logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('DetectorConfigFlags')

    initial_set = set(getEnabledDetectors(flags))

    info_dict['initial_geometry'] = _getDetectorFlagStatus(flags, geometry=True)
    info_dict['initial_enable'] = _getDetectorFlagStatus(flags, geometry=False)

    final_set = initial_set
    has_metadata = False
    if use_metadata:
        simulated_set = set(_loadDetectorsFromMetaData(flags, 'SimulatedDetectors', keep_beampipe=keep_beampipe))
        digitized_set = set(_loadDetectorsFromMetaData(flags, 'DigitizedDetectors', keep_beampipe=keep_beampipe))
        if simulated_set:
            has_metadata = True
            info_dict['metadata_simulated'] = _getDetectorFlagStatus(flags, list=simulated_set, geometry=False)
        if digitized_set:
            has_metadata = True
            info_dict['metadata_digitized'] = _getDetectorFlagStatus(flags, list=digitized_set, geometry=False)

        if digitized_set and digitized_set != initial_set:
            final_set = digitized_set
        elif simulated_set and simulated_set != initial_set:
            final_set = simulated_set

    requested_inconsistency_set = set()
    if custom_list is not None and custom_list:
        requested_set = set(_parseDetectorsList(flags, custom_list))
        if toggle_geometry:
            info_dict['requested_geometry'] = _getDetectorFlagStatus(flags, list=requested_set, geometry=True)
        else:
            info_dict['requested_geometry'] = info_dict['initial_geometry']
        info_dict['requested_enable'] = _getDetectorFlagStatus(flags, list=requested_set, geometry=False)

        if has_metadata:
            requested_inconsistency_set = requested_set.difference(final_set)
            final_set = requested_set.intersection(final_set)
        else:
            final_set = requested_set

    info_dict['final_geometry'] = _getDetectorFlagStatus(flags, list=final_set, geometry=True)
    info_dict['final_enable'] = _getDetectorFlagStatus(flags, list=final_set, geometry=False)

    _printDetectorFlags(flags, info_dict)

    if requested_inconsistency_set:
        log.error('Requested detectors %s which are not enabled in the input file.', list(requested_inconsistency_set))
        raise ValueError(list(requested_inconsistency_set))

    if initial_set != final_set:
        diff_enable = final_set.difference(initial_set)
        if 'Bpipe' in diff_enable and (keep_beampipe or flags.Detector.GeometryBpipe):
            diff_enable.remove('Bpipe')
        diff_disable = initial_set.difference(final_set)
        if diff_enable:
            log.info('Enabling detectors: %s', _parseDetectorsList(flags, diff_enable))
        if diff_disable:
            log.info('Disabling detectors: %s', _parseDetectorsList(flags, diff_disable))
        setupDetectorsFromList(flags, final_set, toggle_geometry, validate_only)


def setupDetectorsFromList(flags, detectors, toggle_geometry=False, validate_only=False):
    """Setup (toggle) detectors from a pre-defined list"""
    changed = False
    # setup logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('DetectorConfigFlags')
    # parse the list
    detectors = _parseDetectorsList(flags, detectors)

    # print summary
    if validate_only:
        log.info('Required detectors: %s', detectors)
    else:
        log.debug('Setting detectors to: %s', detectors)

    # go through all of the detectors and check what should happen
    for d in allDetectors:
        status = d in detectors
        name = f'Detector.Enable{d}'
        if flags.hasFlag(name):
            if flags(name) != status:
                changed = True
                if validate_only:
                    log.warning("Flag '%s' should be %s but is set to %s", name, status, not status)
                else:
                    log.debug("Toggling '%s' from %s to %s", name, not status, status)
                    flags._set(name, status)
        if toggle_geometry:
            name = f'Detector.Geometry{d}'
            if flags.hasFlag(name):
                if flags(name) != status:
                    changed = True
                    if validate_only:
                        log.warning("Flag '%s' should be %s but is set to %s", name, status, not status)
                    else:
                        log.debug("Toggling '%s' from %s to %s", name, not status, status)
                        flags._set(name, status)

    return changed


def enableDetectors(flags, detectors, toggle_geometry=False):
    """Enable detectors from a list"""
    changed = False
    # setup logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('DetectorConfigFlags')
    # parse the list
    detectors = _parseDetectorsList(flags, detectors)

    # debugging
    log.info('Enabling detectors: %s', detectors)

    # go through all of the detectors and enable them if needed
    for d in detectors:
        name = f'Detector.Enable{d}'
        if flags.hasFlag(name):
            if flags(name) is not True:
                changed = True
                log.info("Enabling '%s'", name)
                flags._set(name, True)
        if toggle_geometry:
            name = f'Detector.Geometry{d}'
            if flags.hasFlag(name):
                if flags(name) is not True:
                    changed = True
                    log.info("Enabling '%s'", name)
                    flags._set(name, True)

    return changed


def disableDetectors(flags, detectors, toggle_geometry=False):
    """Disable detectors from a list"""
    changed = False
    # setup logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('DetectorConfigFlags')
    # parse the list
    detectors = _parseDetectorsList(flags, detectors)

    # debugging
    log.info('Disabling detectors: %s', detectors)

    # go through all of the detectors and disable them if needed
    for d in detectors:
        name = f'Detector.Enable{d}'
        if flags.hasFlag(name):
            if flags(name) is not False:
                changed = True
                log.info("Disabling '%s'", name)
                flags._set(name, False)
        if toggle_geometry:
            name = f'Detector.Geometry{d}'
            if flags.hasFlag(name):
                if flags(name) is not False:
                    changed = True
                    log.info("Disabling '%s'", name)
                    flags._set(name, False)

    return changed

