/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include <vector>

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MdtPrepDataContainer.h"


namespace Muon {
    class MdtPrepData;
}

/// New segment class for single ML segments
class TrackletSegment {
    private:
        const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};

        std::vector<const Muon::MdtPrepData*> m_mdts{};     // vector of hits on track
        Amg::Vector3D m_gpos{Amg::Vector3D::Zero()};        // global position of the segment
        double m_alpha{-999.}, m_dalpha{-999.};             // angle of the segment
        double m_rErr{-999.}, m_zErr{-999.};                // error on the r & z coordinates
        int m_pattern{0};                                   // hit pattern of the mdt hits
        bool m_isCombined{false};                           // TrackletSegment build from combined fit with another

    public:
        TrackletSegment() = default;
        TrackletSegment(const Muon::IMuonIdHelperSvc* idHelperSvc,
                        const std::vector<const Muon::MdtPrepData*>& mdts, 
                        const Amg::Vector3D& gpos, 
                        double alpha, double dalpha, 
                        double rErr, double zErr, 
                        int pattern);
                        
        ~TrackletSegment();

        // set functions
        void clearMdt();
        void isCombined(bool iscomb);

        // get functions
        const std::vector<const Muon::MdtPrepData*>& mdtHitsOnTrack() const;
        const Identifier getIdentifier() const;
        const Amg::Vector3D& globalPosition() const;
        double alpha() const;
        double alphaError() const;
        double zError() const;
        double rError() const;
        int getHitPattern() const;
        bool isCombined() const;

        // get properties of the MDT chamber
        int mdtChamber() const;
        int mdtChEta() const;
        int mdtChPhi() const;
        double getChMidPoint() const;
};
