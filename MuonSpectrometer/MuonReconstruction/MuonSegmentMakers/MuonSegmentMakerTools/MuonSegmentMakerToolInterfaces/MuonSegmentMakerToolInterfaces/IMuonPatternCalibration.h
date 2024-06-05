/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSEGMENTMAKETOOLINTERFACES_MUON_IMUONPATTERNCALIBRATION_H
#define MUONSEGMENTMAKETOOLINTERFACES_MUON_IMUONPATTERNCALIBRATION_H

#include <vector>

#include "GaudiKernel/IAlgTool.h"
#include "Identifier/Identifier.h"
#include "MuonPattern/MuonPatternCombination.h"
#include "MuonPattern/MuonPatternCombinationCollection.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRecToolInterfaces/IMdtDriftCircleOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonClusterOnTrackCreator.h"
//

namespace Muon {
    class MuonClusterOnTrack;
    class MdtDriftCircleOnTrack;

    /** @brief The IMuonCalibration is a pure virtual interface for tools to calibrate PRD hits  */
    class IMuonPatternCalibration : virtual public IAlgTool {
    public:
        using ClusterVec = std::vector<const MuonClusterOnTrack*>;
        using MdtVec = std::vector<const MdtDriftCircleOnTrack*>;
        using MdtVecVec = std::vector<MdtVec>;

        struct ROTRegion {
            int regionId{0};
            Amg::Vector3D regionPos{Amg::Vector3D::Zero()};
            Amg::Vector3D regionDir{Amg::Vector3D::Zero()};
            
            const ClusterVec& clusters() const {return m_clusters;}
            const MdtVecVec& mdts() const {return m_mdts;}

            void push_back(std::unique_ptr<const MuonClusterOnTrack> cl){
                // cppcheck-suppress danglingLifetime
                m_clusters.push_back(cl.get());
                m_garbage.push_back(std::move(cl));
            }
            void push_back(MdtVec&& vec) {
                std::transform(vec.begin(), vec.end(),std::back_inserter(m_garbage), 
                    [](const MdtDriftCircleOnTrack* mdt) {return std::unique_ptr<const Trk::MeasurementBase>{mdt};
                });
                m_mdts.push_back(std::move(vec));
            }
        private:
            ClusterVec m_clusters{};
            MdtVecVec m_mdts{};
            std::vector<std::unique_ptr<const Trk::MeasurementBase>> m_garbage{};
        };

        using ROTsPerRegion = std::vector<ROTRegion>;

        /** access to tool interface */
        static const InterfaceID& interfaceID() {
            static const InterfaceID IID_IMuonPatternCalibration("Muon::IMuonPatternCalibration", 1, 0);
            return IID_IMuonPatternCalibration;
        }

        virtual StatusCode calibrate(const EventContext& ctx, const MuonPatternCombination& pat, ROTsPerRegion& hitsPerRegion) const = 0;
        virtual int getRegionId(const Identifier& id) const = 0;      
        virtual bool checkForPhiMeasurements(const MuonPatternCombination& pat) const = 0;

        virtual ~IMuonPatternCalibration() = default;
    };

}  // namespace Muon

#endif
