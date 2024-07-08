/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMDT_CABLING_MUONMDT_CABLINGMAP_H
#define MUONMDT_CABLING_MUONMDT_CABLINGMAP_H



#include "AthenaKernel/CLASS_DEF.h"
#include "MuonCablingData/MdtTdcMap.h"
#include <set>
#include <memory>
#include <map>
#include <vector>
#include <cstdint>
/**********************************************
 *
 * @brief MDT map data object
 *
 **********************************************/

class MdtMezzanineType;
class MdtIdHelper;
class IdentifierHash;
class Identifier;

class MuonMDT_CablingMap {
   public:
    /** typedef to implement the list of mezzanine types */
    using MezzanineTypes = std::map<uint8_t, std::unique_ptr<MdtMezzanineType>>;
    using TdcOffSet = std::set<MdtTdcOffSorter, std::less<>>;
    using TdcOnlSet = std::vector<MdtTdcOnlSorter>;

    /// Helper struct to group the Mezzanine cards mounted on each multilayer
    /// The object provides the following information
    ///   1) List of all mezzanine cards associated with the particular
    ///   multilayer 2) The BME / BIS78 chambers are split into 2 CSM modules
    ///   due to their
    //       larger number of tubes. The csm caches the up to 2 CSM associated
    //       with this object
    struct MdtOffChModule {
        TdcOffSet::const_iterator begin() const { return cards.begin(); }
        TdcOffSet::const_iterator end() const { return cards.end(); }
        TdcOffSet::const_iterator begin() { return cards.begin(); }
        TdcOffSet::const_iterator end() { return cards.end(); }

        /// Mezzanine cards mounted on the chamber
        TdcOffSet cards{};
        /// The up to 2 CSMs to which the cards are connected
        std::array<MdtCablingOnData, 2> csm{};
    };

    using OffToOnlMap = std::map<MdtCablingOffData, MdtOffChModule>;
    /// The online -> offline conversion needs to treat two cases
    ///   tdcId && channelId == 0xFF:
    ///     ** Decode the station name using the first module in the set with
    ///     tdcZero() == 0
    ///     ** Ordinary channel decoding
    /// Helper struct below is collection of all the modules & the first module
    /// used to decode the stationName
    struct MdtTdcModule {
        TdcOnlSet::const_iterator begin() const { return all_modules.begin(); }
        TdcOnlSet::const_iterator end() const { return all_modules.end(); }
        TdcOnlSet::const_iterator begin() { return all_modules.begin(); }
        TdcOnlSet::const_iterator end() { return all_modules.end(); }

        TdcOnlSet all_modules{};

        MdtTdcOnlSorter zero_module{nullptr};
    };
    using OnlToOffMap = std::map<MdtCablingOnData, MdtTdcModule>;

    /** typedef to implement the csm mapping to ROB */
    /* mapping from hashid to ROB identifier as Subdetector+Rodid */
    using ChamberToROBMap = std::map<IdentifierHash, uint32_t>;
    using ROBToChamberMap = std::map<uint32_t, std::vector<IdentifierHash>>;
    using ListOfROB = std::vector<uint32_t>;
    using CablingData = MdtCablingData;
    MuonMDT_CablingMap();
    ~MuonMDT_CablingMap();

    /** Add a new line describing a mezzanine type */
    bool addMezzanineLine(const int type, const int layer, const int sequence,
                          MsgStream& log);

    /** Adds a new mezzanine card mapping*/
    bool addMezanineLayout(std::unique_ptr<MdtMezzanineCard> card,
                           MsgStream& log);

    enum class DataSource { JSON, LegacyCOOL };
    /** Add a new fully configured mezzanine card */
    /** the indexes multilayer, layer, tube refer to the tube connected to the
     * channelZero */
    bool addMezzanine(CablingData  cabling_data, DataSource source,
                      MsgStream& log);

    /** return the offline id given the online id */
    bool getOfflineId(CablingData& cabling_data, MsgStream& log) const;

