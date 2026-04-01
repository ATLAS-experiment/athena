/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigT2CaloCommon_TrigCaloDataAccessSvc_h
#define TrigT2CaloCommon_TrigCaloDataAccessSvc_h


#include "AthenaBaseComps/AthService.h"
#include "TrigT2CaloCommon/ITrigCaloDataAccessSvc.h"
#include "TrigT2CaloCommon/LArCellCont.h"

#include "AthenaKernel/SlotSpecificObj.h"
#include "LArByteStream/LArRodDecoder.h"
#include "TileByteStream/TileCellCont.h"
#include "TileByteStream/TileROD_Decoder.h"
#include "TileByteStream/TileHid2RESrcID.h"
#include "LArRawUtils/LArTT_Selector.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "IRegionSelector/IRegSelTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloEvent/CaloBCIDAverage.h"
#include "LArRawConditions/LArMCSym.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRecConditions/LArFebRodMapping.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArRecConditions/LArRoIMap.h"
#include "LArRecEvent/LArDeadOTXFromSC.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include <mutex>
#include <algorithm> //std::copy
#include <string>
#include <vector>
#include <cstdint>
#include <iterator> //std::back_inserter
#include <set>
#include <initializer_list>
#include <memory>

class IRoiDescriptor;


class TrigCaloDataAccessSvc : public extends<AthService, ITrigCaloDataAccessSvc> {
 public:
  using base_class::base_class;
  using ITrigCaloDataAccessSvc::Status;


  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  
  virtual StatusCode loadCollections ( const EventContext& context,
                                       const IRoiDescriptor& roi,
                                       const DETID detID,
                                       const int sampling,
                                       LArTT_Selector<LArCellCont>& loadedCells ) override;
  
  virtual StatusCode loadCollections ( const EventContext& context,
                                       const IRoiDescriptor& roi,
                                       std::vector<const TileCell*>& loadedCells ) override;
  
  virtual StatusCode loadMBTS ( const EventContext& context,
                                std::vector<const TileCell*>& loadedCells ) override;


  
  virtual StatusCode loadFullCollections ( const EventContext& context,
                                           CaloConstCellContainer& cont ) override;
  
 private:
  
  PublicToolHandle<LArRodDecoder> m_larDecoder { this, "LArDecoderTool", "LArRodDecoder/LArRodDecoder", "Tool to decode LAr raw data" };
  PublicToolHandle<TileROD_Decoder> m_tileDecoder { this, "TileDecoderTool", "TileROD_Decoder/TileROD_Decoder", "Tool to decode Tile raw data" };

  ToolHandle<GenericMonitoringTool> m_monTool{ this, "MonTool", "", "Tool to monitor performance of the service" };

  ServiceHandle<IROBDataProviderSvc>  m_robDataProvider{ this, "ROBDataProvider", "ROBDataProviderSvc/ROBDataProviderSvc", ""};
  ToolHandle<IRegSelTool>           m_regionSelector_TTEM  { this, "RegSelToolEM",  "RegSelTool/RegSelTool_TTEM" };
  ToolHandle<IRegSelTool>           m_regionSelector_TTHEC  { this, "RegSelToolHEC",  "RegSelTool/RegSelTool_TTHEC" };
  ToolHandle<IRegSelTool>           m_regionSelector_FCALEM  { this, "RegSelToolFCALEM",  "RegSelTool/RegSelTool_FCALEM" };
  ToolHandle<IRegSelTool>           m_regionSelector_FCALHAD  { this, "RegSelToolFCALHAD",  "RegSelTool/RegSelTool_FCALHAD" };
  ToolHandle<IRegSelTool>           m_regionSelector_TILE  { this, "RegSelToolTILE",  "RegSelTool/RegSelTool_TILE" };
  
  Gaudi::Property<bool> m_applyOffsetCorrection { this, "ApplyOffsetCorrection", true, "Enable offset correction" };

