/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONREADOUTGEOMETRY_TGCREADOUTPARAMS_H
#define MUONREADOUTGEOMETRY_TGCREADOUTPARAMS_H

// ******************************************************************************
// class TgcReadoutParams
// ******************************************************************************
//
// Description
// -----------
// Provides access to the TGC readout parameters.  Created from one of the
// "sources": TGC_ZebraDetDescrSource, TGC_NovaDetDescrSource, etc.  One
// object is instantiated per chamber type and is pointed to by the
// TGC_DetDescriptor object corresponding to that type.
//
// Inheritance
// -----------
// None.
//
// Dependencies
// ------------
// None.
//
// ******************************************************************************
#include <AthenaBaseComps/AthMessaging.h>
#include <CxxUtils/ArrayHelper.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <array>
#include <string>
#include <vector>


namespace MuonGM {

    class TgcReadoutParams : public AthMessaging {
    public:
        
        // Readout array sizes
        enum TgcReadoutArraySizes { MaxNGaps = 3, MaxNGangs = 180, MaxNStrips = 33 };
        using GasGapIntArray =  std::array<int, MaxNGaps>;
        using GasGapFloatArray = std::array<double, MaxNGaps>;
        using WiregangArray = std::array<int, MaxNGangs>;
        using StripArray = std::array<double, MaxNStrips>;
        /// @brief Default constructor used by the MuonTPCnv tests
        TgcReadoutParams();
        /// @brief Constructor implementing the strip layout per gasGap
        /// @param name: Name of the chamber type
        /// @param iCh: Chamber type index
        /// @param WireSp: Wire pitch
        /// @param NCHRNG: Number of phi sectors
        /// @param numWireGangs: Number of wire gangs in each gas gap
        /// @param IWGS1: Number of wires in each gang in gas gap 1
        /// @param IWGS2: Number of wires in each gang in gas gap 2
        /// @param IWGS3: Number of wires in each gang in gas gap 3
        /// @param PDIST: Distance of the strip frame to the bottom of the chamber
        /// @param SLARGE: Position of each strip mounting point on the top of the frame
        /// @param SSHORT: Position of each strip mounting point on the bottom of the frame
        /// @param numStrip: Array representing the number of strips in each gasgap
        TgcReadoutParams(const std::string& name, 
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
                         GasGapIntArray&& numStrips);

        ~TgcReadoutParams();

        inline const std::string& GetName() const;
        int chamberType() const;
        int nPhiChambers() const;
        int nGaps() const;

        /// Returns the wire pitch
        double wirePitch() const;
        ///
        inline double gangThickness() const;
        /// Returns the number of wire gangs
        int nWireGangs(int gasGap) const;
        /// Returns the total number of wires in a given gang
        int totalWires(int gasGap) const;
        /// Returns the number of wires in a given gang
        int nWires(int gasGap, int gang) const;
        /// Returns the sum of all wires from gang [1 - i) 
        int nSummedWires(int gasGap, int gang) const;
        /// Returns the number of wire pitches that have to be travelled to reach gang i
        double nPitchesToGang(int gasGap, int gang) const;

        // Access to strip parameters
        inline double stripThickness() const;
        int nStrips(int gasGap) const;

        double physicalDistanceFromBase() const;
        /// Returns the signed distance of the i-th's strip's left edge w.r.t 
        /// the center of the bottom chamber edge
        double stripPositionOnLargeBase(int strip, int gasGap) const;
        /// Returns the signed distance of the i-th's strip's left edge w.r.t.
        /// the center of the top chamber edge
        double stripPositionOnShortBase(int strip, int gasGap) const;
        /// Returns the signed distance along the chamber edge of the strip expressed at the 
        /// chamber center
        double stripCenter(int strip , int gasGap) const;
        /// Returns the number of defined strip layers
        int nStripLayers() const { return m_stripPositionCenter.size(); }
    private:
        // Data members
        std::string m_chamberName{};
        int m_chamberType{0};
        double m_wirePitch{0.};
        int m_nPhiChambers{0};
        
        /// Map of number of wires in a given wire gang & gas gap
        std::array<std::vector<int>, MaxNGaps> m_nWires{};
        /// Map describing the number of all wires up to gang i in gasgap j 
        std::array<std::vector<int>, MaxNGaps> m_nAccWires{};

        GasGapIntArray m_nStrips{make_array<int, MaxNGaps>(0)};
        GasGapIntArray m_totalWires{make_array<int, MaxNGaps>(0)};


        // strip postion on the bases for the first layer in +Z
        double m_physicalDistanceFromBase{-9999.};
        /// These 2 arrays represent the left edges of the i-th strip in a Tgc chamber
        /// The numbers are given as the signed distance along the chamber edge measured 
        /// from the center of the top edge (Large base) or of the bottom edge (Short base)
        std::vector<StripArray> m_stripPositionOnLargeBase{};
        std::vector<StripArray> m_stripPositionOnShortBase{};
        /// The position of the strip center is defined as the intersector of the large and short edge
        /// strip position values
        std::vector<StripArray> m_stripPositionCenter{};
        
        inline bool invalidGasGap(int gasGap) const{ return gasGap<1 or gasGap>MaxNGaps;}
        inline bool invalidGang(int gang) const{ return gang<1 or gang>MaxNGangs;}
        // Hard-coded data
        static constexpr double m_gangThickness = 0.05 * Gaudi::Units::mm;
        static constexpr double m_stripThickness = 0.03 * Gaudi::Units::mm;
    };
    double TgcReadoutParams::stripThickness() const { return m_stripThickness; }
    double TgcReadoutParams::gangThickness() const { return m_gangThickness; }

    const std::string& TgcReadoutParams::GetName() const { return m_chamberName; }
}  // namespace MuonGM

#endif  // MUONREADOUTGEOMETRY_TGCREADOUTPARAMS_H
