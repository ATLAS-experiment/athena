/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <cstdlib>
#include "TestTools/initGaudi.h"
#include <Identifier/IdentifierHash.h>
#include <xAODMuonViews/ChamberViewer.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <AthenaBaseComps/AthService.h>

namespace Muon::Test{
    class IdHelperSvcMock : public extends<AthService, Muon::IMuonIdHelperSvc> {
    public:
        using base_class::base_class;

        /** @brief print all fields to string */
        std::string toString(const Identifier& /*id*/) const override { return ""; }
        /** @brief print all fields up to technology to string */
        std::string toStringTech(const Identifier& /*id*/) const override { return ""; }
        /** @brief print all fields up to stationName to string */
        std::string toStringStation(const Identifier& /*id*/) const override { return ""; }
        /** @brief print all fields up to chamber to string */
        std::string toStringChamber(const Identifier& /*id*/) const override { return ""; }
        /** @brief print all fields up to detector element to string */
        std::string toStringDetEl(const Identifier& /*id*/) const override { return ""; }
        /** @brief print all fields up to gas gap to string */
        std::string toStringGasGap(const Identifier& /*id*/) const override { return ""; }
        /** @brief print chamber name to string */
        std::string chamberNameString(const Identifier& /*id*/) const override { return ""; }
        /** @brief returns whether this is a Muon Identifier or not */
        bool isMuon(const Identifier& /*id*/) const override { return false; }
        /** @brief returns whether this is a MDT Identifier or not */
         bool isMdt(const Identifier& /*id*/) const override { return false; }
        /** @brief returns whether this is a MM Identifier or not */
         bool isMM(const Identifier& /*id*/) const override { return false; }
        /** @brief returns whether this is a CSC Identifier or not */
         bool isCsc(const Identifier& /*id*/) const override { return false; }
        /** @brief returns whether this is a RPC Identifier or not */
         bool isRpc(const Identifier& /*id*/) const override { return false; }
        /** @brief returns whether this is a TGC Identifier or not */
         bool isTgc(const Identifier& /*id*/) const override { return false; }
        /** @brief returns whether this is a sTGC Identifier or not */
        bool issTgc(const Identifier& /*id*/) const override { return false; }
        /** @brief returns whether this is a sMDT Identifier or not */
        bool issMdt(const Identifier& /*id*/) const override { return false; }
        /** @brief Returns the module hash associated to an Identifier */
        IdentifierHash moduleHash(const Identifier& id) const override { 
            return IdentifierHash{id.get_identifier32().get_compact()};
        }
        /** @brief Returns the detector element hash associated to an Identifier */
        IdentifierHash detElementHash(const Identifier& id) const override { 
            return IdentifierHash{id.get_identifier32().get_compact()}; 
        }
        /** @brief returns whether this Identifier belongs to an MDT with HPTDC or not
            NOTE that in Run4, no HPTDCs at all are planned to be present any more,
            so this function should be obsolete from Run4 onwards */
        bool hasHPTDC(const Identifier& /*id*/) const override { return false; }
        /** @brief returns whether channel measures phi or not */
        bool measuresPhi(const Identifier& /*id*/) const override { return false; }
        /** @brief returns whether this is an endcap Identifier or not */
        bool isEndcap(const Identifier& /*id*/) const override { return false; }
        /** @brief returns whether trigger chamber id or not */
        bool isTrigger(const Identifier& /*id*/) const override { return false; }
        /** @brief returns whether this is a small chamber, always returns true for TGCs */
        bool isSmallChamber(const Identifier& /*id*/) const override { return false;}
        /** @brief returns gas gap: gasGap for RPC + TGC, wireLayer for CSC, tube for MDT */
        int gasGap(const Identifier& /*id*/) const override { return 0; }
        /** @brief create a chamber ID */
        Identifier chamberId(const Identifier& id) const override {return id; }
        /** @brief create a detector element ID */
        Identifier detElId(const Identifier& id) const override { return id;}
        /** @brief create a gasGap ID (will return layer Id for MDTs) */
        Identifier gasGapId(const Identifier& id) const override { return id; }
        /** @brief create a layer ID, returns tube id for the MDTs */
        Identifier layerId(const Identifier& id) const override { return id; }
        /** @brief access to MdtIdHelper */
         const MdtIdHelper& mdtIdHelper() const override {
            static const MdtIdHelper mdtIdHelper;
            return mdtIdHelper;
        }
        /** @brief access to RpcIdHelper */
         const RpcIdHelper& rpcIdHelper() const override {
            static const RpcIdHelper rpcIdHelper;
            return rpcIdHelper;
        }
        /** @brief access to TgcIdHelper */
         const TgcIdHelper& tgcIdHelper() const override {
            static const TgcIdHelper tgcIdHelper;
            return tgcIdHelper;
        }
        /** @brief access to CscIdHelper */
         const CscIdHelper& cscIdHelper() const override {
            static const CscIdHelper cscIdHelper;
            return cscIdHelper;
        }
        /** @brief access to TgcIdHelper */
         const sTgcIdHelper& stgcIdHelper() const override {
            static const sTgcIdHelper stgcIdHelper;
            return stgcIdHelper;
        }
        /** @brief access to CscIdHelper */
         const MmIdHelper& mmIdHelper() const override {
            static const MmIdHelper mmIdHelper;
            return mmIdHelper;
        }
        /** @brief calculate chamber index from Identifier */
         MuonStationIndex::ChIndex chamberIndex(const Identifier& /*id*/) const override {
            return MuonStationIndex::ChIndex::ChUnknown;
        }
        /** @brief calculate station index from Identifier */
         MuonStationIndex::StIndex stationIndex(const Identifier& /*id*/) const override {
            return MuonStationIndex::StIndex::StUnknown;
        }
        /** @brief calculate phi index from Identifier (not supported for MDT hits) */
         MuonStationIndex::PhiIndex phiIndex(const Identifier& /*id*/) const override {
            return MuonStationIndex::PhiIndex::PhiUnknown;
        }
        /** @brief calculate detector region index from Identifier */
         MuonStationIndex::DetectorRegionIndex regionIndex(const Identifier& /*id*/) const override {
            return MuonStationIndex::DetectorRegionIndex::DetectorRegionUnknown;
        }
        /** @brief calculate layer index from Identifier */
         MuonStationIndex::LayerIndex layerIndex(const Identifier& /*id*/) const override {
            return MuonStationIndex::LayerIndex::LayerUnknown;
        }
        /** @brief calculate layer index from Identifier */
         MuonStationIndex::TechnologyIndex technologyIndex(const Identifier& /*id*/) const override {
            return MuonStationIndex::TechnologyIndex::TechnologyUnknown;
        }
        /** @brief Recieve all technologies in a station */
         const std::set<MuonStationIndex::TechnologyIndex>& 
                    technologiesInStation(MuonStationIndex::StIndex /*stIndex*/) const  {
                        static const std::set<MuonStationIndex::TechnologyIndex> emptySet;
                        return emptySet;
                    }
        /** @brief Return stationPhi for all technologies */
         int stationPhi(const Identifier& /*id*/) const override { return 0; }
        /** @brief Return stationEta for all technologies */
         int stationEta(const Identifier& /*id*/) const override { return 0; }
        /** @brief Return stationName for all technologies */
         int stationName(const Identifier& /*id*/) const override { return 0; }
        /** @brief Return the stationName string for all technologies*/
         std::string stationNameString(const Identifier& /*id*/) const  { return "TEST";}
        /** @brief Return stationRegion for all technologies */
         int stationRegion(const Identifier& /*id*/) const  { return 0;}
        /** @brief return sector number 1-16, odd=large, even=small */
         int sector(const Identifier& /*id*/) const override { return 0; }
        /** @brief returns whether the RPC identifiers are loaded */
         bool hasRPC() const  { return false; }
        /** @brief returns whether the sTGC identifiers are loaded */
         bool hasTGC() const  { return false; }
        /** @brief returns whether the MDT identifiers are loaded */
         bool hasMDT() const override { return false; }
        /** @brief returns whether the CSC identifiers are loaded */
         bool hasCSC() const  { return false; }
        /** @brief returns whether the sTGC identifiers are loaded */
         bool hasSTGC() const override { return false; }
        /** @brief returns whether the Mircomegas identifiers are loaded */
         bool hasMM() const override { return false; }
      };
}
namespace xAOD::Test{
    class TestObj{
        public:
            TestObj(const IdentifierHash& idHash):
                m_hash{idHash}{}
            IdentifierHash identifierHash() const {
                return m_hash;
            }
        private:
            IdentifierHash m_hash{};
    };
    class TestObjId {
        public:
            TestObjId(const Identifier& id):
                m_id{id}{}
            const Identifier& identify() const { return m_id; }
        private:
            Identifier m_id{};
    };
    using TestContainer = std::vector<std::unique_ptr<TestObj>>;
    using TestContainerId = std::vector<std::unique_ptr<TestObjId>>;
 }


 #define PRINT_MSG(MSG) \
   std::cout<<__func__<<"() - "<<__LINE__<<": "<<MSG<<std::endl;

