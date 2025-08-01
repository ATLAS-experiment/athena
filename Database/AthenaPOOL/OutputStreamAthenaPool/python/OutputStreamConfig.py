# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, ConfigurationError
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from AthenaCommon.Logging import logging


def outputStreamName(streamName):
   return f"Stream{streamName}"


# keepProvenanceTagsRegEx - RexEx string to match processing tags in the Event provenance. Only matching tags will be copied
#                           to the new DataHeader. Empty string rejects all tags. Direct provenance is not affected
#                           (see extendProvenanceRecord)
def OutputStreamCfg(flags, streamName, ItemList=[], MetadataItemList=[],
                    disableEventTag=False, trigNavThinningSvc=None, takeItemsFromInput=False,
                    extendProvenanceRecord=True, keepProvenanceTagsRegEx=None, AcceptAlgs=[], HelperTools=[]):
   eventInfoKey = "EventInfo"
   if flags.Common.ProductionStep in [ProductionStep.PileUpPresampling, ProductionStep.PileUpPretracking, ProductionStep.MinbiasPreprocessing]:
      eventInfoKey = f"{flags.Overlay.BkgPrefix}EventInfo"

   msg = logging.getLogger("OutputStreamCfg")
   flagName = f"Output.{streamName}FileName"
   if flags.hasFlag(flagName):
      fileName = flags._get(flagName)
   else:
      fileName = f"my{streamName}.pool.root"
      msg.info("No file name predefined for stream %s. Using %s", streamName, fileName)

   if fileName in flags.Input.Files:
      raise ConfigurationError("Same name for input and output file %s" % fileName)

   result = ComponentAccumulator(sequence = CompFactory.AthSequencer("AthOutSeq", StopOverride=True))

   # Set up AthenaPoolCnvSvc through PoolWriteCfg
   from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
   result.merge(PoolWriteCfg(flags))

   # define athena output stream
   writingTool = CompFactory.AthenaOutputStreamTool(
      f"{outputStreamName(streamName)}Tool",
      DataHeaderKey=outputStreamName(streamName),
      MetaDataPoolContainerPrefix=f"{flags.Output.StorageTechnology.MetaData}:MetaData",
      MetaDataOutputCollection=f"{flags.Output.StorageTechnology.MetaData}:MetaDataHdr",
      ConversionService="AthenaPoolSharedIOCnvSvc" if flags.MP.UseSharedReader or flags.MP.UseSharedWriter else "AthenaPoolCnvSvc",
   )

   # If we're running in augmentation mode, configure the writing tool accordingly
   parentStream = f"Output.{streamName}ParentStream"
   childStream = f"Output.{streamName}ChildStream"
   if flags.hasFlag(childStream):
      writingTool.SaveDecisions = True
   elif flags.hasFlag(parentStream):
      disableEventTag = True
      writingTool.OutputCollection = f"POOLContainer_{streamName}"
      writingTool.PoolContainerPrefix = f"CollectionTree_{streamName}"
      writingTool.MetaDataOutputCollection = f"MetaDataHdr_{streamName}"
      writingTool.MetaDataPoolContainerPrefix = f"MetaData_{streamName}"
      msg.info("Stream %s running in augmentation mode with %s as parent", streamName, flags._get(parentStream))

   # In DAOD production the EventInfo is prepared specially by the SlimmingHelper to ensure it is written in AuxDyn form
   # So for derivations the ItemList from the SlimmingHelper alone is used without the extra EventInfo items
   finalItemList = []
   if any(name in streamName for name in {"DAOD_", "D2AOD_"}):
      finalItemList = ItemList
   else:
      finalItemList = [f"xAOD::EventInfo#{eventInfoKey}", f"xAOD::EventAuxInfo#{eventInfoKey}Aux."] + ItemList 

   outputStream = CompFactory.AthenaOutputStream(
      outputStreamName(streamName),
      StreamName=outputStreamName(streamName),
      WritingTool=writingTool,
      ItemList=finalItemList,
      MetadataItemList=MetadataItemList,
      OutputFile=fileName,
      HelperTools=HelperTools,
   )
   if takeItemsFromInput:
      # Avoid explicitly setting the property unless we want to set it
      # to True (default in C++ is False). This avoids CA merge
      # conflicts.
      outputStream.TakeItemsFromInput = True
   if not extendProvenanceRecord:
       # Treat this similar to takeItemsFromInput
       # (C++ default in this case is True)
       outputStream.ExtendProvenanceRecord = False
   if keepProvenanceTagsRegEx is not None:
      # C++ defaults to '.*' which means all. Overwrite only on request.
      outputStream.KeepProvenanceTagsRegEx = keepProvenanceTagsRegEx
   outputStream.AcceptAlgs += AcceptAlgs
   outputStream.ExtraOutputs.add(("DataHeader", f"StoreGateSvc+{outputStreamName(streamName)}"))
   if flags.Scheduler.CheckOutputUsage and flags.Concurrency.NumThreads > 0:
      outputStream.ExtraInputs = {tuple(l.split('#')) for l in finalItemList if '*' not in l and 'Aux' not in l}
      # Ignore dependencies
      from AthenaConfiguration.MainServicesConfig import OutputUsageIgnoreCfg
      result.merge(OutputUsageIgnoreCfg(flags, outputStream.name))

   result.addService(CompFactory.StoreGateSvc("MetaDataStore"))
   outputStream.MetadataStore = result.getService("MetaDataStore")

   # Support for MT thinning.
   thinningCacheTool = CompFactory.Athena.ThinningCacheTool(f"ThinningCacheTool_Stream{streamName}",
                                                            StreamName=outputStreamName(streamName))
   if trigNavThinningSvc is not None:
      thinningCacheTool.TrigNavigationThinningSvc = trigNavThinningSvc
   outputStream.HelperTools.append(thinningCacheTool)

   # Event Tag
   if not disableEventTag:
      key = "SimpleTag"
      outputStream.WritingTool.AttributeListKey=key

      propagateInputAttributeList = False
      if "AthenaAttributeList#Input" in flags.Input.TypedCollections:
         from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
         result.merge(SGInputLoaderCfg(flags, ["AthenaAttributeList#Input"]))
         propagateInputAttributeList = True

      # build eventinfo attribute list
      tagBuilder = CompFactory.EventInfoTagBuilder(AttributeList=key,
                                                   Tool=CompFactory.EventInfoAttListTool(),
                                                   EventInfoKey=eventInfoKey,
                                                   PropagateInput=propagateInputAttributeList)
      result.addEventAlgo(tagBuilder)

   # For xAOD output
   if "AOD" in streamName:
      outputStream.WritingTool.SubLevelBranchName = "<key>"

   result.addEventAlgo(outputStream, domain='IO')
   return result