  SG::ReadHandleKey<CaloBCIDAverage> m_bcidAvgKey
   {this, "BCIDAvgKey", "CaloBCIDAverage", "SG Key of CaloBCIDAverage object"} ;
  SG::ReadCondHandleKey<LArMCSym> m_mcsymKey 
   {this, "MCSymKey", "LArMCSym", "SG Key of LArMCSym object"} ;
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_onOffIdMappingKey
   {this, "CablingKey", "LArOnOffIdMap", "SG Key for LArOnOffIdMapping"} ;
  SG::ReadCondHandleKey<LArFebRodMapping> m_febRodMappingKey
   {this, "RodFebKey", "LArFebRodMap", "SG Key for LArFebRodMapping"} ;
  SG::ReadCondHandleKey<LArBadChannelCont> m_bcContKey
   {this, "LArBadChannelKey", "LArBadChannel", "Key of the LArBadChannelCont CDO" };
  SG::ReadCondHandleKey<LArRoIMap> m_larRoIMapKey
   {this, "LArRoIMapKey", "LArRoIMap", "Key of the LArRoIMap CDO" };
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey
   {this, "CaloDetDescrManager", "CaloDetDescrManager", "SG Key for CaloDetDescrManager in the Condition Store" };
  SG::ReadCondHandleKey<TileHid2RESrcID> m_tileHid2RESrcIDKey
   {this, "TileHid2RESrcID", "TileHid2RESrcIDHLT", "SG Key of TileHid2RESrcID object"} ;
  SG::ReadHandleKey<LArDeadOTXFromSC> m_deadOTXFromSCKey
   {this, "LArDeadOTXFromSC", "DeadOTXFromSC", "Key of the DeadOTXFromSC CDO" };
  bool m_correctDead{false};

  /**
   * @brief convience structure to keep together a collection and auxiliar full collection selectors
   */
  struct HLTCaloEventCache {
    std::mutex mutex;    
    std::unique_ptr<LArCellCont> larContainer;
    LArRodBlockStructure* larRodBlockStructure_per_slot; // LAr Rod Block to ease decoding
    uint16_t rodMinorVersion;
    uint32_t robBlockType;
    std::unique_ptr<TileCellCont> tileContainer;
    std::unique_ptr<CaloCellContainer> fullcont;
    std::unique_ptr<TileROD_Decoder::D0CellsHLT> d0cells;
    unsigned int lastFSEvent;
  };

  // cells created in lateInit which must be deleted in finalize
  std::vector<unsigned int> m_insertedCells;
  
  SG::SlotSpecificObj< HLTCaloEventCache > m_hLTCaloSlot;

  std::mutex m_getCollMutex; // Make sure writing to a collection is protected
  std::mutex m_lardecoderProtect;  // protection for the larRodDecoder
  std::mutex m_tiledecoderProtect;  // protection for the tileRodDecoder

  void reset_LArCol(LArCellCollection* coll);
  void reset_TileCol(TileCellCollection* col);

  void lateInit( const EventContext& context );
  std::once_flag m_lateInitFlag;

  void convertROBs(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& robFrags, LArCellCont* larcell, LArRodBlockStructure*& larRodBlockStructure, uint16_t rodMinorVersion, uint32_t robBlockType, const LArDeadOTXFromSC* dead );
  void convertROBs( const EventContext& context, const std::vector<IdentifierHash>& rIds, TileCellCont* tilecell, TileROD_Decoder::D0CellsHLT* d0cells );


  /**
   * @brief fill the set of missing robs given the request and response from RoBDatProvider
   **/
  void missingROBs( const std::vector<uint32_t>& request,
		    const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& response,
		    std::set<uint32_t>& missing ) const;

  /**
   * @brief clear fragments of the collection for which ROBs were not available
   **/
  void clearMissing( const std::vector<uint32_t>& request,
		     const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& response, 
		     LArCellCont* larcell );

  /**
   * @brief LAr TT collections preparation code
   **/
  StatusCode prepareLArCollections( const EventContext& context,
                                    const IRoiDescriptor& roi,
                                    const int sampling,
                                    DETID detector );

  StatusCode prepareTileCollections( const EventContext& context,
                                     const IRoiDescriptor& roi );

  StatusCode prepareMBTSCollections( const EventContext& context);

  StatusCode prepareLArFullCollections( const EventContext& context );

  StatusCode prepareTileFullCollections( const EventContext& context );

  std::vector<uint32_t> m_vrodid32fullDet;
  std::vector<uint32_t> m_vrodid32tile;
  std::vector<unsigned int> m_mbts_add_rods;
  std::vector<IdentifierHash> m_rIdstile;
  std::vector<std::vector<uint32_t> > m_vrodid32fullDetHG;
};


#endif
