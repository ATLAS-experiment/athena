# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @file POOLtoEI_Skeleton.py
# @purpose Configure the POOLtoEI transformation using the ComponentAccumulator
# @author Javier Sanchez
# @date March 2024

from AthenaCommon import JobProperties
from PyJobTransforms.TransformUtils import (processPreExec, processPreInclude,
                                            processPostExec, processPostInclude)

# force no legacy job properties
JobProperties.jobPropertiesDisallowed = True


def configurePOOL2EIglobals(runArgs, flags):
    """ Configure POOL2EI global vars """

    class AttributeDict(dict):
        def __getattr__(self, attr):
            return self[attr]

        def __setattr__(self, attr, value):
            self[attr] = value

    job = AttributeDict()

    from AthenaCommon.Logging import logging
    eilog = logging.getLogger('pool_to_ei')

    # Input
    inputFileGiven = 0
    for filetype in ('POOL', 'AOD', 'ESD', 'EVNT', 'HITS', 'RDO', 'AOD_MRG'):
        if hasattr(runArgs, "input" + filetype + "File"):
            In = getattr(runArgs, "input" + filetype + "File")
            if type(In) is not list:
                In = [In]
            flags.Input.Files = In
            inputFileGiven += 1

    if inputFileGiven == 0:
        raise RuntimeError("No input file given")
    if inputFileGiven > 1:
        raise RuntimeError("Only one input file format is allowed")

    def getArgVal(args, key, defaultValue):
        if hasattr(args, key):
            return getattr(args, key)
        else:
            return defaultValue

    # Output
    import os
    job.Out = getArgVal(runArgs, "outputEIFile",
                        'pool2ei.{:08d}.spb'.format(os.getpid()))

    # options
    job.DoProvenanceRef = getArgVal(runArgs, "provenance", True)
    job.DoTriggerInfo = getArgVal(runArgs, 'trigger', True)
    job.SendToBroker = getArgVal(runArgs, "sendtobroker", False)
    job.EiDsName = getArgVal(runArgs, "eidsname", None)
    job.TestBrk = getArgVal(runArgs, "testbrk", False)
    job.EiFmt = getArgVal(runArgs, "eifmt", 0)

    # Tier0 job identification
    job.TaskID = getArgVal(runArgs, "_taskid", None)
    job.JobID = getArgVal(runArgs, "_jobid", None)
    job.AttemptNumber = getArgVal(runArgs, "_attempt", None)

    def findFirstKeyforValue(dict, value):
        """ Look for value in dict and return the first key or None if not found """
        k = next((k for k, v in dict.items() if v == value), None)
        return k

    # find key for 'EventStreamInfo'
    EventStreamInfo_key = findFirstKeyforValue(
        flags.Input.MetadataItems, 'EventStreamInfo')

    metadata_items = flags.Input.MetadataItems
    job.meta_hlt_hltconfigkeys = (
        '/TRIGGER/HLT/HltConfigKeys' in metadata_items)
    job.meta_hlt_prescalekey = ('/TRIGGER/HLT/PrescaleKey' in metadata_items)
    job.meta_lvl1_lvl1configkey = (
        '/TRIGGER/LVL1/Lvl1ConfigKey' in metadata_items)
    job.meta_hlt_menu = ('/TRIGGER/HLT/Menu' in metadata_items)
    job.meta_lvl1_menu = ('/TRIGGER/LVL1/Menu' in metadata_items)
    job.meta_triggermenu = ('TriggerMenu' in metadata_items)
    job.meta_triggermenujson_hlt = ('TriggerMenuJson_HLT' in metadata_items)
    job.meta_triggermenujson_l1 = ('TriggerMenuJson_L1' in metadata_items)

    eilog.info("meta_hlt_hltconfigkeys: {}".format(job.meta_hlt_hltconfigkeys))
    eilog.info("meta_hlt_prescalekey: {}".format(job.meta_hlt_prescalekey))
    eilog.info("meta_lvl1_lvl1configkey: {}".format(
        job.meta_lvl1_lvl1configkey))
    eilog.info("meta_hlt_menu: {}".format(job.meta_hlt_menu))
    eilog.info("meta_lvl1_menu: {}".format(job.meta_lvl1_menu))
    eilog.info("meta_triggermenu: {}".format(job.meta_triggermenu))
    eilog.info("meta_triggermenujson_hlt: {}".format(
        job.meta_triggermenujson_hlt))
    eilog.info("meta_triggermenujson_l1: {}".format(
        job.meta_triggermenujson_l1))

    collections = flags.Input.TypedCollections
    job.item_eventinfo = ('EventInfo#EventInfo' in collections
                          or 'EventInfo#McEventInfo' in collections)
    job.item_eventinfoBS = ('EventInfo#ByteStreamEventInfo' in collections)
    job.item_xaod_eventinfo = ('xAOD::EventInfo#EventInfo' in collections)
    job.item_xaod_TrigConfKeys = (
        'xAOD::TrigConfKeys#TrigConfKeys' in collections)
    job.item_xaod_TrigDecision = (
        'xAOD::TrigDecision#xTrigDecision' in collections)

    if not (job.item_eventinfo or job.item_xaod_eventinfo or job.item_eventinfoBS):
        # EventInfo should exist
        job.item_eventinfo = True

    eilog.info("item_eventinfo: {}".format(job.item_eventinfo))
    eilog.info("item_eventinfoBS: {}".format(job.item_eventinfoBS))
    eilog.info("item_xaod_eventinfo: {}".format(job.item_xaod_eventinfo))
    eilog.info("item_xaod_TrigConfKeys: {}".format(job.item_xaod_TrigConfKeys))
    eilog.info("item_xaod_TrigDecision: {}".format(job.item_xaod_TrigDecision))

    job.projectName = flags.Input.ProjectName

    # if EVNT, disable trigger processing
    if job.DoTriggerInfo:
        try:
            if (EventStreamInfo_key is not None and ('StreamEVGEN' in flags.Input.ProcessingTags or 'StreamEVNT' in flags.Input.ProcessingTags)):
                eilog.info("Disable trigger processing for EVNT files")
                job.DoTriggerInfo = False

            if (EventStreamInfo_key is not None and flags.Input.isMC):
                if not (job.meta_hlt_hltconfigkeys or job.item_xaod_TrigConfKeys):
                    eilog.info("Disable trigger processing for MC files "
                               "with no trigger inside")
                    job.DoTriggerInfo = False
        except Exception:
            pass

    return job


