/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_MUONCHAMBER_H
#define MUONREADOUTGEOMETRYR4_MUONCHAMBER_H

#ifndef SIMULATIONBASE
/**
 *  A muon chamber is a collection of readout elements belonging to the same station.
 *  In the barrel, it's typically 2 Mdt multi layers + a couple of RPC layers, while the
 *  endcape chambers either consist of Mdt multi layers, multiple TGC layers or in the 
 *  case of the NSW, 1 sTGC layer or 2 Micromega multilayers.
 * 
 *  The MuonChamber class provides the sets of radout elements and the definitions of the
 *  minimal surrounding box. 
*/

#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <Acts/Geometry/TrapezoidVolumeBounds.hpp>
#include <Acts/Geometry/Volume.hpp>

#include <set>

namespace MuonGMR4{
    class MuonChamber;
}
namespace ActsTrk {
    template<> TransformCacheDetEle<MuonGMR4::MuonChamber>::TransformCacheDetEle(const IdentifierHash& hash, 
                                                                                 const MuonGMR4::MuonChamber* chamber);
    
    template <> Amg::Transform3D 
        TransformCacheDetEle<MuonGMR4::MuonChamber>::fetchTransform(const DetectorAlignStore* store) const;
    template <> const IDetectorElement*
        TransformCacheDetEle<MuonGMR4::MuonChamber>::parent() const;

}

namespace MuonGMR4 {
    class MuonChamber {
        public:
            using ReadoutSet = std::vector<const MuonReadoutElement*>;                
            struct defineArgs{
                /// List of readout elements in the chamber
                ReadoutSet readoutEles{};
                /// Definition of the surrounding box
                double halfXLong{0.};
                double halfXShort{0.};
                double halfY{0.};
                double halfZ{0.};
                /// Transformation to the chamber volume
                Amg::Transform3D centerTrans{Amg::Transform3D::Identity()};
            };

            MuonChamber(defineArgs&& args);
            MuonChamber(const MuonChamber& other);
            MuonChamber(MuonChamber&& other);
            MuonChamber& operator=(const MuonChamber& other);
            MuonChamber& operator=(MuonChamber&& other);

            /// Returns the idHelperSvc
            const Muon::IMuonIdHelperSvc* idHelperSvc() const;
            /// Returns the chamber index
            Muon::MuonStationIndex::ChIndex chamberIndex() const;
            // Returns the station name of the chamber
            int stationName() const;            
            /// Returns the station eta of the chamber
            int stationEta() const;
            /// Returns the station phi of the chamber
            int stationPhi() const;
            /// Returns the detector type of the primary detector element
            ActsTrk::DetectorType detectorType() const;
            /// Returns the sector of the detector element
            int sector() const;
            /// Returns the list of all associated readout elements 
            const ReadoutSet& readOutElements() const;
            /// Returns the transformation of the MuonChamber
            const Amg::Transform3D& localToGlobalTrans(const ActsGeometryContext& gctx) const;
            Amg::Transform3D globalToLocalTrans(const ActsGeometryContext& gctx) const;
            /// Surrounding box dimensions
            double halfXLong() const;
            double halfXShort() const;
            double halfY() const;
            double halfZ() const;

            std::shared_ptr<Acts::Volume> boundingVolume(const ActsGeometryContext& gctx) const;
            std::shared_ptr<Acts::TrapezoidVolumeBounds> bounds() const;

            const defineArgs& parameters() const;
        private:
           defineArgs m_args{};
           using AlignmentStore = ActsGeometryContext::AlignmentStore;
           ActsTrk::TransformCacheDetEle<MuonChamber> m_localToGlobal{IdentifierHash{0}, this};
    };
    
    std::ostream& operator<<(std::ostream& ostr,
                             const MuonChamber::defineArgs& args);

    std::ostream& operator<<(std::ostream& ostr,
                             const MuonChamber& chamber);

}


#endif
#endif