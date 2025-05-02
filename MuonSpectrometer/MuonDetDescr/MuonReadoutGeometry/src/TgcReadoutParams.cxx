/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometry/TgcReadoutParams.h"

#include <utility>


namespace MuonGM {
    TgcReadoutParams::TgcReadoutParams():
        AthMessaging{"TgcReadoutParams"}{}
    TgcReadoutParams::TgcReadoutParams(const std::string& name, 
                                       int iCh, 
                                       int Version, 
                                       double WireSp, 
                                       const int NCHRNG, 
                                       GasGapIntArray && numWireGangs,
                                       WiregangArray && IWGS1, 
                                       WiregangArray && IWGS2, 
                                       WiregangArray && IWGS3,                                        
                                       GasGapIntArray && numStrips):
        AthMessaging{"TgcRadoutParams - "+name},
            m_chamberName(name), 
            m_chamberType(iCh), 
            m_readoutVersion(Version), 
            m_wirePitch(WireSp), 
            m_nPhiChambers(NCHRNG),
            m_nStrips{std::move(numStrips)} {
        
        for (int iGap =0 ; iGap < MaxNGaps; ++iGap){
            m_nWires[iGap].resize(numWireGangs[iGap]);
            m_nAccWires[iGap].resize(numWireGangs[iGap]);
        }
        for (int iGang = 0; iGang < MaxNGangs; ++iGang) {
            if (iGang < numWireGangs[0]) {
                m_nWires[0][iGang] = IWGS1[iGang];
                m_totalWires[0] += IWGS1[iGang];
            }
            if (iGang < numWireGangs[1]) {
                m_nWires[1][iGang] = IWGS2[iGang];
                m_totalWires[1] += IWGS2[iGang];
            }
            if (iGang < numWireGangs[2]) {
                m_nWires[2][iGang] = IWGS3[iGang];
                m_totalWires[2] += IWGS3[iGang];
            }
        }
        for (size_t iGap = 0; iGap < m_nWires.size(); ++iGap) {
            // Grap the total wires in the gasGap
            const int totWires = totalWires(iGap + 1);
            int accumWires = totWires;
            for (int iGang = m_nWires[iGap].size() - 1; iGang >= 0; --iGang) {
                accumWires -= m_nWires[iGap][iGang];
                m_nAccWires[iGap][iGang] = accumWires;
            }
        }
    }
    TgcReadoutParams::TgcReadoutParams(const std::string& name, 
                                       int iCh, 
                                       int Version, 
                                       double WireSp, 
                                       const int NCHRNG, 
                                       GasGapIntArray && numWireGangs,
                                       WiregangArray&& IWGS1, 
                                       WiregangArray&& IWGS2, 
                                       WiregangArray&& IWGS3,
                                       double PDIST, 
                                       StripArray&& SLARGE, 
                                       StripArray&& SSHORT,                         
                                       GasGapIntArray&& numStrips):
        TgcReadoutParams(name, iCh, Version, WireSp, NCHRNG, std::move(numWireGangs), 
                        std::move(IWGS1), std::move(IWGS2), std::move(IWGS3),
                        std::move(numStrips)){
      
        m_physicalDistanceFromBase = PDIST;
        m_stripPositionOnLargeBase = std::move(SLARGE);
        m_stripPositionOnShortBase = std::move(SSHORT);
        for (size_t s = 0 ; s < m_stripPositionOnLargeBase.size() - 1; ++s) {
            /// Position of the left strip's left edge at the chamber chamber center + the one of the
            /// right edge
            m_stripPositionCenter[s] = 0.25 *( m_stripPositionOnLargeBase[s] + m_stripPositionOnShortBase[s] +
                                            m_stripPositionOnLargeBase[s+1] + m_stripPositionOnShortBase[s+1]);
        }

    }

    TgcReadoutParams::~TgcReadoutParams() = default;

    // Access to general parameters

    int TgcReadoutParams::chamberType() const { return m_chamberType; }
    int TgcReadoutParams::readoutVersion() const { return m_readoutVersion; }
    int TgcReadoutParams::nPhiChambers() const { return m_nPhiChambers; }
    int TgcReadoutParams::nGaps() const { return 2 + (nStrips(3) > 1); }
    // Access to wire gang parameters
    double TgcReadoutParams::wirePitch() const { return m_wirePitch; }

    int TgcReadoutParams::nWireGangs(int gasGap) const {
        if (invalidGasGap(gasGap)) {
            ATH_MSG_FATAL(__func__<<":"<<__LINE__<<"nWireGangs(" << gasGap << ") gasGap out of allowed range: 1-" << MaxNGaps );
            throw std::out_of_range("input gas gap index is incorrect");
        }
        return m_nWires[gasGap - 1].size();
    }

