/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CNVTOOLS_sTgcRdoToPrepDataToolMT
#define MUONTGC_CNVTOOLS_sTgcRdoToPrepDataToolMT

#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonRDO/STGC_RawDataContainer.h"
#include "MuonPrepRawData/sTgcPrepDataContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "STgcClusterization/ISTgcClusterBuilderTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "NSWCalibTools/INSWCalibTool.h"
#include "MuonPrepRawData/MuonPrepDataCollection_Cache.h"
#include "xAODMuonPrepData/sTgcStripContainer.h"
#include "xAODMuonPrepData/sTgcWireContainer.h"
#include "xAODMuonPrepData/sTgcPadContainer.h"
#include "xAODMuonPrepData/sTgcStripAuxContainer.h"
#include "xAODMuonPrepData/sTgcWireAuxContainer.h"
#include "xAODMuonPrepData/sTgcPadAuxContainer.h"
#include "xAODMuonViews/FillContainer.h"

namespace MuonGMR4{
  class MuonDetectorManager;
}

namespace Muon 
{
  /** @class STGC_RawDataToPrepDataTool 
   *  This is the algorithm that convert STGC Raw data  To STGC PRD  as a tool.
   */  

  class sTgcRdoToPrepDataToolMT : public extends<AthAlgTool, IMuonRdoToPrepDataTool>
    {
    public:
      /** Constructor */
      using base_class::base_class;
      
      /** Destructor */
      virtual ~sTgcRdoToPrepDataToolMT()=default;
      
      /** Standard AthAlgTool initialize method */
      virtual StatusCode initialize() override;
      
      /** Decode RDO to PRD  
       *  A vector of IdentifierHash are passed in, and the data corresponding to this list (i.e. in a Region of Interest) are converted.  
       *  @param requestedIdHashVect          Vector of hashes to convert i.e. the hashes of ROD collections in a 'Region of Interest'  
       *  @return selectedIdHashVect This is the subset of requestedIdVect which were actually found to contain data   
       *  (i.e. if you want you can use this vector of hashes to optimise the retrieval of data in subsequent steps.) */
      
      StatusCode decode(const EventContext& ctx, const std::vector<IdentifierHash>& idVect) const override;      
      StatusCode decode(const EventContext& ctx, const std::vector<uint32_t>& robIds) const override;
      StatusCode provideEmptyContainer(const EventContext& ctx) const override;

    protected:
      using PrdKey_t =  SG::WriteHandleKey<sTgcPrepDataContainer>;
      /** @brief Data cache to hold the translated prd & xAOD containers */
      struct DataCache {
        using StripCont_t = xAOD::FillContainer<xAOD::sTgcStripContainer,
                                                xAOD::sTgcStripAuxContainer>;
        using WireCont_t = xAOD::FillContainer<xAOD::sTgcWireContainer,
                                               xAOD::sTgcWireAuxContainer>;
        using PadCont_t = xAOD::FillContainer<xAOD::sTgcPadContainer,
                                              xAOD::sTgcPadAuxContainer>;
        StripCont_t strips{};
        WireCont_t  wires{};
        PadCont_t   pads{};

        DataCache(const std::size_t hashMax,
                  const PrdKey_t& key,
                  const EventContext& ctx);
        
        SG::WriteHandle<sTgcPrepDataContainer> prdWriteHandle{};

        std::vector<std::unique_ptr<sTgcPrepDataCollection>> collections{};

        const MuonGMR4::MuonDetectorManager* detMgr{nullptr};

        bool isValid{false};

        ~DataCache();
        DataCache& operator=(DataCache&& other) = default;
        DataCache(DataCache&& other) = default;

        void translateAndSort(sTgcPrepDataCollection& coll);
      };
      
      StatusCode processCollection(const EventContext& ctx, 
                                   DataCache& xAODcontainers,
                                   const STGC_RawDataCollection *rdoColl) const;
            
      DataCache setupOutputContainers(const EventContext& ctx) const;
      const STGC_RawDataContainer* getRdoContainer(const EventContext& ctx) const;

      void processRDOContainer(const EventContext& ctx,
                               DataCache& xAODcontainers,
                               const std::vector<IdentifierHash>& idsToDecode) const;

      SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muDetMgrKey {this, "DetectorManagerKey", "MuonDetectorManager", "Key of input MuonDetectorManager condition data"}; 

      ServiceHandle<IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "MuonIdHelperSvc/MuonIdHelperSvc"};

    
      SG::ReadHandleKey<STGC_RawDataContainer> m_rdoContainerKey{this, "InputCollection", "sTGCRDO", "RDO container to read"};
      PrdKey_t m_stgcPrepDataContainerKey{this, "OutputCollection", "STGC_Measurements", "sTgcPrepDataContainer to record"};
      Gaudi::Property<bool> m_merge{this, "Merge", true}; // merge Prds

      ToolHandle<ISTgcClusterBuilderTool> m_clusterBuilderTool{this,"ClusterBuilderTool","SimpleSTgcClusterBuilderTool/SimpleSTgcClusterBuilderTool"};
      ToolHandle<INSWCalibTool> m_calibTool{this,"NSWCalibTool", ""};

      /// This is the key for the cache for the sTGC PRD containers, can be empty
      SG::UpdateHandleKey<sTgcPrepDataCollection_Cache> m_prdContainerCacheKey{this, "PrdCacheKey", "", "Optional external cache for the sTGC PRD container"};

      // xAOD output containers keys
      SG::WriteHandleKey<xAOD::sTgcStripContainer> m_xAODStripKey{this, "xAODStripKey", "", "If empty, do not produce xAOD, otherwise this is the key of the output xAOD MDT PRD container"};
      SG::WriteHandleKey<xAOD::sTgcPadContainer>  m_xAODPadKey{this, "xAODPadKey", "", "If empty, do not produce xAOD, otherwise this is the key of the output xAOD MDT PRD container"};
      SG::WriteHandleKey<xAOD::sTgcWireContainer> m_xAODWireKey{this, "xAODWireKey", "", "If empty, do not produce xAOD, otherwise this is the key of the output xAOD MDT PRD container"};

      Gaudi::Property<bool> m_useNewGeo{this, "UseR4DetMgr", false,
                                         "Switch between the legacy and the new geometry"};

      const MuonGMR4::MuonDetectorManager* m_detMgrR4{nullptr};


   }; 
} // end of namespace

#endif // MUONTGC_CNVTOOLS_STGCRDOTOPREPDATATOOL_H