#define PRINT_ERROR(MSG) \
   std::cerr<<__func__<<"() - "<<__LINE__<<": "<<MSG<<std::endl;

bool testViewsHashes(const std::vector<std::uint32_t>& hashRanges) {
    xAOD::Test::TestContainer testContainer{};
  
    for (std::uint32_t hash = 0 ; hash < hashRanges.size(); ++hash) {
        for (std::uint32_t i = 0; i < hashRanges[hash]; ++i) {
            testContainer.emplace_back(std::make_unique<xAOD::Test::TestObj>(IdentifierHash{hash}));
        }
    }
    const std::uint32_t allSize = std::accumulate(hashRanges.begin(), hashRanges.end(), 0);
    if (testContainer.size() != allSize) {
        PRINT_ERROR( "Test container size does not match expected size: " << testContainer.size() << " != " << allSize );
        return false;
    }
    xAOD::ChamberViewer viewer{testContainer};
    if (viewer.size() == 0){
        PRINT_ERROR( "ChamberViewer size is zero, expected non-zero size." );
        return false;
    }
    std::size_t idx{0}, totalSize{0};
    do{
        if (viewer.size() == 0) {
            PRINT_ERROR( "ChamberViewer size is zero in iteration " << idx );
            return false;
        }
        PRINT_MSG("ChamberViewer size: " << viewer.size() << " at index " << idx );
        totalSize += viewer.size();
        if (hashRanges[idx] != viewer.size()) {
            PRINT_ERROR( "Size mismatch at index " << idx << ": expected " << hashRanges[idx] 
                      << ", got " << viewer.size() );
            return false;
        }
    } while (viewer.next() && ++idx);
    if (totalSize != allSize) {
        PRINT_ERROR( "Total size mismatch: expected " << allSize << ", got " << totalSize );
        return false;
    }
    

    for (std::uint32_t hash = 0 ; hash < hashRanges.size(); ++hash) {
        if (!viewer.loadView(IdentifierHash{hash})){
            PRINT_ERROR( "Failed to load view for hash: " << hash );
            return false;
        }
        if (viewer.size() != hashRanges[hash]) {
            PRINT_ERROR( "View size mismatch for hash " << hash << ": expected " << hashRanges[hash] 
                      << ", got " << viewer.size() );
            return false;
        }
        PRINT_MSG( "Loaded view for hash: " << hash << " with size: " << viewer.size() );
    }
    PRINT_MSG( "Test for view ids with hashes passed!" );
    return true;
}

