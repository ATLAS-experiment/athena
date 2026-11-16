/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Class to store and manipulate the information of patterns for RPCPatternMatching algorithm.
// Pattern structure:
// m_header: 16 bits, number of trigger sector (0-63)
// m_BIBM1: 32 bits, unique identifiers for BI and BM1 strips (16 bits each, BI in the higher 16 bits, BM1 in the lower 16 bits)
// m_BM2BO: 32 bits, unique identifiers for BM2 and BO strips (16 bits each, BM2 in the higher 16 bits, BO in the lower 16 bits)
// m_coordBIBM1: 32 bits, coordinates for BI and BM1 strips (12 bits each, BI in the higher 12 bits, BM1 in the middle 12 bits)
//              + 8 bits for pt (lower 8 bits)
// m_coordBM2BO: 32 bits, coordinates for BM2 and BO strips (12 bits each, BM2 in the higher 12 bits, BO in the middle 12 bits)
//              + 8 bits for pt (lower 8 bits)
// m_infos: 32 bits, additional information (eta bin: higher 5 bits, charge: next 1 bit, eta: next 14 bits, phi: next 9 bits, 3 bits reserved)
//          The decision for such structure, reflect a fragment of the final candidate format the L1Muon Barrel SL will provide.

#ifndef RPCMuonPattern_H
#define RPCMuonPattern_H 

#include "GaudiKernel/MsgStream.h"
#include <iomanip>
#include <cstdint>
#include <map> 
#include <cmath>

namespace L1Muon {
    class RPCMuonPattern {
        public:
        RPCMuonPattern() = default;

        RPCMuonPattern(const uint16_t header, const uint32_t BIBM1, const uint32_t BM2BO,
                            const uint32_t coordBIBM1, const uint32_t coordBM2BO,
                            const uint32_t infos) : 
                            m_header(header), m_BIBM1(BIBM1), m_BM2BO(BM2BO), 
                            m_coordBIBM1(coordBIBM1), m_coordBM2BO(coordBM2BO), 
                            m_infos(infos) {} ;

        RPCMuonPattern(const uint16_t header,
                                             const uint32_t packedBI, const uint32_t packedBM1, 
                                             const uint32_t packedBM2, const uint32_t packedBO,
                                             const uint32_t coordBI, const uint32_t coordBM1,
                                             const uint32_t coordBM2, const uint32_t coordBO,
                                             const uint32_t pt, const uint32_t packedInfos) : 
                            m_header(header),
                            m_BIBM1( (packedBI & m_lower16BitsMask) << m_highestStripShift | (packedBM1 & m_lower16BitsMask) ), // combine BI and BM1 into a single 32-bit value, take the lower 16 bits for both BI and BM1
                            m_BM2BO( (packedBM2 & m_lower16BitsMask) << m_highestStripShift | (packedBO & m_lower16BitsMask) ), // combine BM2 and BO into a single 32-bit value, take the lower 16 bits for both BM2 and BO
                            m_coordBIBM1( (coordBI & m_lower12BitsMask) << m_highestCoordShift | (coordBM1 & m_lower12BitsMask) << m_lowestCoordShift | (pt & m_lower8BitsMask)), // combine coordinates into a single 32-bit value, take the lower 8 bits for each coordinate
                            m_coordBM2BO( (coordBM2 & m_lower12BitsMask) << m_highestCoordShift | (coordBO & m_lower12BitsMask) << m_lowestCoordShift ), // combine coordinates into a single 32-bit value, take the lower 8 bits for each coordinate                            
                            m_infos(packedInfos) {}  ;

        RPCMuonPattern(const RPCMuonPattern& other, const uint32_t newInfos) :
                            m_header(other.m_header), m_BIBM1(other.m_BIBM1), m_BM2BO(other.m_BM2BO),
                            m_coordBIBM1(other.m_coordBIBM1), m_coordBM2BO(other.m_coordBM2BO),
                            m_infos(newInfos) {}  ;

