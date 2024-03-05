#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__all__ = ['metadata']

metadata = {}


def _setup():
    from PyUtils.MetaReader import read_metadata

    from AthenaCommon.Logging import logging
    msg = logging.getLogger('MetaReaderPeeker')

    global metadata

    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
    inFiles = athenaCommonFlags.FilesInput()

    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
    if athenaCommonFlags.isOnline() and (not inFiles or all([f.strip() == '' for f in inFiles])):
        # set minimal items of inputFileSummary
        metadata = {
          'file_type':'BS',
          'eventTypes':['IS_DATA','IS_ATLAS','IS_PHYSICS'],
          'TagStreamsRef':''
        }
    else:
        if len(inFiles) < 1:
            msg.warning("No input files specified yet! Cannot do anything.")
            return
            
            
        for inFile in inFiles:

            metadatas = read_metadata(inFile, mode='peeker', promote=True, ignoreNonExistingLocalFiles=True)
            
            for foundFile, metadata in metadatas.items():
                
                metadata['file_name'] = inFile
                if metadata.get("nentries"):
                    break
            else:
                continue # This make the previous break to exit from both for loops
            break
            
            # if no nentries > 0 metadata is found, it will keep the last

_setup()
