/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometry/TgcReadoutParams.h"
#include "GeoModelKernel/throwExcept.h"

#include <utility>


namespace MuonGM {
    TgcReadoutParams::TgcReadoutParams():
        AthMessaging{"TgcReadoutParams"}{}
    TgcReadoutParams::TgcReadoutParams(const std::string& name, 
                                       int iCh, 
                                       double WireSp, 
                                       const int NCHRNG, 
                                       GasGapIntArray && numWireGangs,
                                       WiregangArray&& IWGS1, 
                                       WiregangArray&& IWGS2, 
                                       WiregangArray&& IWGS3,
                                       double PDIST, 
                                       std::vector<StripArray>&& SLARGE, 
                                       std::vector<StripArray>&& SSHORT,
                                       GasGapIntArray&& numStrips):

        AthMessaging{"TgcReadoutParams - "+name},
            m_chamberName{name}, 
            m_chamberType{iCh}, 
            m_wirePitch{WireSp}, 
            m_nPhiChambers{NCHRNG},
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
      
        m_physicalDistanceFromBase = PDIST;
        m_stripPositionOnLargeBase = std::move(SLARGE);
        m_stripPositionOnShortBase = std::move(SSHORT);
        m_stripPositionCenter.resize(m_stripPositionOnShortBase.size());
        for (size_t l = 0; l < m_stripPositionOnLargeBase.size(); ++l) {
            for (size_t s = 0 ; s < m_stripPositionOnLargeBase[l].size() - 1; ++s) {
               /// Position of the left strip's left edge at the chamber chamber center + the one of the
                /// right edge
                m_stripPositionCenter[l][s] = 0.25 *( m_stripPositionOnLargeBase[l][s] + m_stripPositionOnShortBase[l][s] +
                                                      m_stripPositionOnLargeBase[l][s+1] + m_stripPositionOnShortBase[l][s+1]);
            }
        }
    }

    TgcReadoutParams::~TgcReadoutParams() = default;

    // Access to general parameters

    int TgcReadoutParams::chamberType() const { return m_chamberType; }
    int TgcReadoutParams::nPhiChambers() const { return m_nPhiChambers; }
    int TgcReadoutParams::nGaps() const { return 2 + (nStrips(3) > 1); }
    // Access to wire gang parameters
    double TgcReadoutParams::wirePitch() const { return m_wirePitch; }

    int TgcReadoutParams::nWireGangs(int gasGap) const {
        if (invalidGasGap(gasGap)) {
            THROW_EXCEPTION("gasGap "<<gasGap<<" is out of allowed range: 1-" << MaxNGaps );
        }
        return m_nWires[gasGap - 1].size();
    }

    int TgcReadoutParams::totalWires(int gasGap) const {
        if (invalidGasGap(gasGap)) {
            THROW_EXCEPTION("gasGap "<<gasGap<<" is out of allowed range: 1-" << MaxNGaps );
        }
        return m_totalWires[gasGap - 1];
    }

    int TgcReadoutParams::nWires(int gasGap, int gang) const {
        if (invalidGasGap(gasGap) or invalidGang(gang)) {
            THROW_EXCEPTION( " gasGap " << gasGap << " or gang " << gang << " out of allowed range" );           
        }
        return m_nWires[gasGap - 1][gang - 1];
    }
    int TgcReadoutParams::nSummedWires(int gasGap, int gang) const {
        if (invalidGasGap(gasGap) or invalidGang(gang)) {
            THROW_EXCEPTION( " gasGap " << gasGap << " or gang " << gang << " out of allowed range" );
        }
        return m_nAccWires[gasGap -1 ][gang - 1];
    }
    double TgcReadoutParams::nPitchesToGang(int gasGap, int gang) const {
        if (invalidGasGap(gasGap) or invalidGang(gang)) {
            THROW_EXCEPTION( " gasGap " << gasGap << " or gang " << gang << " out of allowed range" );
        }
        const double nPit = 1.*m_nAccWires[gasGap -1][gang - 1] +
                            0.5*(m_nWires[gasGap-1][gang-1] -1) -
                            0.5*m_totalWires[gasGap -1];
        return nPit;   
    }
    // Access to strip parameters
    int TgcReadoutParams::nStrips(int gasGap) const {
        if (invalidGasGap(gasGap)) {
            THROW_EXCEPTION("gasGap "<<gasGap<<" is out of allowed range: 1-" << MaxNGaps );
        }
        return m_nStrips[gasGap - 1];
    }
    double TgcReadoutParams::physicalDistanceFromBase() const { return m_physicalDistanceFromBase; }

    double TgcReadoutParams::stripPositionOnLargeBase(int istrip, int gasGap) const {
        // all gas gaps have the same n. of strips (=> check the first one)
        if (istrip > m_nStrips[0] + 1 || istrip < 1){
          THROW_EXCEPTION("Input strip n. " << istrip
              << " out of range in TgcReadoutParams::stripPositionOnLargeBase for TgcReadoutParams of name/type " << m_chamberName << "/"
              << m_chamberType << "  - Nstrips = " << m_nStrips[0] << " MaxNStrips = " << MaxNStrips );
        }
        if (nStripLayers() > 1){ 
            if( gasGap > nStripLayers() || gasGap == 0) {
                THROW_EXCEPTION("Input gasGap n. "<<gasGap<<" is out of the allowed range [1-"<<nStripLayers()<<"]");
            }
        } else {
            gasGap = 1;
        }
        return m_stripPositionOnLargeBase[gasGap -1][istrip - 1];
    }
    double TgcReadoutParams::stripPositionOnShortBase(int istrip, int gasGap) const {
        // all gas gaps have the same n. of strips (=> check the first one)
        if (istrip > m_nStrips[0] + 1 || istrip < 1) {
          THROW_EXCEPTION(__func__<<"() "<<__LINE__<<" - Input strip n. " << istrip
                  << " out of range in TgcReadoutParams::stripPositionOnShortBase for TgcReadoutParams of name/type " << m_chamberName << "/"
                  << m_chamberType << "  - Nstrips = " << m_nStrips[0] << " MaxNStrips = " << MaxNStrips );
        }
        if (nStripLayers() > 1  || gasGap == 0){ 
            if( gasGap > nStripLayers()) {
                THROW_EXCEPTION("Input gasGap n. "<<gasGap<<" is out of the allowed range [1-"<<nStripLayers()<<"]");
            }
        } else {
            gasGap = 1;
        }
        return m_stripPositionOnShortBase[gasGap-1][istrip - 1];
    }
    
    double TgcReadoutParams::stripCenter(int istrip, int gasGap) const {
        if (istrip > m_nStrips[0] + 1) {
            THROW_EXCEPTION("Input strip n. " << istrip
              << " out of range in TgcReadoutParams::stripPositionOnLargeBase for TgcReadoutParams of name/type " << m_chamberName << "/"
              << m_chamberType << "  - Nstrips = " << m_nStrips[0] << " MaxNStrips = " << MaxNStrips );
        }
        if (nStripLayers() > 1){ 
            if(gasGap > nStripLayers() || gasGap == 0) {
                THROW_EXCEPTION("Input gasGap n. "<<gasGap<<" is out of the allowed range [1-"<<nStripLayers()<<"]");
            }
        } else {
            gasGap = 1;
        }
        return m_stripPositionCenter[gasGap-1][istrip -1];
    }
}  // namespace MuonGM