def addToESD(flags, itemOrList, **kwargs):
   """
   Adds items to ESD stream

   The argument can be either list of items or just one item
   if further customisations are needed for output stream they can be passed via kwargs

   returns CA to be merged i.e.: result.merge(addToESD(flags, "xAOD::CoolObject"))
   """
   if not flags.Output.doWriteESD:
      return ComponentAccumulator()
   items = [itemOrList] if isinstance(itemOrList, str) else itemOrList
   return OutputStreamCfg(flags, "ESD", ItemList=items, **kwargs)


def addToAOD(flags, itemOrList, **kwargs):
   """
   Adds items to AOD stream

   @see add addToESD
   """
   if not flags.Output.doWriteAOD:
      return ComponentAccumulator()
   items = [itemOrList] if isinstance(itemOrList, str) else itemOrList
   return OutputStreamCfg(flags, "AOD", ItemList=items, **kwargs)

def addToMetaData(flags, streamName, itemOrList, AcceptAlgs=[], HelperTools=[], **kwargs):
   """
   Adds Metadata items to the stream named streamName

   Similar to addToESD/AOD, itemOrList can be either a list of items or just one time
   The additional arguments, AcceptAlgs and HelperTools, are passed to the underlying stream
   The former is needed when there are special kernels, e.g., simulation/derivation
   The latter is needed primarily for the propagation of the FileMetaData tool

   Returns CA to be merged
   """
   flagName = f"Output.doWrite{streamName}"
   if not flags.hasFlag(flagName):
       return ComponentAccumulator()
   items = [itemOrList] if isinstance(itemOrList, str) else itemOrList
   return OutputStreamCfg(flags, streamName, MetadataItemList=items,
                          AcceptAlgs=AcceptAlgs, HelperTools=HelperTools, **kwargs)
