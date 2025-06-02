/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_PADDESIGN_H
#define MUONREADOUTGEOMETRYR4_PADDESIGN_H

#include <MuonReadoutGeometryR4/StripDesign.h>
#include "CxxUtils/ArrayHelper.h"
#include <GaudiKernel/SystemOfUnits.h>

namespace MuonGMR4{

/* The pad readout channels in sTgc chambers consists of 2D structure. Chambers may contain
 * different number of pads depending on the gasGap. Measurements in the eta direction for pads
 * are in millimeters, whereas for the phi direction, we have angular measurements in degrees.
 * same number of wires. Hence, the channel width & pitch vary across the board. The wire group 
 * PadDesign inherits from the StripDesign class and contains functions that can define the geometry
 * of pad layers. There are several functions to get the corners and center of each individual pads
 * w.r.t. the center of the chamber. Extra methods will be added according to the needs.
*/
class PadDesign;
using PadDesignPtr = GeoModel::TransientConstSharedPtr<PadDesign>;
    
class PadDesign: public StripDesign {
    public:
        PadDesign() = default;
        /// set sorting operator
        bool operator<(const PadDesign& other) const;

        /// Defines the Phi direction layout of the pad detector by specifing the starting angle w.r.t.
        /// the layer center, the number of pads in the phi direction, angular pitch to the next pad, 
        /// and finally, the staggering in mm for the inner edges of the pads. Note: the outer edges of  
        /// the pads with either phi = 1, or phi = numPadPhi are fixed, they do not stagger.
        void definePadRow(const double firstPadPhiDiv,
                                const int numPadPhi,
                                const double anglePadPhi,
                                const int padPhiShift);
        /// Defines the Eta direction layout of the pad detector by specifing the height of the first pad
        /// in millimeters, the number of pads in the eta direction, height of the pads other than the first row, 
        /// and finally, the maximum number of pads accomodated in a pad column which is 18, hardcoded. The numbering for
        /// the next pad column will start with a factor of 19, regardless of the number of pads in the previous column.
        void definePadColumn(const double firstPadHeight,
                                const int numPadEta,
                                const double padHeight,
                                const int maxPadEta = 18);
        /// Returns the total number of pads in a layer
        int numPads() const;                        
        /// Returns the angle of the first pad outer edge w.r.t. the gasGap center from the beamline
        double firstPadPhiDiv() const;
        /// Returns the number of pads in the Phi direction in the given gasGap layer
        int numPadPhi() const;
        /// Returns the angular pitch of the pads in the phi direction
        double anglePadPhi() const;
        /// Returns the staggering shift of inner pad edges in the phi direction
        double padPhiShift() const;
        /// Returns the height of the pads that are adjacent to the bottom edge of the trapezoid active area
        double firstPadHeight() const;
        /// Returns the number of pads in the eta direction in the given layer
        int numPadEta() const;
        /// Returns the height of all the pads that are not adjacent to the bottom edge of the trapezoid active area
        double padHeight() const;
        /// Returns the maximum number of pads that can be contained in a column of a pad. Used to match the pad numbering scheme
        int maxPadEta() const;
        /// Returns the pad (eta,phi) for a given pad number in sequence (1,2,3,...18,19,20...)
        std::pair<int, int> padNumber(const int SeqChannel) const;
        /// Returns a pair of Eta and Phi index for the given conventional pad number
        std::pair<int, int> padEtaPhi(const int channel) const;
        /// Returns the Eta index of the pad for the given conventional pad number
        int padEta(const int channel) const;
        /// Returns the Phi index of the pad for the given conventional pad number
        int padPhi(const int channel) const;
        /// Extracting the distance from gasGap center to beamline from the local to global transformation of the padLayer
        void defineBeamlineRadius(const double radius);
        /// Returns the distance between the gasGap center and the beamline
        double beamlineRadius() const;
        /// Defining an array of four vectors to store the pad corner position in the order of the enum defined below
        using localCornerArray = std::array<Amg::Vector2D, 4>;   
        enum  padCorners{ botLeft=0, botRight, topLeft, topRight };
        /// Returns an array of local pad corner positions given the sequential pad channel or the Eta/Phi Id.
        localCornerArray padCorners(const int channel) const;
        localCornerArray padCorners(const std::pair<int, int>& padEtaPhi) const;
        /// Override from stripDesign. This function will give the center of the pad by taking the sequential channel number as input
        Amg::Vector2D stripPosition(int stripNum) const override final;
        /// Function to retrieve the pad eta and phi given a local position coordinate
        std::pair<int, int> channelNumber(const Amg::Vector2D& hitPos) const;
        /// Function gives the maximum active area length available in the local x for the given hit local y
        double maxActiveX(const double locY) const;
        /// Function gives the angular width of the sector
        double sectorAngle() const;

    private:
        void print(std::ostream& ostr) const override final; 
        /// Angle of the first pad outer edge w.r.t. the gasGap center from the beamline
        double m_firstPadPhiDiv{0.};
        /// Number of pads in the Phi direction in the given gasGap layer
        int m_numPadPhi{0};
        /// Angular pitch of the pads in the phi direction
        double m_anglePadPhi{0.};
        /// The staggering shift of inner pad edges in the phi direction
        double m_padPhiShift{0.};
        /// Height of the pads that are adjacent to the bottom edge of the trapezoid active area
        double m_firstPadHeight{0.};
        /// Number of pads in the eta direction in the given layer
        int m_numPadEta{0};
        /// Height of all the pads that are not adjacent to the bottom edge of the trapezoid active area
        double m_padHeight{0.};
        /// The maximum number of pads that can be contained in a column of a pad. Used to match the pad numbering scheme
        int m_maxPadEta{18};
        /// Stores the beamline radius extracted from the local to global transformation
        double m_radius{0.};
        /// Large sector opening angle
        static constexpr double m_largeSectorAngle{28.};
        /// Small sector opening angle
        static constexpr double m_smallSectorAngle{17.};       
};

struct PadDesignSorter{
    bool operator()(const PadDesignPtr&a, const PadDesignPtr& b) const {
            return (*a) < (*b);
    }
    bool operator()(const PadDesign&a ,const PadDesign& b) const {
        return a < b;
    }
};

}
#include <MuonReadoutGeometryR4/PadDesign.icc>
#endif