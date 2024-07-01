# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger('IOVDbAutoCfgFlags')

def getLastGlobalTag(prevFlags):
    if not prevFlags.Input.Files:
        return ""

    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    globaltag = GetFileMD(prevFlags.Input.Files).get("IOVDbGlobalTag", None)
    if globaltag is None:
        return ""

    if isinstance(globaltag, list):  # if different tags have been used at different steps
        globaltag = globaltag[-1]

    return globaltag


def getDatabaseInstanceDefault(flags):
    # MC
    if flags.Input.isMC:
        return "OFLP200"

    # real-data
    try:
        year = int(flags.Input.ProjectName[4:6])
    except Exception:
        log.warning("Failed to extract year from project tag %s. Assuming CONDBR2.", flags.Input.ProjectName)
        return "CONDBR2"

    if year > 13:
        return "CONDBR2"
    else:
        return "COMP200"
