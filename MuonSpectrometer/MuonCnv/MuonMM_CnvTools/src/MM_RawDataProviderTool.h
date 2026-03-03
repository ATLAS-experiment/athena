/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMM_CNVTOOLS_MM_RAWDATAPROVIDERTOOL_H
#define MUONMM_CNVTOOLS_MM_RAWDATAPROVIDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonMM_CnvTools/IMM_ROD_Decoder.h"
#include "MuonRDO/MM_RawDataContainer.h"
#include "MuonRDO/MM_RawDataCollection_Cache.h"
#include <string>
#include <vector>

class IdentifierHash;

namespace Muon {

  /** @class MM_RawDataProviderTool
   *  A tool to decode MM ROB fragments into MM RDO (based on the sTGC tool).
   *  @author Leonidas Fountas <leonidas.fountas@cern.ch>
   */
   
  class MM_RawDataProviderTool : public extends<AthAlgTool, IMuonRawDataProviderTool>
  {
    public:
      MM_RawDataProviderTool(const std::string& t, const std::string& n, const IInterface* p);

      /** Default destructor */
      virtual ~MM_RawDataProviderTool() = default;
      
      /** Standard AlgTool method */
      virtual StatusCode initialize() override;

      // IMuonRawDataProviderTool interface - EventContext-based methods
      using IMuonRawDataProviderTool::convert;
      virtual StatusCode convert(const EventContext&) const override;
      virtual StatusCode convert(const std::vector<IdentifierHash>&, const EventContext&) const override;
      virtual StatusCode convert(const std::vector<uint32_t>&, const EventContext&) const override;

    protected:
      /** Method that converts the ROBFragments into the passed container */
      virtual StatusCode convertIntoContainer(const EventContext& ctx, const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>&, 
                                              const std::vector<IdentifierHash>&, MM_RawDataContainer&) const;

      StatusCode initRdoContainer(const EventContext&, MM_RawDataContainer*&) const;

      /** The ID helper */
      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
      
      /** Decoder for ROB fragment RDO conversion */
      ToolHandle<IMM_ROD_Decoder> m_decoder{this, "Decoder", "Muon::MmROD_Decoder/MmROD_Decoder"};
      
      /** RDO container key */
      SG::WriteHandleKey<MM_RawDataContainer> m_rdoContainerKey{ this, "RdoLocation", "MMRDO", "Name of the MMRDO produced by RawDataProvider"};

      unsigned int m_maxhashtoUse{0};

      /** Rob Data Provider handle */
      ServiceHandle<IROBDataProviderSvc>  m_robDataProvider;

      /**Flag to skip decoding and write empty container**/
      Gaudi::Property<bool> m_skipDecoding{this, "SkipDecoding", false, "Skip the decoding but still write the container"};
      
      std::vector<uint32_t>  m_allRobIds;
      
      // Key for the cache for the MM containers; can be empty
      SG::UpdateHandleKey<MM_RawDataCollection_Cache> m_rdoContainerCacheKey{this, "MmContainerCacheKey", 
                                                  "", "Optional external cache for the MM container"};
  };

} // end of namespace

#endif // MUONMM_CNVTOOLS_MM_RAWDATAPROVIDERTOOL_H