        uint16_t getHeader() const { return m_header; };
        uint16_t getTriggerSector() const { return m_header; }; // same as getHeader()
        uint32_t getBIBM1() const { return m_BIBM1; };
        uint32_t getBM2BO() const { return m_BM2BO; };
        uint32_t getCoordBIBM1() const { return m_coordBIBM1; };
        uint32_t getCoordBM2BO() const { return m_coordBM2BO; };
        uint32_t getInfos() const { return m_infos; };
        uint32_t getStripBI() const { return m_BIBM1 >> m_highestStripShift; }; // get the BI strip from the packed BI-BM1 information
        uint32_t getStripBM1() const { return m_BIBM1 & m_lower16BitsMask; }; // get the BM1 strip from the packed BI-BM1 information
        uint32_t getStripBM2() const { return m_BM2BO >> m_highestStripShift; }; // get the BM2 strip from the packed BM2-BO information
        uint32_t getStripBO() const { return m_BM2BO & m_lower16BitsMask; }; // get the BO strip from the packed BM2-BO information
        uint32_t getCoordBI() const { return m_coordBIBM1 >> m_highestCoordShift; }; // get the BI coordinate from the packed coordinates
        uint32_t getCoordBM1() const { return (m_coordBIBM1 >> m_lowestCoordShift) & m_lower12BitsMask; }; // get the BM1 coordinate from the packed coordinates
        uint32_t getCoordBM2() const { return m_coordBM2BO >> m_highestCoordShift; }; // get the BM2 coordinate from the packed coordinates
        uint32_t getCoordBO() const { return (m_coordBM2BO >> m_lowestCoordShift) & m_lower12BitsMask; }; // get the BO coordinate from the packed coordinates
        uint32_t getPt() const { return m_coordBIBM1 & m_lower8BitsMask; }; // get the pt from the packed coordinates
        uint32_t getEtaBin() const { return m_infos >> m_etaBinShift; }; // get the eta bin region from the packed infos
        uint32_t getCharge() const { return (m_infos >> m_chargeShift) & m_lower1BitMask; }; // get the charge from the packed infos
        uint32_t getEta() const { return (m_infos >> m_etaShift) & m_lower14BitsMask; }; // get the eta from the packed infos
        uint32_t getPhi() const { return (m_infos >> m_phiShift) & m_lower9BitsMask; }; // get the phi from the packed infos

        float getEtaFloat() const { return static_cast<float>(getEta()) * 5.4f / 16383.0f - 2.7f; }; // convert the eta from 14 bits to float in the range [-2.7, 2.7]
        float getPhiFloat() const { return static_cast<float>(getPhi()) * (2.0f * static_cast<float>(M_PI)) / 511.0f; }; // convert the phi from 9 bits to float in the range [0, 2pi]
        float getPtFloat() const { return static_cast<float>(getPt()) * 127.0f / 255.0f; }; // convert the pt from 8 bits to float in the range [0, 127]
        float getCoordSign() const { return (getEtaFloat() >= 0.0f) ? 1.0f : -1.0f; }; // get the sign of the coordinate based on the eta value
        float getCoordBIFloat() const { return getCoordSign() * static_cast<float>(getCoordBI()) * 16368.0f / 1023.0f; }; // convert the BI coordinate from 10 bits to float in the range [0, 16368]
        float getCoordBM1Float() const { return getCoordSign() * static_cast<float>(getCoordBM1()) * 16368.0f / 1023.0f; }; // convert the BM1 coordinate from 10 bits to float in the range [0, 16368]
        float getCoordBM2Float() const { return getCoordSign() * static_cast<float>(getCoordBM2()) * 16368.0f / 1023.0f; }; // convert the BM2 coordinate from 10 bits to float in the range [0, 16368]
        float getCoordBOFloat() const { return getCoordSign() * static_cast<float>(getCoordBO()) * 16368.0f / 1023.0f; }; // convert the BO coordinate from 10 bits to float in the range [0, 16368]
        uint32_t getMaxThreshold() const {
            static const std::map<int, int> th_map = {{4, 0}, {5, 1}, {6, 2}, {7, 3}, {8, 4}, {9, 5}, {10, 6},
                                                    {12, 7}, {15, 8}, {18, 9}, {20, 10}, {25, 11}, {30, 12},
                                                    {40, 13}, {80, 14}, {100, 15}};
            // upper_bound gives first element > x
            auto it = th_map.upper_bound(std::round(getPtFloat()));
            if (it == th_map.begin()) { return -1; }
            --it;
            return uint32_t(it->first);   // returns threshold value
            // return it->second; // or ID, if that's what you want
        };

