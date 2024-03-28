/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATA_PREPARATION_ALG_H
#define ACTSTRK_DATA_PREPARATION_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "IRegionSelector/IRegSelTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

#include <set>

// Cache
#include "EventContainers/IdentifiableContainer.h"
#include "EventContainers/IdentifiableCache.h"
#include "src/Cache.h"

namespace ActsTrk {

  template <typename external_collection_t, bool useCache>
    class DataPreparationAlg
    : public AthReentrantAlgorithm {
  private:
    using object_t = typename external_collection_t::base_value_type;
    using input_collection_t = external_collection_t;
    using output_collection_t = ConstDataVector<input_collection_t>;

    using cache_read_handle_key_t = typename ActsTrk::Cache::Handles<object_t>::ReadHandleKey;
    using cache_read_handle_t = typename ActsTrk::Cache::Handles<object_t>::ReadHandle;
    
  public:
    DataPreparationAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~DataPreparationAlg() override = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  protected:
    virtual xAOD::DetectorIDHashType retrieveDetectorIDHash(const object_t& obj) const;

  private:
    // Different ways of filling the output collection
    StatusCode fillFromContainer(const EventContext& ctx,
				 output_collection_t& outputCollection) const;
    StatusCode fillFromCache(const EventContext& ctx,
			     output_collection_t& outputCollection) const;

    // Common way of fetching the hash ids from the RoI(s)
    StatusCode fetchIdHashes(const EventContext& ctx,
			     std::set<IdentifierHash>& hashes) const;
    
  private:
    ToolHandle< GenericMonitoringTool > m_monTool {this, "MonTool", "",
	"Monitoring tool"};

    ToolHandle<IRegSelTool> m_regionSelector {this, "RegSelTool", "",
      "Region selector tool"};

    SG::ReadCondHandleKey< InDetDD::SiDetectorElementCollection > m_detEleCollKey {this, "DetectorElements", "",
      "Key of input SiDetectorElementCollection"};
    
    SG::ReadHandleKey< input_collection_t > m_inputCollectionKey {this, "InputCollection", "",
	"Input Collection that will go throug selection process"};

    SG::ReadHandleKey< TrigRoiDescriptorCollection > m_roiCollectionKey {this, "RoIs", "",
      "RoIs to read in"};
    
    SG::WriteHandleKey< output_collection_t > m_outputCollectionKey {this, "OutputCollection", "",
	"Output Collection - result of the selection process"};

    cache_read_handle_key_t m_inputIdentifiableContainer {this, "InputIDC", "",
      "The input IDC container"};
  };

} // namespace

#include "DataPreparationAlg.icc"

#endif
