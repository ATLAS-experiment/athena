/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

#include "MuonStationIndex/MuonStationIndex.h"
#include "L1MuonS1RPC/RPCSLStripConverter.h"

#define GREEN_TEXT "\033[32m"
#define RED_TEXT   "\033[31m"
#define RESET_TEXT "\033[0m"

namespace {
    void assert_true(bool condition, const std::string& testName, int& passCount) {
        if (!condition) {
            std::cerr << RED_TEXT << "[FAIL] " << testName << RESET_TEXT << std::endl;
            std::abort();
        }
        ++passCount;
    }
}

void test_triggerSector_composition(int& passCount) {
    std::cout << "[RUN] Testing trigger sector packing and mapping (0..63)..." << std::endl;

    for (uint8_t side = 0; side <= 1; ++side) {
        for (uint8_t sector = 0; sector <= 15; ++sector) {
            for (uint8_t doubletPhi = 0; doubletPhi <= 1; ++doubletPhi) {
                
                uint16_t expectedTrigSec = (side << 5) | (sector << 1) | doubletPhi;

                uint32_t packed = L1Muon::RPCSLStripConverter::pack(
                    side != 0,
                    sector,
                    doubletPhi != 0,
                    Muon::MuonStationIndex::StIndex::BM,
                    /*doubletR=*/1,
                    /*chamber=*/0,
                    /*doubletZ=*/0,
                    /*strip=*/0,
                    /*measuresPhi=*/false
                );

                uint16_t unpackedTrigSec = L1Muon::RPCSLStripConverter::getTriggerSector(packed);
                assert_true(unpackedTrigSec == expectedTrigSec, 
                    "Trigger sector mismatch: expected " + std::to_string(expectedTrigSec) + 
                    ", got " + std::to_string(unpackedTrigSec), passCount);

                assert_true(L1Muon::RPCSLStripConverter::getSide(packed) == side, 
                    "Side unpacked incorrectly: expected " + std::to_string(side) + 
                    ", got " + std::to_string(L1Muon::RPCSLStripConverter::getSide(packed)), passCount);

                assert_true(L1Muon::RPCSLStripConverter::getDoubletPhi(packed) == doubletPhi + 1, 
                    "DoubletPhi unpacked incorrectly", passCount);

                assert_true(L1Muon::RPCSLStripConverter::getSector(packed) == sector + 1, 
                    "Sector unpacked incorrectly", passCount);
            }
        }
    }
    std::cout << GREEN_TEXT << "  Passed trigger sector composition tests." << RESET_TEXT << std::endl;
}

void test_bor_bom_special_handling(int& passCount) {
    std::cout << "[RUN] Testing BOR/BOM (BO, doubletR=1, sector 11 & 15)..." << std::endl;

    std::vector<uint8_t> borSectors = {10, 14}; // 0-based: sectors 11 and 15
    for (uint8_t sector : borSectors) {
        for (uint8_t strip = 0; strip <= 63; ++strip) {
            uint32_t packed = L1Muon::RPCSLStripConverter::pack(
                /*side=*/false,
                sector,
                /*doubletPhi=*/false,
                Muon::MuonStationIndex::StIndex::BO,
                /*doubletR=*/0,
                /*chamber=*/2,
                /*doubletZ=*/1,
                strip,
                /*measuresPhi=*/true
            );

            assert_true(L1Muon::RPCSLStripConverter::getStation(packed) == L1Muon::SLRPCStation::BI_BOR_BOM,
                "BOR/BOM station not remapped to BI_BOR_BOM in pack()", passCount);

            assert_true(L1Muon::RPCSLStripConverter::getRPCStation(packed) == Muon::MuonStationIndex::StIndex::BO,
                "getRPCStation failed to identify BOR/BOM as StIndex::BO", passCount);

            assert_true(L1Muon::RPCSLStripConverter::getDoubletR(packed) == 1,
                "getDoubletR failed for BOR/BOM: expected 1, got " + 
                std::to_string(L1Muon::RPCSLStripConverter::getDoubletR(packed)), passCount);

            uint8_t unpackedStrip = L1Muon::RPCSLStripConverter::getStrip(packed);
            assert_true(unpackedStrip == strip + 1,
                "BOR/BOM strip unpacking mismatch: expected " + std::to_string(strip + 1) + 
                ", got " + std::to_string(unpackedStrip), passCount);
        }
    }
    std::cout << GREEN_TEXT << "  Passed BOR/BOM station tests." << RESET_TEXT << std::endl;
}

