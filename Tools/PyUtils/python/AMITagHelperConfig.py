# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""Utilities to get and set the AMITag in the metadata

The input AMITag is contained in the AMITag attribute of the /TagInfo in-file
metadata object.

The AMITag of the current processing step is provided by command line argument
to the transform.

The tool provides a function to get and check the AMITags from the input and
add the output AMITag, configuring the TagInfoMgr with the information.

    Usage:
        from PyUtils.AMITagHelperConfig import AMITagCfg
        cfg.merge(AMITagCfg(ConfigFlags, runArgs))
"""
from AthenaCommon.Logging import logging
log = logging.getLogger('AMITagHelper')

import re
amitagRegex = re.compile('^[a-z][0-9]+')


def inputAMITagsFromEnvironment(silent=False):
    tagsFromINDS = []
    # from INDS environmental variable
    from os import environ
    varINDS = environ.get('INDS', '')
    if varINDS:
        # try extracting the AMITag of the input dataset from its name
        tagsFromINDS = varINDS.split('.')[-1].split('_')
        tagsFromINDS = [tag for tag in tagsFromINDS if amitagRegex.match(tag)]
        if not silent:
            log.debug('AMITag from input dataset name: {}'.format(tagsFromINDS))
    return tagsFromINDS


def inputAMITags(flags, fixBroken=False, silent=False):
    """Returns AMITag of input, split in a list, e.g. ['e6337','s3681','r13145']

    Looks up AMITag of input from ConfigFlag input file's in-file
    metadta. If nothing can be retrieved from the in-file metadata return empty
    list, otherwise return value from metadata. Inform about differences.
    """
    if flags.Input.SecondaryFiles and not flags.Overlay.DataOverlay:
        files = flags.Input.SecondaryFiles
    else:
        files = flags.Input.Files

    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    tags = GetFileMD(files).get('AMITag', '')
    tagsFromINDS = inputAMITagsFromEnvironment(silent)

    if tags:
        tags = tags.split('_')
        # remove duplicates
        from itertools import groupby
        tags = [key for key, _group in groupby(tags)]
        if not silent:
            log.info(f'Read AMITag from metadata: {tags}')
    else:
        log.info("Cannot access /TagInfo/AMITag from in-file metadata")
        # Set it to an empty list or the value from INDS
        return tagsFromINDS

    if tagsFromINDS and tags != tagsFromINDS:
        if not silent:
            log.warning("AMITag mismatch, check metadata of input dataset")
        if fixBroken:
            if not silent:
                log.warning(f"Will change {tags} to {tagsFromINDS} based on INDS environment variable")
            tags = tagsFromINDS

    return tags


def AMITagCfg(flags, runArgs=None, fixBroken=False):
    """Add input and output AMITag values and set result in in-file metadata

    The AMITags will be combined with '_' as delimiters. The result is set in
    the in-file metadata.
    """
    tags = inputAMITags(flags, fixBroken)
    if runArgs is not None:
        if hasattr(runArgs, 'AMITag') and runArgs.AMITag not in tags:
            tags += [runArgs.AMITag]
            log.info(f'Adding AMITag from execution: {runArgs.AMITag}')
        elif hasattr(runArgs, 'AMIConfig') and runArgs.AMIConfig not in tags:
            tags += [runArgs.AMIConfig]
            log.info(f'Adding AMITag from execution: {runArgs.AMIConfig}')

    if not tags:
        from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
        return ComponentAccumulator()

    valueAMITag = '_'.join(tags)

    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    return TagInfoMgrCfg(flags, tagValuePairs={'AMITag': valueAMITag})
