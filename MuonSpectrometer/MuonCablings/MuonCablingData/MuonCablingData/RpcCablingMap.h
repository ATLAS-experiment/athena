/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCABLINGDATA_RPCCABLINGMAP_H
#define MUONCABLINGDATA_RPCCABLINGMAP_H

#include <set>
#include <unordered_map>

#include "MuonCablingData/RpcCablingData.h"
#include "MuonCablingData/RpcFlatCableTranslator.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"

/**********************************************
 *
 * @brief NRPC map data object
 *
 **********************************************/

class RpcIdHelper;
namespace Muon {
    class IMuonIdHelperSvc;

    class RpcCablingMap {
    public:
        /** typedef to implement the csm mapping to ROB */
        /* mapping from hashid to ROB identifier as Subdetector+Rodid */
        using ChamberToROBMap = std::unordered_map<IdentifierHash, uint32_t>;
        using ROBToChamberMap = std::unordered_map<uint32_t, std::vector<IdentifierHash>>;
        using ListOfROB = std::vector<uint32_t>;

        using FlatCablePtr = GeoModel::TransientConstSharedPtr<Muon::RpcFlatCableTranslator>;
        /** @brief Helper struct to associate all strips in an eta/phi gasGap to the corresponding Tdc */
        struct GasGapToTdcAssociation {
            /** @brief Lowest strip number covered by the cable */
            uint8_t firstStrip{0};
            /** @brief Pointer to the flat cable card */
            FlatCablePtr flatCable{};
            /** @brief Associated tdc chip */
            RpcCablingOnlineID tdcID{};
            /** @brief Ordering operator by strip */
            bool operator<(const GasGapToTdcAssociation &other) const {
                return firstStrip < other.firstStrip;
            }
        };
        /** @brief Helper struct to associate all Tdcs to their corresponding strips in the gasGap */
        struct TdcToGasGapAssociation {
            /** @brief Pointer to the flat cable card */
            FlatCablePtr flatCable{};
            /** @brief Associated tdc chip */
            RpcCablingOfflineID gasGapID{};
            /** @brief first strip */
            uint8_t firstStrip{0};
        };
        /** @brief Helper struct to ship all information from the Database  */
        struct CoolDBEntry: public RpcCablingOnlineID, RpcCablingOfflineID {
            /** @brief Lowest strip number covered by the cable */
            uint8_t firstStrip{0}; 
            /** @brief Pointer to the flat cable card */
            FlatCablePtr flatCable{};
        }; 

        /** @brief Constructor taking the IdHelperSvc */
        RpcCablingMap(const IMuonIdHelperSvc* idHelperSvc);
        ~RpcCablingMap();

        /** return the offline id given the online id */
        bool getOfflineId(RpcCablingData &cabling_data, MsgStream& log) const;

        /** return the online id given the offline id */
        bool getOnlineId(RpcCablingData &cabling_data, MsgStream& log) const;


        /** @brief Converts the translation cache back to an Identifier
          *  @param translationCache: Reference to the translation cache to take the Identifier fields from
          *  @param id: Reference to the target Identifier to modify
          *  @param checkValid: Checks whether the resuling Identifier is valid within the boundaries
          *                     of the associated fields. May trigger the function to return false,
          *                     but is also tremendously slow!!! */
        bool convert(const RpcCablingData &translationCache, Identifier &id, bool checkValid = true) const;
        /** @brief Copies the Identifier field onto the translationCache
         *  @param id: Identifier of the offline channel to copy
         *  @param setSideBit: Indicate that the chip is located at the HV side of the chamber (BIL-RPC) */
        bool convert(const Identifier &id, RpcCablingData &translationCache, bool setSideBit) const;
        /** @brief Inserts a new dbEntry into the map
         *  @param dbEntry: Reference to all the DB information to insert
         *  @param log: Refernce to the MsgStream of the executing algorithm to report failures & debugs */
        bool insertChannels(CoolDBEntry&& dbEntry, MsgStream& log);
        /** @brief Returns the list of all Rpc flat cable cards */
        std::set<FlatCablePtr> flatCables() const;
        /// Performs consistency checks for the cabling data (I.e. looking for 0
        /// strips and overlaps)
        bool finalize(MsgStream& log);

        /** return the ROD id of a given chamber, given the hash id */
        uint32_t getROBId(const IdentifierHash& stationCode, MsgStream& log) const;
        /** get the robs corresponding to a vector of hashIds, copied from Svc
         * before the readCdo migration */
        ListOfROB getROBId(const std::vector<IdentifierHash>& rpcHashVector, MsgStream& log) const;
        /** return a HashId list for a  given ROD */
        const std::vector<IdentifierHash>& getChamberHashVec(const uint32_t ROBI, MsgStream& log) const;

        std::vector<IdentifierHash> getChamberHashVec(const ListOfROB& ROBs, MsgStream& log) const;

        /** return the full list of ROD id */
        const ListOfROB& getAllROBId() const;


    private:
        /** Pointer to the RpcIdHelper */
        const RpcIdHelper &m_rpcIdHelper;
        using AllTdcsPerGasGap = std::vector<GasGapToTdcAssociation>;
        using AllGasGapsPerTdc = std::vector<TdcToGasGapAssociation>;
        using OfflToOnlMap = std::map<RpcCablingOfflineID, AllTdcsPerGasGap>;
        using OnlToOfflMap = std::map<RpcCablingOnlineID, AllGasGapsPerTdc>;
        
        /** @brief Map storing the connection from online -> offline channels */
        OnlToOfflMap m_onToOffline{};
        /// Map to cache the offline -> online conversions */
        OfflToOnlMap m_offToOnline{};

        ChamberToROBMap m_chambROBs{};
        ROBToChamberMap m_ROBHashes{};

        /** full list of ROBs */
        ListOfROB m_listOfROB{};
    };
}

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
CLASS_DEF( Muon::RpcCablingMap , 195104634 , 1 );
CONDCONT_DEF( Muon::RpcCablingMap , 138104006 );
#endif