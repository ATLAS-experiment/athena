# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import logging
msg = logging.getLogger(__name__)

def executeFromFragment(fragment_string, flags, cfg=None):
    """Execute a function from a pre/post include fragment."""
    # detect legacy job options in the transition period:
    if fragment_string.endswith('.py'):
        msg.warning(f'Trying to load legacy job options {fragment_string}. This should NOT be used in production!')
        fragment_string = fragment_string[:-3]
        fragment_string = fragment_string.replace('/', '.')
        msg.warning(f'Resolved to {fragment_string}')

    parts = fragment_string.split('.')
    if len(parts) < 2:
        raise ValueError('Pre/post include should be of the form Package.Module.Function or Package.Function if defined in __init__.py')

    function = parts[-1]
    module = '.'.join(parts[:-1])

    from importlib import import_module
    loaded_module = import_module(module)
    function_def = getattr(loaded_module, function)

    if not cfg:
        function_def(flags)
        return

    from inspect import getfullargspec
    argspec = getfullargspec(function_def)
    print(argspec)
    argCount = len(argspec.args)
    defaultsCount = len(argspec.defaults) if argspec.defaults else 0
    if argCount - defaultsCount == 1:
        cfg.merge(function_def(flags))
    else:
        function_def(flags, cfg)


def processPreExec(runArgs, flags):
    """Process preExec from runtime arguments."""
    if hasattr(runArgs, 'preExec') and runArgs.preExec and runArgs.preExec != 'NONE':
        ConfigFlags = flags  # noqa: F841
        from AthenaConfiguration.Enums import HIMode,BeamType  # noqa: F401
        for cmd in runArgs.preExec:
            exec(cmd)


def processPostExec(runArgs, flags, cfg):
    """Process postExec from runtime arguments."""
    if not flags.locked():
        raise RuntimeError('Running a postExec before locking ConfigFlags')

    if hasattr(runArgs, 'postExec') and runArgs.postExec and runArgs.postExec != 'NONE':
        ConfigFlags = flags  # noqa: F841
        from AthenaConfiguration.ComponentFactory import CompFactory # noqa: F401
        for cmd in runArgs.postExec:
            exec(cmd)


def processPreInclude(runArgs, flags):
    """Process preInclude from runtime arguments."""
    if hasattr(runArgs, 'preInclude') and runArgs.preInclude and runArgs.preInclude != 'NONE':
        for fragment in runArgs.preInclude:
            executeFromFragment(fragment, flags)


def processPostInclude(runArgs, flags, cfg):
    """Process postInclude from runtime arguments."""
    if not flags.locked():
        raise RuntimeError('Running a postInclude before locking ConfigFlags')

    if hasattr(runArgs, 'postInclude') and runArgs.postInclude and runArgs.postInclude != 'NONE':
        for fragment in runArgs.postInclude:
            executeFromFragment(fragment, flags, cfg)


def UseFrontier(flags):
    """Switch database to using FRONTIER, but with a fallback
    to DBRelease if FRONTIER_SERVER is undefined (e.g., on HPC)

    Move from RecJobTransforms to PyJobTransforms to enable use
    in simulation ATN and KV jobs
    """
    from os import environ
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

    if flags.IOVDb.UseCREST:
        raise RuntimeError('Using PyJobTransforms.UseCREST (setting flags.IOVDb.UseCREST = True) and PyJobTransforms.UseFrontier in the same job is not supported!')

    cfg = ComponentAccumulator()
    if environ.get('FRONTIER_SERVER'):
        msg.info('Enabling FRONTIER DB access')
        from IOVDbSvc.IOVDbSvcConfig import DBReplicaSvcCfg
        cfg.merge(DBReplicaSvcCfg(flags, vetoDBRelease=True))
    else:
        msg.info('Using default DB access')

    return cfg


def UseCREST(flags):
    """PreInclude to switch to using CREST rather than COOL
    """
    flags.IOVDb.UseCREST = True
    from os import environ
    msg.info('Enabling CREST DB access')
    if environ.get('CREST_SERVER'):
        flags.IOVDb.CrestServer = environ.get('CREST_SERVER')
    else:
        msg.info('CREST_SERVER environment variable not defined - using fall-back.')
    msg.info(f'Using CrestServer: {flags.IOVDb.CrestServer}')


def DumpPickle(flags, cfg):
    """Dump the pickle file for the current configuration"""
    with open("Configuration.pkl", "wb") as f:
        cfg.store(f)


def SortInput(flags, cfg):
    """Method that should be post-included after a typicaly configured Athena job
    It will read original input files, create a list of Events in memory, sort them and produce
    an intermediate Event Collection file that Athena will read instead of the original inputs
    Event information is read from EventInfoTags (stored by default in all Athena data files)
    The default sort key value (Lumi) can be oveerriden, as the sorting order
    The intermediate Collection file can be inspected using CollQuery cmdline utility"""
    import os
    inputs = cfg.getService("EventSelector").InputCollections

    # set default sort parameters, read overrides from locals()
    tmpCollFile = locals().get("AthenaInputSortCollName", "sortedEventRefs" + str(os.getpid()) )
    sortTag     = locals().get("AthenaInputSortTag",      "LumiBlockN")
    sortOrd     = locals().get("AthenaInputSortOrder",    "Ascending")

    from CollectionUtilities.SortedCollectionCreator import SortedCollectionCreator
    sorter = SortedCollectionCreator(name="SortEvents")
    # Sort Inputs based on one of the EventInfoTag attributes
    # Store sorted event collection in a temporary file
    # This should run as postInclude, so we assume EventSelector.InputCollections is set earlier

    # moved execution to a subprocess, because Gaudi messaging created by collections causes
    # AppManager errors
    rc = sorter.executeInSubprocess(inputs, outputCollection=tmpCollFile, sortAttribute=sortTag, sortOrder=sortOrd)
    if rc != 0:
       msg.error(f"Sorting failed with exit code: {rc}")

    # Reading Events through References require a populated FileCatalog
    for inpfile in inputs:
        os.system('pool_insertFileToCatalog {}'.format(inpfile))

    # Tell Athena to use the sorted collection instead of the original inputs
    cfg.getService("EventSelector").InputCollections = [tmpCollFile + ".root"]
    cfg.getService("EventSelector").CollectionType = "RootCollection"
    return cfg
