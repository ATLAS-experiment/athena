/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 properties of a plane based detector allowing for a stereo angle
 ----------------------------------------------------------------------
***************************************************************************/

#ifndef MUONREADOUTGEOMETRY_MUONPADDESIGN_H
#define MUONREADOUTGEOMETRY_MUONPADDESIGN_H

#include <cmath>
#include <utility>
#include <vector>

#include "GeoPrimitives/GeoPrimitives.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "CxxUtils/ArrayHelper.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "AthenaBaseComps/AthMessaging.h"
namespace MuonGM {

    /// Parameters defining the design of the readout sTGC pads
    /**
       The parameters below are the ones from the parameter book.  The
       naming convention used here refers to the one explained on p.90
       ("naming of .xml tags and .h variables") of
       https://twiki.cern.ch/twiki/bin/view/Atlas/NSWParameterBook#Parameter_book

       Note that the pads do not exist as objects in memory. Instead, a
       MuonPadDesign is defined for each layer in a module. From the
       readout parameters one can map position <-> channel:
       - for each sim hit position identify the pad (channel)
       - for each channel determine the center position
    */
    struct MuonPadDesign: public AthMessaging {
    public:
        MuonPadDesign();
        int padEtaMin{0};
        int padEtaMax{0};

        double inputRowPitch{0.};
        double inputPhiPitch{0.};
        double signY{0.};
        double firstRowPos{0.};
        double firstPhiPos{0.};

        double Length{0.};
        double sWidth{0.};
        double lWidth{0.};
        double Size{0.};
        double thickness{0.};
        double radialDistance{0.};  ///< DT-2015-11-29 distance from the beamline to the center of the module

        double sPadWidth{0.};
        double lPadWidth{0.};
        double xFrame{0.};
        double ysFrame{0.};
        double ylFrame{0.};
        double yCutout{0.};
        int nPadH{0};
        int nPadColumns{0};
        double PadPhiShift{0.};
        int etasign{0};
        int isLargeSector{0};
        double sectorOpeningAngle{0.};
        bool isConvertedFromPhaseII{false};
        static constexpr double largeSectorOpeningAngle{28.0};
        static constexpr double smallSectorOpeningAngle{17.0};

        std::pair<int, int> etaPhiId(const int channel) const;
        int channelId(const std::pair<int, int>& padId) const;


        /** distance to channel - residual */
        double distanceToChannel(const Amg::Vector2D& pos, bool measPhi, int channel = 0) const;
        
        Amg::Vector2D distanceToPad(const Amg::Vector2D& pos, int channel) const;

        /// whether pos is within the sensitive area of the module
        bool withinSensitiveArea(const Amg::Vector2D& pos) const;
        /// lowest y (local) of the sensitive volume
        double minSensitiveY() const;
        /// highest y (local) of the sensitive volume
        double maxSensitiveY() const;
        /// largest (abs, local) x of the sensitive volume
        double maxAbsSensitiveX(const double& y) const;

        /** calculate local channel number, range 1=nstrips like identifiers. Returns -1 if out of range */
        std::pair<int, int> channelNumber(const Amg::Vector2D& pos) const;

        /** calculate local channel position for a given channel number */
        bool channelPosition(const std::pair<int, int>& pad, Amg::Vector2D& pos) const;
        bool channelPosition(const int channel, Amg::Vector2D& pos) const;

        /** calculate local channel corners for a given channel number */
        using CornerArray = std::array<Amg::Vector2D, 4>;   
        enum  padCorners{ botLeft=0, botRight, topLeft, topRight };
        bool channelCorners(const std::pair<int, int>& pad, CornerArray& corners) const;
        bool channelCorners(const int channel, CornerArray& corners) const;
        /** calculate local channel width */
        double channelWidth(const Amg::Vector2D& pos, bool measPhi, bool preciseMeas = false) const;

        /** thickness of gas gap */
        double gasGapThickness() const;

        /** access to cache */
        void setR(double R) { this->radialDistance = R; }
    };

    inline std::pair<int, int> MuonPadDesign::etaPhiId(const int channel) const {        
        return std::make_pair( ((channel -1) % 18) + 1, ( (channel -1) / 18) + 1);
    }
    inline int MuonPadDesign::channelId(const std::pair<int, int>& padId) const {
        return 1 + (padId.first - 1)  + (padId.second - 1 ) * 18;
    }

    inline double MuonPadDesign::distanceToChannel(const Amg::Vector2D& pos, bool measPhi, int channel) const {
        // if channel number not provided, get the nearest channel ( mostly for validation purposes )

        std::pair<int, int> pad{};
        if (channel < 1) {  // retrieve nearest pad indices
            pad = channelNumber(pos);
        } else {  // hardcode - or add a member saving idHelper max
            pad = etaPhiId(channel);
        }

        Amg::Vector2D chPos{Amg::Vector2D::Zero()};
        if (!channelPosition(pad, chPos)) return -10000.;

        if (!measPhi) return (pos.y() - chPos.y());

        // the "phi" distance to be compared with channel width (taking into account the stereo angle)

        return (pos.x() - chPos.x());
    }

    inline double MuonPadDesign::channelWidth(const Amg::Vector2D& pos, bool measPhi, bool preciseMeas) const {
        // get Eta and Phi indices, and corner positions of given pad
        const std::pair<int, int>& pad = channelNumber(pos);
        std::array<Amg::Vector2D,4> corners{make_array<Amg::Vector2D, 4>(Amg::Vector2D::Zero())};
        channelCorners(pad, corners);

        // For eta pad measurement, return height of given pad
        if (!measPhi) return corners.at(2)[1] - corners.at(0)[1];

        // Return the width at the top of the pads
        /* This is used by default and allows for track/segment association to pads to work correctly
           In these cases, the given position of the pad is always its centre so we can not know the
           precise position where we want to compute the width so its best to give a larger value
        */
        if (!preciseMeas) return corners.at(3)[0] - corners.at(2)[0];

        // Return precise Phi width for a given precise position on the pad
        /* This is only used when the precise position of a hit, track or segment is known on the pad
           Only to be used in specific cases, like in the digitization when we need to know the exact
           width along a certain point in the pad, e.g. for charge sharing. 
        */
        
        double WidthTop = corners.at(3)[0] - corners.at(2)[0];
        double WidthBot = corners.at(1)[0] - corners.at(0)[0];
        return WidthBot + (WidthTop - WidthBot) * (pos.y() - corners.at(0)[1]) / (corners.at(2)[1]-corners.at(0)[1]);
    }

    inline double MuonPadDesign::gasGapThickness() const { return thickness; }

}  // namespace MuonGM
#endif  // MUONREADOUTGEOMETRY_MUONPADDESIGN_H
