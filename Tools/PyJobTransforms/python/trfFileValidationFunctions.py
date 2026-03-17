# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

## @package PyJobTransforms.trfFileValidationFunctions
# @brief Transform file validation functions
# @author atlas-comp-transforms-dev@cern.ch
# @version $Id: trfUtils.py 578615 2014-01-15 21:22:05Z wbreaden $

import logging
msg = logging.getLogger(__name__)

import PyJobTransforms.trfExceptions as trfExceptions

## @brief Integrity function for file class argPOOLFile, argHITSFile, argRDOFile and argEVNTFile
def returnIntegrityOfPOOLFile(fname, **kwargs):
    from PyJobTransforms.trfValidateRootFile import checkFile, msg as logger
    import multiprocessing

    level = kwargs.get('level')
    if level is not None:
        if level < msg.getEffectiveLevel():
            msg.setLevel(level)
            msg.debug(f"Set logging level of {msg.name!r} to {logging.getLevelName(level)!r}")
        if level < logger.getEffectiveLevel():
            logger.setLevel(level)
            msg.debug(f"Set logging level of {logger.name!r} to {logging.getLevelName(level)!r}")

    msg.debug(f"Current process: {multiprocessing.current_process().name}")

    rc = checkFile(fileName=fname, the_type='event', requireTree=False)
    if rc == 0:
        return (True, "integrity of {fileName} good".format(fileName = str(fname)))
    else:
        return (False, "integrity of {fileName} bad: return code: {integrityStatus}".format(fileName = str(fname), integrityStatus = rc))

## @brief Integrity function for file class argNTUPFile
def returnIntegrityOfNTUPFile(fname):
    from PyJobTransforms.trfValidateRootFile import checkFile
    rc = checkFile(fileName = fname, the_type = 'basket', requireTree = False)
    if rc == 0:
        return (True, "integrity of {fileName} good".format(fileName = str(fname)))
    else:
        return (False, "integrity of {fileName} bad: return code: {integrityStatus}".format(fileName = str(fname), integrityStatus = rc))

## @brief Integrity function for file class argBSFile
def returnIntegrityOfBSFile(fname):
    try:
        from PyJobTransforms.trfUtils import call
        rc = call(["AtlListBSEvents", "-c", fname],
            logger = msg,
            message = "Report by AtlListBSEvents: ",
            timeout = None
        )
    except trfExceptions.TransformTimeoutException:
        return False
    if rc == 0:
        return (True, "integrity of {fileName} good".format(fileName = str(fname)))
    else:
        return (False, "integrity of {fileName} bad: return code: {integrityStatus}".format(fileName = str(fname), integrityStatus = rc))

## @brief Integrity function for file class argHISTFile
def returnIntegrityOfHISTFile(fname):
    rc = 0 # (default behaviour)
    if rc == 0:
        return (True, "integrity of {fileName} good".format(fileName = str(fname)))
    else:
        return (False, "integrity of {fileName} bad: return code: {integrityStatus}".format(fileName = str(fname), integrityStatus = rc))