    /** return the online id given the offline id */
    bool getOnlineId(CablingData& cabling_data, MsgStream& log) const;
    /** converts the cabling data into an identifier. The check valid argument
     * optionally enables the check that the returned identifier is actually
     * well defined within the ranges but is also slow */
    bool convert(const CablingData& cabling_data, Identifier& id,
                 bool check_valid = true) const;
    /** converts the identifier into a cabling data object. Returns false if the
     * Identifier is not Mdt */
    bool convert(const Identifier& id, CablingData& cabling_data) const;

    /** return the ROD id of a given chamber, given the hash id */
    uint32_t getROBId(const IdentifierHash& stationCode, MsgStream& log) const;
    /** get the robs corresponding to a vector of hashIds, copied from Svc
     * before the readCdo migration */
    ListOfROB getROBId(const std::vector<IdentifierHash>& mdtHashVector,
                       MsgStream& log) const;

    /** return a vector of HashId lists for a  given list of ROD's */
    std::vector<IdentifierHash> getMultiLayerHashVec(
        const std::vector<uint32_t>& ROBId_list, MsgStream& log) const;

    /** return a HashId list for a  given ROD */
    const std::vector<IdentifierHash>& getMultiLayerHashVec(
        const uint32_t ROBI, MsgStream& log) const;

    /** return the ROD id of a given chamber */
    const ListOfROB& getAllROBId() const;

    /// Returns the map to convert the online -> offline identifiers
    const OnlToOffMap& getOnlineConvMap() const;
    /// Returns hte map to convert the offline -> online identifiers
    const OffToOnlMap& getOfflineConvMap() const;

    bool finalize_init(MsgStream& log);

    /// Transforms the identifier to an IdentifierHash corresponding to the
    /// module
    bool getStationCode(const CablingData& map_data, IdentifierHash& mdtHashId,
                        MsgStream& log) const;
    /// Transforms the identifier to an IdentifierHash corresponding to the
    /// multilayer In this case, the multi layer represents the CSM chip
    bool getMultiLayerCode(const CablingData& map_data, Identifier& multiLayer,
                           IdentifierHash& mdtHashId, MsgStream& log) const;

    /// Returns whether the channel belongs to the first or second mounted CSM
    /// card
    unsigned int csmNumOnChamber(const CablingData& map_data,
                                 MsgStream& log) const;
    /// Returns if the cabling map has found multilayers connected to 2 CSM
    /// cards
    bool has2CsmML() const;
    ///
    using MezzCardPtr = MdtMezzanineCard::MezzCardPtr;
    MezzCardPtr getHedgeHogMapping(uint8_t mezzCardId) const;

   private:
    /** private function to add a chamber to the ROD map */
    bool addChamberToROBMap(const CablingData& cabling_data, MsgStream& log);

    /** Pointer to the MdtIdHelper */
    const MdtIdHelper* m_mdtIdHelper{nullptr};

    /** assignment and copy constructor operator (hidden) */
    MuonMDT_CablingMap& operator=(const MuonMDT_CablingMap& right) = delete;
    MuonMDT_CablingMap(const MuonMDT_CablingMap&) = delete;

    OffToOnlMap m_toOnlineConv{};
    OnlToOffMap m_toOfflineConv{};
    std::vector<std::unique_ptr<MdtTdcMap>> m_tdcs{};

    /** map returning a detector element hashes associated with a given ROD */
    ROBToChamberMap m_ROBToMultiLayer{};

    /** full list of ROBs */
    ListOfROB m_listOfROB{};

    /** map returning the RODid for a given chamber ID */
    ChamberToROBMap m_chamberToROB{};
    /** map raturning the RODid for a given multi layer ID */
    ChamberToROBMap m_multilayerToROB{};
    /** Switch to check whether the layout has chambers with 2 CSM chips*/
    bool m_2CSM_cham{false};

    /// @brief  List of mezzanine cards
    using MezzCardList = std::vector<MezzCardPtr>;
    MezzCardList m_mezzCards{};
    /// In the legacy data format several transformations on the hedgehog layout
    /// were applied during the final TdcMap build
    MezzCardPtr legacyHedgehogCard(CablingData& cabling, MsgStream& msg) const;
};

CLASS_DEF(MuonMDT_CablingMap, 51038731, 1)
#include "AthenaKernel/CondCont.h"
CLASS_DEF(CondCont<MuonMDT_CablingMap>, 34552845, 0)

#endif