void test_roundtrip_all_fields(int& passCount) {
    std::cout << "[RUN] Testing round-trip pack/unpack across all standard stations..." << std::endl;

    struct TestCase {
        bool side;
        uint8_t sector;
        bool doubletPhi;
        Muon::MuonStationIndex::StIndex station;
        uint8_t doubletR;
        uint8_t chamber;
        uint8_t doubletZ;
        uint8_t strip;
        bool measuresPhi;
    };

    std::vector<TestCase> cases = {
        {false, 0, false, Muon::MuonStationIndex::StIndex::BI, 0, 0, 0, 0, false},
        {true, 5, true, Muon::MuonStationIndex::StIndex::BM, 1, 7, 1, 32, true},
        {false, 8, false, Muon::MuonStationIndex::StIndex::BO, 2, 10, 2, 63, false},
        {true, 15, true, Muon::MuonStationIndex::StIndex::BO, 2, 15, 2, 63, true}
    };

    for (const auto& c : cases) {
        uint32_t packed = L1Muon::RPCSLStripConverter::pack(
            c.side, c.sector, c.doubletPhi, c.station, c.doubletR, c.chamber, c.doubletZ, c.strip, c.measuresPhi
        );

        bool outSide{false};
        uint8_t outSector{0};
        uint8_t outDoubletPhi{0};
        Muon::MuonStationIndex::StIndex outStation{Muon::MuonStationIndex::StIndex::BI};
        uint8_t outChamber{0};
        uint8_t outDoubletZ{0};
        uint8_t outStrip{0};
        bool outMeasuresPhi{false};

        L1Muon::RPCSLStripConverter::unpack(
            packed, outSide, outSector, outDoubletPhi, outStation, outChamber, outDoubletZ, outStrip, outMeasuresPhi
        );

        assert_true(outSide == c.side, "Unpack mismatch on side", passCount);
        assert_true(outSector == c.sector + 1, "Unpack mismatch on sector (+1 expected)", passCount);
        assert_true(outDoubletPhi == int(c.doubletPhi)+1, "Unpack mismatch on doubletPhi", passCount);
        assert_true(outStation == c.station, "Unpack mismatch on station", passCount);
        assert_true(outChamber == c.chamber + 1, "Unpack mismatch on chamber (+1 expected)", passCount);
        assert_true(outDoubletZ == c.doubletZ + 1, "Unpack mismatch on doubletZ (+1 expected)", passCount);
        assert_true(outStrip == c.strip + 1, "Unpack mismatch on strip (+1 expected)", passCount);
        assert_true(outMeasuresPhi == c.measuresPhi, "Unpack mismatch on measuresPhi", passCount);
    }

    std::cout << GREEN_TEXT << "  Passed round-trip tests." << RESET_TEXT << std::endl;
}

void test_bitfield_masks(int& passCount) {
    std::cout << "[RUN] Testing mask invariants and boundary isolation..." << std::endl;

    uint32_t packed = L1Muon::RPCSLStripConverter::pack(
        true, 15, true,
        Muon::MuonStationIndex::StIndex::BO, 2,
        15, 2, 127, true
    );

    uint32_t higherBits = packed & (~0x3FFFFFu);
    assert_true(higherBits == 0, "High bits above bit 21 were unintentionally set", passCount);

    uint32_t withoutStrip = packed & L1Muon::RPCSLStripConverter::s_maskStripNumber;
    uint32_t stripBitsOnly = (packed ^ withoutStrip);
    assert_true((stripBitsOnly >> 1) == 127, "s_maskStripNumber failed to isolate strip bits", passCount);

    std::cout << GREEN_TEXT << "  Passed bitfield mask isolation tests." << RESET_TEXT << std::endl;
}

int main() {
    std::cout << "===========================================" << std::endl;
    std::cout << " Running RPCSLStripConverter Unit Test     " << std::endl;
    std::cout << "===========================================" << std::endl;

    int passCount = 0;

    test_triggerSector_composition(passCount);
    test_bor_bom_special_handling(passCount);
    test_roundtrip_all_fields(passCount);
    test_bitfield_masks(passCount);

    std::cout << "===========================================" << std::endl;
    std::cout << GREEN_TEXT << " ALL " << passCount << " ASSERTIONS PASSED SUCCESSFULLY" << RESET_TEXT << std::endl;
    std::cout << "===========================================" << std::endl;

    return 0;
}