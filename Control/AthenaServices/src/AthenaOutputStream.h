// Dear emacs, this is -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENASERVICES_ATHENAOUTPUTSTREAM_H
#define ATHENASERVICES_ATHENAOUTPUTSTREAM_H

// STL include files
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

// Required for inheritance
#include "Gaudi/Property.h"
#include "GaudiKernel/IDataSelector.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

// Framework include files
#include "AthenaBaseComps/FilteredAlgorithm.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "AthenaKernel/IAthenaOutputTool.h"
#include "AthenaKernel/IDictLoaderSvc.h"
#include "AthenaKernel/ITPCnvSvc.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIoComponent.h"
#include "StoreGate/WriteHandleKey.h"

// Local include files
#include "CompressionInfo.h"
#include "MetaDataSvc.h"
#include "OutputStreamSequencerSvc.h"
#include "SelectionVetoes.h"

// forward declarations
namespace SG {
   class DataProxy;
   class IFolder;
   class IAuxStoreIO;
   class FolderItem;
}

/** @class AthenaOutputStream
   * @brief algorithm that marks for write data objects in SG
   * @author srinir@bnl.gov
   */
class AthenaOutputStream : public extends<FilteredAlgorithm,
                                          IIncidentListener, IIoComponent> {

public:
   /// typedefs
   typedef std::vector<SG::DataProxy*>     Items;
   typedef std::vector<std::pair<std::string, std::string> > TypeKeyPairs;
   typedef std::recursive_mutex mutex_t;

   /// Standard algorithm Constructor
   AthenaOutputStream(const std::string& name, ISvcLocator* pSvcLocator);

   /// Standard Destructor
   virtual ~AthenaOutputStream();

   /// \name implement IAlgorithm
   //@{
   virtual StatusCode initialize() override;
   virtual StatusCode finalize() override;
   virtual StatusCode execute() override;
   //@}

   /// Stream the data
   virtual StatusCode write();

   /// Clear list of selected objects
   void clearSelection();

   /// Collect data objects for output streamer list
   StatusCode collectAllObjects();

   /// Return the list of selected objects
   IDataSelector* selectedObjects() {
      return &m_objects;
   }

   /// Incident service handle listening for MetaDataStop
   virtual void handle(const Incident& incident) override;

   /// Callback method to reinitialize the internal state of the component for I/O purposes (e.g. upon @c fork(2))
   virtual StatusCode io_reinit() override;
   virtual StatusCode io_finalize() override;

protected:
   /// Handle to the @c StoreGateSvc store where the data we want to
   /// write out resides
   ServiceHandle<StoreGateSvc>  m_dataStore{this, "Store", "StoreGateSvc/StoreGateSvc", "Handle to event store"};
   ServiceHandle<StoreGateSvc>  m_metadataStore{this, "MetadataStore", "StoreGateSvc/MetaDataStore", "Handle to metadata store"};
   ServiceHandle<StoreGateSvc>* m_currentStore;

   /// Handles to all the necessary services
   ServiceHandle<MetaDataSvc>              m_metaDataSvc{this, "MetaDataSvc", "MetaDataSvc"};
   ServiceHandle<IDictLoaderSvc>           m_dictLoader{this, "AthDictLoaderSvc", "AthDictLoaderSvc"};
   ServiceHandle<ITPCnvSvc>                m_tpCnvSvc{this, "AthTPCnvSvc", "AthTPCnvSvc"};
   ServiceHandle<IIncidentSvc>             m_incidentSvc{this, "IncidentSvc", "IncidentSvc"};
   ServiceHandle<IClassIDSvc>              m_pCLIDSvc{this, "ClassIDSvc", "ClassIDSvc"};
   ServiceHandle<OutputStreamSequencerSvc> m_outSeqSvc{this, "OutputStreamSequencerSvc", "OutputStreamSequencerSvc"};

   /// Stream name (defaults to algorithm name)
   StringProperty           m_streamName{this, "StreamName", "", "Name of the output stream"};

   /// Vector of item names
   StringArrayProperty      m_itemList{this, "ItemList", {}, "List of items to write", "OutputStreamItemList"};

   /// Vector of item names
   StringArrayProperty      m_metadataItemList{this, "MetadataItemList", {}, "List of metadata items to write","OutputStreamItemList"};

   /// Provenance record selection
   StringProperty           m_keepProvenances {this, "KeepProvenanceTagsRegEx", {".*"},
                              "RegEx pattern to select processing tags for which DataHeader should retain provenances"};

   /// Vector of item names
   StringArrayProperty      m_compressionListHigh{this, "CompressionListHigh", {}, "Lossy float compression list (high)"};

   /// Vector of item names
   StringArrayProperty      m_compressionListLow{this, "CompressionListLow", {}, "Lossy float compression list (low)"};

   /// Number of mantissa bits in the float compression
   UnsignedIntegerProperty  m_compressionBitsHigh{this, "CompressionBitsHigh", 7, "Lossy float compression bits (high)"};

   /// Number of mantissa bits in the float compression
   UnsignedIntegerProperty  m_compressionBitsLow{this, "CompressionBitsLow", 15, "Lossy float compression bits (low)"};

   /// List of items that are known to be present in the transient store
   /// (and hence we can make input dependencies on them).
   StringArrayProperty      m_transientItems{this, "TransientItems", {}, "Transient item list"};

   /// Name of the output file
   StringProperty           m_outputName{this, "OutputFile", "DidNotNameOutput.root", "Name of the output file"};

   /// Name of the persistency service capable to write data from the store
   StringProperty           m_persName{this, "EvtConversionSvc", "EventPersistencySvc", "Name of the persistency service writing data"};

   /// set to true to force read of data objects in item list
   BooleanProperty m_forceRead{this, "ForceRead", true, "Force read data objects in ItemList"};

   /// Set to false to omit adding the current DataHeader into the DataHeader history
   /// This will cause the input file to be neglected for back navigation (replace mode).
   BooleanProperty m_extendProvenanceRecord{this, "ExtendProvenanceRecord", true, "Extend provenance record"};

   /// Set to write out everything in input DataHeader
   BooleanProperty m_itemListFromTool{this, "TakeItemsFromInput", false, "Write everything in input DataHeader to output"};

   /// The top-level folder with items to be written
   ToolHandle<SG::IFolder>  m_p2BWritten;

   /// The top-level folder with items to be compressed high
   ToolHandle<SG::IFolder>  m_compressionDecoderHigh;

   /// The top-level folder with items to be compressed low
   ToolHandle<SG::IFolder>  m_compressionDecoderLow;

   /// Decoded list of transient ids.
   ToolHandle<SG::IFolder>  m_transient;

   /// Map of (clid,key) pairs to be excluded (comes from m_excludeList)
   std::multimap<CLID,std::string> m_CLIDKeyPairs;

   /// Collection of objects being selected
   IDataSelector m_objects;

   /// Objects overridden by `exact' handling.
   IDataSelector m_altObjects;

   /// Collection of DataObject instances owned by this service.
   /// FIXME: it would be simpler to just have m_objects be a vector
   /// of DataObjectSharedPtr<DataObject>, but that implies interface changes.
   std::vector<std::unique_ptr<DataObject> > m_ownedObjects;

   /// pointer to AthenaOutputStreamTool
   ToolHandle<IAthenaOutputStreamTool> m_streamer;

   /// vector of AlgTools that that are executed by this stream
   ToolHandleArray<IAthenaOutputTool> m_helperTools{this, "HelperTools", {}, "List of AlgTools used by this stream"};

   // flag set by MetaDataStop if OutputSequencer is used with EndEvent
   bool m_writeMetadataAndDisconnect{false};

   /// Number of events written to this output stream
   std::atomic<int> m_events{0};

   // ------- Event Ranges handling in MT -------
   /// map of filenames assigned to active slots
   std::map< unsigned, std::string >  m_slotRangeMap;

   /// map of RangeIDs (as used by the Sequencer) for each Range filename generated
   std::map< std::string, std::string > m_rangeIDforRangeFN;

   /// map of streamerTools handling event ranges in MT
   std::map< std::string, std::unique_ptr<IAthenaOutputStreamTool> > m_streamerMap;

   /// mutex for this Stream write() and handle() methods
   mutex_t  m_mutex;

   /// Handler for ItemNames Property
   void itemListHandler(Gaudi::Details::PropertyBase& /* theProp */);

   /// Handler for ItemNames Property
   void excludeListHandler(Gaudi::Details::PropertyBase& /* theProp */);

   /// Handler for ItemNames Property
   void compressionListHandlerHigh(Gaudi::Details::PropertyBase& /* theProp */);

   /// Handler for ItemNames Property
   void compressionListHandlerLow(Gaudi::Details::PropertyBase& /* theProp */);

private:
   /// Key used for recording selected dynamic variable information
   /// to the event store.
   SG::WriteHandleKey<SG::SelectionVetoes> m_selVetoesKey
   { this, "SelVetoesKey", "" };

   /// Key used for recording lossy float compressed variable information
   /// to the event store.
   SG::WriteHandleKey<SG::CompressionInfo> m_compInfoKey
   { this, "CompInfoKey", "" };

   /// Output attributes
   std::string m_outputAttributes;

   /// Add item data objects to output streamer list
   StatusCode addItemObjects(const SG::FolderItem&, SG::SelectionVetoes& vetoes, SG::CompressionInfo& compInfo);

   void handleVariableSelection (const SG::IConstAuxStore& auxstore,
                                 SG::DataProxy& itemProxy,
                                 const std::string& aux_attr,
                                 SG::SelectionVetoes& vetoes) const;

   /// Write MetaData for this stream (by default) or for a substream outputFN (in ES mode)
   void writeMetaData( const std::string& outputFN="" );

   /// Helper function for building the compression lists
   std::set<std::string> buildCompressionSet (const ToolHandle<SG::IFolder>& handle,
                                              const CLID& item_id,
                                              const std::string& item_key) const;

   // close an EventService substream that was writing to 'rangeFN' output
   void finalizeRange( const std::string & rangeFN );

  /// Helper function to load dictionaries (both transient and persistent)
  /// for a given type.
  /// We want to to this explicitly during initialization to avoid sporadic
  /// failures seen loading dictionaries while multiple threads are running.
  /// See ATEAM-697 and ATEAM-749.
  void loadDict (CLID clid);

  /// Glob-style matcher, where the only meta-character is '*'
  bool simpleMatch(const std::string& pattern, const std::string& text);
};

#endif // ATHENASERVICES_OUTPUTSTREAM_H