        int getPriority() const {
            // BI-BO
            if (((m_BIBM1 & m_lower16BitsMask) == m_nullBM1EtaStrip && (m_BM2BO >> m_highestStripShift) == m_nullBM2EtaStrip) ||
                ((m_BIBM1 & m_lower16BitsMask) == m_nullBM1PhiStrip && (m_BM2BO >> m_highestStripShift) == m_nullBM2PhiStrip)) { return 1; }
            // BI-BM1-BM2
            else if (((m_BM2BO & m_lower16BitsMask) == m_nullBOEtaStrip) ||
                    ((m_BM2BO & m_lower16BitsMask) == m_nullBOPhiStrip)) { return 2; }
            // BM1-BM2-BO
            else if (((m_BIBM1 >> m_highestStripShift) == m_nullBIEtaStrip) ||
                    ((m_BIBM1 >> m_highestStripShift) == m_nullBIPhiStrip)) { return 3; }
            // BI-BM1-BO
            else if (((m_BM2BO >> m_highestStripShift) == m_nullBM2EtaStrip) ||
                    ((m_BM2BO >> m_highestStripShift) == m_nullBM2PhiStrip)) { return 4; }
            // BI-BM2-BO
            else if (((m_BIBM1 & m_lower16BitsMask) == m_nullBM1EtaStrip) ||
                    ((m_BIBM1 & m_lower16BitsMask) == m_nullBM1PhiStrip)) { return 5; }
            else { return 0; } // no valid pattern
        };
        
        void print(MsgStream& msg) const {
            msg << "RPCMuonPattern: header: " << std::hex << m_header 
                      << ", BIBM1: " << m_BIBM1 
                      << ", BM2BO: " << m_BM2BO 
                      << ", coordBIBM1: " << m_coordBIBM1
                      << ", coordBM2BO: " << m_coordBM2BO
                      << ", infos: " << m_infos << std::dec << endmsg;
            msg << "  BI strip: " << getStripBI()
                      << ", BM1 strip: " << getStripBM1()
                      << ", BM2 strip: " << getStripBM2()
                      << ", BO strip: " << getStripBO() << endmsg;
        };

        private:

        uint16_t m_header{0};
        uint32_t m_BIBM1{0};
        uint32_t m_BM2BO{0};
        uint32_t m_coordBIBM1{0};
        uint32_t m_coordBM2BO{0};
        uint32_t m_infos{0};

        // Constants for bit manipulation
        //// Masks for extracting specific bits from the packed data
        static constexpr uint32_t m_lower16BitsMask = 0x0000FFFF;
        static constexpr uint32_t m_lower12BitsMask = 0x00000FFF;
        static constexpr uint32_t m_lower8BitsMask = 0x000000FF;
        static constexpr uint32_t m_lower1BitMask = 0x00000001;
        static constexpr uint32_t m_lower14BitsMask = 0x00003FFF;
        static constexpr uint32_t m_lower9BitsMask = 0x000001FF;

        //// Shift values for packing and unpacking the data
        static constexpr int m_highestStripShift = 16;
        static constexpr int m_highestCoordShift = 20;
        static constexpr int m_lowestCoordShift = 8;
        static constexpr int m_etaBinShift = 27;
        static constexpr int m_chargeShift = 26;
        static constexpr int m_etaShift = 12;
        static constexpr int m_phiShift = 3;

        // Null values for each station and strip type
        // These values are used to indicate that a particular station or strip type is not present
        static constexpr uint16_t m_nullBIEtaStrip = 0x0000;
        static constexpr uint16_t m_nullBM1EtaStrip = 0x4000;
        static constexpr uint16_t m_nullBM2EtaStrip = 0x8000;
        static constexpr uint16_t m_nullBOEtaStrip = 0xc000;
        static constexpr uint16_t m_nullBIPhiStrip = 0x0001;
        static constexpr uint16_t m_nullBM1PhiStrip = 0x4001;
        static constexpr uint16_t m_nullBM2PhiStrip = 0x8001;
        static constexpr uint16_t m_nullBOPhiStrip = 0xc001;

    };
} // end of namespace L1Muon
#endif // RPCMuonPattern_H