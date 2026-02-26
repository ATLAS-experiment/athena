/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CNVTOOLS_STGC_RawDataProviderTool_H
#define MUONTGC_CNVTOOLS_STGC_RawDataProviderTool_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "MuonSTGC_CnvTools/ISTGC_ROD_Decoder.h"
#include "MuonRDO/STGC_RawDataContainer.h"

#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "MuonRDO/STGC_RawDataCollection_Cache.h"

namespace Muon
{
  /** @class STGC_RawDataProviderTool
   *  A tool to decode STGC ROB fragments into STGC RDO (based on the TGC tool).
   *  @author Stelios Angelidakis <sangelid@cern.ch> */
  
  class STGC_RawDataProviderTool : public extends<AthAlgTool, IMuonRawDataProviderTool> {
    public:
      /** Default constructor */
      using base_class::base_class;
      /** Default destructor */
      virtual ~STGC_RawDataProviderTool() = default;
      
      /** Standard AlgTool method */
      virtual StatusCode initialize() override;
      // used
      using IMuonRawDataProviderTool::convert;

      virtual StatusCode convert(const EventContext& ctx) const override;
      virtual StatusCode convert(const std::vector<IdentifierHash>& chamberHashes, 
                                 const EventContext& ctx) const override;
      virtual StatusCode convert(const std::vector<uint32_t>& robIDs, 
                                 const EventContext& ctx) const override;
      
    private:
      /** Method that converts the ROBFragments into the passed container */
      StatusCode convertIntoContainer (const EventContext& ctx,
                                       const ROBFragmentList& fragements, 
                                       const std::vector<IdentifierHash>& chamberHashes, 
                                       STGC_RawDataContainer& target) const;

      /** The ID helper */
      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

      /** Decoder for ROB fragment RDO conversion */
      ToolHandle<ISTGC_ROD_Decoder> m_decoder{this, "Decoder", "Muon::STGC_ROD_Decoder/STGC_ROD_Decoder"};

      /** RDO container key */
      SG::WriteHandleKey<STGC_RawDataContainer> m_rdoContainerKey{ this, "RdoLocation", "sTGCRDO", "Name of the sTGCRDO produced by RawDataProvider"};	//MT

      unsigned int m_maxhashtoUse{0}; //MT

      /** Rob Data Provider handle */
      ServiceHandle<IROBDataProviderSvc>  m_robDataProvider{this, "RobProviderSvc", "ROBDataProviderSvc"};

      /**Flag to skip decoding and write empty container**/
      Gaudi::Property<bool> m_skipDecoding{this, "SkipDecoding", false, "Skip the decoding but still write the container"};


      StatusCode initRdoContainer(const EventContext&, STGC_RawDataContainer*&) const;
      std::vector<uint32_t>  m_allRobIds;
  
      SG::UpdateHandleKey<STGC_RawDataCollection_Cache> m_rdoContainerCacheKey{this, "sTgcContainerCacheKey", "" ,
                                                                            "Optional external cache for the sTGC container"};
  };
} // end of namespace

#endif // MUONTGC_CNVTOOLS_TGC_RAWDATAPROVIDERTOOLMT_H