    int TgcReadoutParams::totalWires(int gasGap) const {
        if (invalidGasGap(gasGap)) {
            ATH_MSG_FATAL(__func__<<":"<<__LINE__<<"(" << gasGap << ") gasGap out of allowed range: 1-" << MaxNGaps );
            throw std::out_of_range("input gas gap index is incorrect");
        }
        return m_totalWires[gasGap - 1];
    }

    int TgcReadoutParams::nWires(int gasGap, int gang) const {
        if (invalidGasGap(gasGap) or invalidGang(gang)) {
            ATH_MSG_FATAL( __func__<<":"<<__LINE__<<" gasGap " << gasGap << " or gang " << gang << " out of allowed range" );
            throw std::out_of_range("input gas gap or wire gang index are incorrect");
        }
        return m_nWires[gasGap - 1][gang - 1];
    }
    int TgcReadoutParams::nSummedWires(int gasGap, int gang) const {
        if (invalidGasGap(gasGap) or invalidGang(gang)) {
            ATH_MSG_FATAL( __func__<<":"<<__LINE__<<" gasGap " << gasGap << " or gang " << gang << " out of allowed range" );
            throw std::out_of_range("input gas gap or wire gang index are incorrect");
        }
        return m_nAccWires[gasGap -1 ][gang - 1];
    }
    double TgcReadoutParams::nPitchesToGang(int gasGap, int gang) const {
        if (invalidGasGap(gasGap) or invalidGang(gang)) {
            ATH_MSG_FATAL( __func__<<":"<<__LINE__<<" gasGap " << gasGap << " or gang " << gang << " out of allowed range" );
            throw std::out_of_range("input gas gap or wire gang index are incorrect");
        }
        const double nPit = 1.*m_nAccWires[gasGap -1][gang - 1] +
                            0.5*(m_nWires[gasGap-1][gang-1] -1) -
                            0.5*m_totalWires[gasGap -1];
        return nPit;   
    }
    // Access to strip parameters
    int TgcReadoutParams::nStrips(int gasGap) const {
        if (invalidGasGap(gasGap)) {
            ATH_MSG_FATAL( __func__<<":"<<__LINE__<<"(" << gasGap << ") gasGap out of allowed range: 1-" << MaxNGaps );
            throw std::out_of_range("input gas gap index is incorrect");
        }
        return m_nStrips[gasGap - 1];
    }
    double TgcReadoutParams::physicalDistanceFromBase() const { return m_physicalDistanceFromBase; }

    double TgcReadoutParams::stripPositionOnLargeBase(int istrip) const {
        // all gas gaps have the same n. of strips (=> check the first one)
        if (istrip > m_nStrips[0] + 1 || istrip < 1){
          ATH_MSG_FATAL(__func__<<"() "<<__LINE__<<" - Input strip n. " << istrip
              << " out of range in TgcReadoutParams::stripPositionOnLargeBase for TgcReadoutParams of name/type " << m_chamberName << "/"
              << m_chamberType << "  - Nstrips = " << m_nStrips[0] << " MaxNStrips = " << MaxNStrips );
          throw std::out_of_range("invalid strip index");
        }
        return m_stripPositionOnLargeBase[istrip - 1];
    }
    double TgcReadoutParams::stripPositionOnShortBase(int istrip) const {
        // all gas gaps have the same n. of strips (=> check the first one)
        if (istrip > m_nStrips[0] + 1 || istrip < 1) {
          ATH_MSG_FATAL(__func__<<"() "<<__LINE__<<" - Input strip n. " << istrip
                  << " out of range in TgcReadoutParams::stripPositionOnShortBase for TgcReadoutParams of name/type " << m_chamberName << "/"
                  << m_chamberType << "  - Nstrips = " << m_nStrips[0] << " MaxNStrips = " << MaxNStrips );
          throw std::out_of_range("invalid strip index");
        }
        return m_stripPositionOnShortBase[istrip - 1];
    }
    
    double TgcReadoutParams::stripCenter(int istrip) const {
        if (istrip > m_nStrips[0] + 1) {
          ATH_MSG_FATAL( "Input strip n. " << istrip
              << " out of range in TgcReadoutParams::stripPositionOnLargeBase for TgcReadoutParams of name/type " << m_chamberName << "/"
              << m_chamberType << "  - Nstrips = " << m_nStrips[0] << " MaxNStrips = " << MaxNStrips );
          throw std::out_of_range("invalid strip index");
        }
        return m_stripPositionCenter[istrip -1];
    }
}  // namespace MuonGM