bool testViewsIds(const std::vector<std::uint32_t>& hashRanges) {


    xAOD::Test::TestContainerId testContainer{};
  
    for (std::uint32_t hash = 0 ; hash < hashRanges.size(); ++hash) {
        for (std::uint32_t i = 0; i < hashRanges[hash]; ++i) {
            testContainer.emplace_back(std::make_unique<xAOD::Test::TestObjId>(Identifier{hash}));
        }
    }
    const std::uint32_t allSize = std::accumulate(hashRanges.begin(), hashRanges.end(), 0);
    if (testContainer.size() != allSize) {
        PRINT_ERROR( "Test container size does not match expected size: " << testContainer.size() << " != " << allSize );
        return false;
    }
     ISvcLocator* pSvcLoc{nullptr};
    if (!Athena_test::initGaudi("StoreGate/StoreGate_jobOptions.txt", pSvcLoc)) {
        PRINT_ERROR( "This test can not be run" );
        return false;
    }
    auto testSvc = std::make_unique<Muon::Test::IdHelperSvcMock>("Test", pSvcLoc);

    xAOD::ChamberViewer viewer{testContainer, testSvc.get()};
    if (viewer.size() == 0){
        PRINT_ERROR( "ChamberViewer size is zero, expected non-zero size." );
        return false;
    }
    std::size_t idx{0}, totalSize{0};
    do{
        if (viewer.size() == 0) {
            PRINT_ERROR( "ChamberViewer size is zero in iteration " << idx );
            return false;
        }
        PRINT_MSG( "ChamberViewer size: " << viewer.size() << " at index " << idx );
        totalSize += viewer.size();
        if (hashRanges[idx] != viewer.size()) {
            PRINT_ERROR( "Size mismatch at index " << idx << ": expected " << hashRanges[idx] 
                      << ", got " << viewer.size() );
            return false;
        }
    } while (viewer.next() && ++idx);
    if (totalSize != allSize) {
        PRINT_ERROR( "Total size mismatch: expected " << allSize << ", got " << totalSize );
        return false;
    }
    

    for (std::uint32_t hash = 0 ; hash < hashRanges.size(); ++hash) {
        if (!viewer.loadView(Identifier{hash})){
            PRINT_ERROR( "Failed to load view for hash: " << hash );
            return false;
        }
        if (viewer.size() != hashRanges[hash]) {
            PRINT_ERROR( "View size mismatch for hash " << hash << ": expected " << hashRanges[hash] 
                      << ", got " << viewer.size() );
            return false;
        }
        PRINT_MSG( "Loaded view for hash: " << hash << " with size: " << viewer.size() );
    }
    PRINT_MSG( "All tests passed successfully!" );
    return true;
}

 int main () {
    std::vector<std::uint32_t> hashRanges{6,9,2,3,5,8,13,21,55};
    if (!testViewsHashes(hashRanges)) {
        return EXIT_FAILURE;
    }
    if (!testViewsIds(hashRanges)) {
        return EXIT_FAILURE;
    }
   
    return EXIT_SUCCESS;
 }