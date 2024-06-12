/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONNRPC_CABLING_MUONNRPC_CABLINGMAP_H
#define MUONNRPC_CABLING_MUONNRPC_CABLINGMAP_H
#include <set>

#include "AthenaKernel/CLASS_DEF.h"
#include "Identifier/Identifier.h"
#include "MuonCablingData/NrpcCablingData.h"
/**********************************************
 *
 * @brief NRPC map data object
 *
 **********************************************/

class RpcIdHelper;
class IdentifierHash;

class MuonNRPC_CablingMap {
   public:
    /** typedef to implement the csm mapping to ROB */
    /* mapping from hashid to ROB identifier as Subdetector+Rodid */
    using ChamberToROBMap = std::map<IdentifierHash, uint32_t>;
    using ROBToChamberMap = std::map<uint32_t, std::vector<IdentifierHash>>;
    using ListOfROB = std::vector<uint32_t>;
    MuonNRPC_CablingMap();
    ~MuonNRPC_CablingMap();

    /** return the offline id given the online id */
    bool getOfflineId(NrpcCablingData& cabling_data, MsgStream& log) const;

    /** return the online id given the offline id */
    bool getOnlineId(NrpcCablingData& cabling_data, MsgStream& log) const;
    /** converts the cabling data into an identifier. The check valid argument
     * optionally enables the check that the returned identifier is actually
     * well defined within the ranges but is also slow */
    bool convert(const NrpcCablingData& cabling_data, Identifier& id,
                 bool check_valid = true) const;
    /** converts the identifier into a cabling data object. Returns false if the
     * Identifier is not Nrpc */
    bool convert(const Identifier& id, NrpcCablingData& cabling_data) const;

    /// Inserts a cabling object into the map
    bool insertChannels(const NrpcCablingCoolData& cabling_data, MsgStream& log);
    /// Performs consistency checks for the cabling data (I.e. looking for 0
    /// strips and overlaps)
    bool finalize(MsgStream& log);

    /** return the ROD id of a given chamber, given the hash id */
    uint32_t getROBId(const IdentifierHash& stationCode, MsgStream& log) const;
    /** get the robs corresponding to a vector of hashIds, copied from Svc
     * before the readCdo migration */
    ListOfROB getROBId(const std::vector<IdentifierHash>& rpcHashVector,
                       MsgStream& log) const;
    /** return a HashId list for a  given ROD */
    const std::vector<IdentifierHash>& getChamberHashVec(const uint32_t ROBI,
                                                         MsgStream& log) const;
    /** return the full list of ROD id */
    const ListOfROB& getAllROBId() const;

   private:
    using OnlToOfflMap = std::map<NrpcCablOnDataByTdc, NrpcCablingOfflineID, std::less<>>;
    using OfflToOnlMap = std::map<NrpcCablingOfflineID, NrpcCablOnDataByStripSet, std::less<>>;
    /// Map to cache the online -> offline conversions
    OnlToOfflMap m_onToOffline{};
    /// Map to cache the offline -> online conversions
    OfflToOnlMap m_offToOnline{};

    ChamberToROBMap m_chambROBs{};
    ROBToChamberMap m_ROBHashes{};

   /** Pointer to the RpcIdHelper */
    const RpcIdHelper* m_rpcIdHelper{};

    /** full list of ROBs */
    ListOfROB m_listOfROB{};

};

CLASS_DEF(MuonNRPC_CablingMap, 94020450, 0)
#include "AthenaKernel/CondCont.h"
CLASS_DEF(CondCont<MuonNRPC_CablingMap>, 207572956, 0)

#endif