def fromRunArgs(runArgs):
    """ Configure and run the transformation """

    from AthenaCommon.Logging import logging
    from AthenaCommon.Constants import INFO

    eilog = logging.getLogger('pool_to_ei')
    eilog.info('****************** STARTING POOL->EI MAKING *****************')

    # configure flags

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    commonRunArgsToFlags(runArgs, flags)

    # process pre-include/exec
    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    # To respect --athenaopts
    flags.fillFromArgs()

    # configure job globals form runArgs and flags
    job = configurePOOL2EIglobals(runArgs, flags)

    # Lock flags
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    cfg.merge(MetaDataSvcCfg(flags))

    # Load these objects from StoreGate
    loadFromSG = []
    if job.item_eventinfo:
        if flags.Input.isMC:
            loadFromSG.append(('EventInfo', 'StoreGateSvc+McEventInfo'))
        else:
            loadFromSG.append(('EventInfo', 'StoreGateSvc+EventInfo'))
    if job.item_eventinfoBS:
        loadFromSG.append(('EventInfo', 'StoreGateSvc+ByteStreamEventInfo'))
    if job.item_xaod_eventinfo:
        loadFromSG.append(('xAOD::EventInfo', 'StoreGateSvc+EventInfo'))
    if job.item_xaod_TrigConfKeys:
        loadFromSG.append(('xAOD::TrigConfKeys', 'StoreGateSvc+TrigConfKeys'))
    if job.item_xaod_TrigDecision:
        loadFromSG.append(('xAOD::TrigDecision', 'StoreGateSvc+xTrigDecision'))

    from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
    cfg.merge(SGInputLoaderCfg(flags, loadFromSG))

    if job.item_eventinfoBS:
        # instruct POOL2EI to read EventInfo
        job.item_eventinfo = True

    # algorithm
    from EventIndexProducer.POOL2EI_Lib import POOL2EI
    pool2ei = POOL2EI('pool2ei', OutputLevel=INFO, **job)
    cfg.addEventAlgo(pool2ei)

    # service
    from EventIndexProducer.POOL2EI_Lib import POOL2EISvc
    pool2eisvc = POOL2EISvc(algo=pool2ei)
    cfg.addService(pool2eisvc, create=True)

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    if flags.Exec.OutputLevel <= INFO:
        cfg.printConfig()

    # Run the final accumulator
    sc = cfg.run()

    import sys
    sys.exit(sc.isFailure())
