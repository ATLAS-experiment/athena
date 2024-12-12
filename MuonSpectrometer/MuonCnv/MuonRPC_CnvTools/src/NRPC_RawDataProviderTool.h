/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRPCCNVTOOLS_MUONNRPCRAWDATAPROVIDERTOOL_H
#define MUONRPCCNVTOOLS_MUONNRPCRAWDATAPROVIDERTOOL_H

#include <set>

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonCablingData/RpcCablingMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "eformat/Issue.h"
#include "eformat/SourceIdentifier.h"

#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"


////////////////////////////////////////////////////////////////////////////////////////////////
//  Each ROB contains 6 e-links -> each e-link collects data from 3 TDCs (18 TDCs per ROB) 
//  1 ROB -> 1 Sector of BIS78
//  
//  Bytestream definitions (32 bits words)
//  word1 -> Number of words from e-link (3 TDCs): 6 (for empty) + 1 per hit
//  word2 -> e-link ID
//  word3 -> BCID
//  word4 -> L1ID + Counter from event counter reset
//  word5 -> hits (1 per word)
//  (...)
//  word(N-1) -> Trailer a0 + counters of hits in BCID+0, BCID+1, BCID+2
//  word(N) -> counters of hits in BCID+3, BCID+4, BCID+5, BCID+6
//
//-------------------------------------------------------------------------------------------
//
//  -- BCID decoding (word3)
//  -- some inversions needed as follows, each letter represents 4 bits:
//    word: ab|cd|ef|gh
//    correct word: gh|ef|cd|ab
//    BCID: g|he|fc|da  (last 4 bits "b" not used)
//
//
//  -- HITS decoding (word5 and following, when non empty)
//  -- some inversions needed as follows, each letter represents 4 bits:
//    word: ab|cd|ef|gh
//    correct word: gh|ef|cd|ab
//    TDC: gh
//    Channel: ef
//    Time over threshold: cd * 0.4 = ToT (ns) 
//    BCID-hit: a (last 4 bits of real BCID of the hit, may be different from the nominal BCID)
//    time: b * 1.6 = time (ns)
//
//////////////////////////////////////////////////////////////////////////////////////////////////


namespace Muon {

    class NRPC_RawDataProviderTool : public extends<AthAlgTool, IMuonRawDataProviderTool> {
    public:
        NRPC_RawDataProviderTool(const std::string&, const std::string&, const IInterface*);

        /** default destructor */
        virtual ~NRPC_RawDataProviderTool() = default;

        /** standard Athena-Algorithm method */
        virtual StatusCode initialize() override;

      
        /** Convert method - declared in Muon::IMuonRdoToPrepDataTool*/
        virtual StatusCode convert(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs) const override;
        virtual StatusCode convert(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
                                   const std::vector<IdentifierHash>&) const override;
        /** the new ones */
        virtual StatusCode convert() const override;  //!< for the entire event
        virtual StatusCode convert(const std::vector<IdentifierHash>& HashVec) const override;
        virtual StatusCode convert(const std::vector<uint32_t>& robIds) const override;  //!< for a particular vector of ROBId's
        /** EventContext **/
        virtual StatusCode convert(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
                                   const EventContext& ctx) const override;
        virtual StatusCode convert(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
                                   const std::vector<IdentifierHash>&, const EventContext& ctx) const override;
        virtual StatusCode convert(const EventContext& ctx) const override;  //!< for the entire event
        virtual StatusCode convert(const std::vector<IdentifierHash>& HashVec, const EventContext& ctx) const override;
        virtual StatusCode convert(const std::vector<uint32_t>& robIds,
                                   const EventContext& ctx) const override;  //!< for a particular vector of ROBId's
        /** Convert method */
        virtual StatusCode convertIntoContainer(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
                                                xAOD::NRPCRDOContainer& nrpcContainer) const;

        virtual StatusCode fillCollections(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag, xAOD::NRPCRDOContainer& rdoIdc) const;

    private:
        typedef OFFLINE_FRAGMENTS_NAMESPACE::PointerType BS;
        
        SG::WriteHandleKey<xAOD::NRPCRDOContainer> m_rdoContainerKey{this, "NrpcRdoKey", "NRPCRDO", "WriteHandleKey for Output AOD::NRPCRDOContainer"};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        // Rob Data Provider handle
        ServiceHandle<IROBDataProviderSvc> m_robDataProvider{this, "ROBDataProviderSvc", "ROBDataProviderSvc"};

        SG::ReadCondHandleKey<RpcCablingMap> m_readKey{this, "ReadKey", "MuonNRPC_CablingMap", "Key of MuonNRPC_CablingMap"};
    };
}  // namespace Muon

#endif
