# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Creation: Karsten Koeneke
def GetCurrentStreamName( msg ):
    """ Helper to decide where to get the input stream name from."""
    from AthenaCommon.AppMgr import ServiceMgr as svcMgr
    try:
        input_files = svcMgr.EventSelector.InputCollections
    except AttributeError:
        from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
        input_files = athenaCommonFlags.FilesInput()

    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    processingTags = GetFileMD(input_files).get("processingTags", [])
    return processingTags[-1] if processingTags else 'N/A'


def GetCurrentSkimmingCycle( msg ):
    from AthenaCommon.AppMgr import ServiceMgr as svcMgr
    try:
        input_files = svcMgr.EventSelector.InputCollections
    except AttributeError:
        from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
        input_files = athenaCommonFlags.FilesInput()

    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    return GetFileMD(input_files).get('currentCutCycle', -1) + 1


def CreateCutFlowSvc( seq=None, addMetaDataToAllOutputFiles=True ):
    """
    Helper to create the CutFlowSvc, extract the needed information from
    the input file, and also schedule all the needed stuff.
    """
    # Create a message logger
    from AthenaCommon.Logging import logging
    msg = logging.getLogger( "CreateCutFlowSvc" )

    # Get the service manager
    from AthenaCommon.AppMgr import ServiceMgr as svcMgr

    # Determine current input stream name
    inputStreamName = GetCurrentStreamName( msg=msg )
    msg.debug("CreateCutFlowSvc: Have inputStreamName = %s", inputStreamName)

    skimmingCycle = GetCurrentSkimmingCycle( msg=msg )
    msg.debug("CreateCutFlowSvc: Have skimmingCycle = %s", skimmingCycle)

    # Create the CutFlowSvc instance
    import AthenaCommon.CfgMgr as CfgMgr
    if not hasattr(svcMgr,"CutFlowSvc"):
        cutFlowSvc = CfgMgr.CutFlowSvc()
        svcMgr += cutFlowSvc
    else:
        cutFlowSvc = svcMgr.CutFlowSvc
    cutFlowSvc.Configured  = True
    cutFlowSvc.InputStream = inputStreamName
    cutFlowSvc.SkimmingCycle = skimmingCycle

    # Make sure MetaDataSvc is ready
    if not hasattr(svcMgr,'MetaDataSvc'):
      from AthenaServices.AthenaServicesConf import MetaDataSvc
      svcMgr += MetaDataSvc( "MetaDataSvc" )

    # Add BookkeeperTools
    from EventBookkeeperTools.EventBookkeeperToolsConf import BookkeeperTool

    # Standard event bookkeepers
    output_name = "CutBookkeepers"
    cutflowtool = BookkeeperTool("BookkeeperTool",
                                 InputCollName = output_name,
                                 OutputCollName= output_name)
    svcMgr.ToolSvc += cutflowtool

    # Add tool to MetaDataSvc
    svcMgr.MetaDataSvc.MetaDataTools += [cutflowtool]

    # Check if we have a sequence given
    if not seq :
        # Fetch the AthAlgSeq, i.e., one of the existing master sequences where one should attach all algorithms
        seq = CfgMgr.AthSequencer("AthAlgSeq")
        pass

    # First of all, schedule AllExecutedEventsCounterAlg
    if not hasattr(seq,"AllExecutedEvents"):
        if not seq.isLocked():
            # Need to schedule it after the xAODMaker::EventInfoCnvAlg such that xAOD::EventInfo is present
            index = 0
            if hasattr( seq, "xAODMaker::EventInfoCnvAlg" ):
                for alg in seq:
                    index += 1
                    if alg.getName() == "xAODMaker::EventInfoCnvAlg": break
                    pass
                pass
            msg.debug("Adding AllExecutedEventsCounterAlg with name AllExecutedEvents to sequence with name %s at position %i", seq.getName(), index)
            seq.insert( index, CfgMgr.AllExecutedEventsCounterAlg("AllExecutedEvents") )
            pass
        else :
            msg.info("Could NOT add AllExecutedEventsCounterAlg with name AllExecutedEvents to locked sequence with name %s", seq.getName())
            pass
        pass

    # If wanted, add the meta-data to all output files
    if addMetaDataToAllOutputFiles:
        msg.debug("Adding CutBookkeepers the the output meta data of all output streams")
        from OutputStreamAthenaPool.MultipleStreamManager import MSMgr
        # Explicitely add file metadata from input and from transient store,
        # but only the ones that we always create.
        MSMgr.AddMetaDataItemToAllStreams( "xAOD::CutBookkeeperContainer#"+output_name+"*" )
        MSMgr.AddMetaDataItemToAllStreams( "xAOD::CutBookkeeperAuxContainer#"+output_name+"*Aux.*" )
        MSMgr.AddMetaDataItemToAllStreams( "xAOD::CutBookkeeperContainer#Incomplete"+output_name+"*" )
        MSMgr.AddMetaDataItemToAllStreams( "xAOD::CutBookkeeperAuxContainer#Incomplete"+output_name+"*Aux.*" )
        pass

    return

def CreateBookkeeperTool( name="BookkeeperTool" ):

  from AthenaCommon.AppMgr  import ServiceMgr as svcMgr

  # Make sure MetaDataSvc is ready
  if not hasattr(svcMgr,'MetaDataSvc'):
    from AthenaServices.AthenaServicesConf import MetaDataSvc
    svcMgr += MetaDataSvc( "MetaDataSvc" )

  # Add BookkeeperTools
  from EventBookkeeperTools.EventBookkeeperToolsConf import BookkeeperTool

  # Standard event bookkeepers
  cutflowtool = BookkeeperTool(name,
                               InputCollName=name,
                               OutputCollName = name)
  svcMgr.ToolSvc += cutflowtool

  # Add tool to MetaDataSvc
  #svcMgr.MetaDataSvc.MetaDataTools += [cutflowtool]

  return

def CreateBookkeeperDumperTool(name='BookkeeperDumperTool'):
    from AthenaCommon.AppMgr import ServiceMgr as svcMgr

    # Make sure MetaDataSvc is ready
    if not hasattr(svcMgr, 'MetaDataSvc'):
        from AthenaServices.AthenaServicesConf import MetaDataSvc
        svcMgr += MetaDataSvc('MetaDataSvc')

    # Add BookkeeperDumperTool
    from EventBookkeeperTools.EventBookkeeperToolsConf import BookkeeperDumperTool
    tool = BookkeeperDumperTool(name)
    svcMgr.ToolSvc += tool

    # Add tool to MetaDataSvc
    svcMgr.MetaDataSvc.MetaDataTools += [tool]
