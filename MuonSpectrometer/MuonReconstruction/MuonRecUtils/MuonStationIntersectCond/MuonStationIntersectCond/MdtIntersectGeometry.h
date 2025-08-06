/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSTATIONINTESECTCOND_MDTINTERSECTGEOMETRY_H
#define MUONSTATIONINTESECTCOND_MDTINTERSECTGEOMETRY_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonStationIntersectCond/MuonIntersectGeometry.h"
#include "TrkDriftCircleMath/MdtChamberGeometry.h"
class MsgStream;
namespace MuonGM {
    class MuonDetectorManager;
    class MdtReadoutElement;
}  // namespace MuonGM

class MdtCondDbData;

namespace Muon {

    class MdtIntersectGeometry : public MuonIntersectGeometry {
    public:
        MdtIntersectGeometry(MsgStream& msg, const Identifier& chid, const IMuonIdHelperSvc* idHelperSvc,
                             const MuonGM::MuonDetectorManager* detMgr, const MdtCondDbData* dbData);

        MdtIntersectGeometry(const MdtIntersectGeometry& right) = delete;
        MdtIntersectGeometry& operator=(const MdtIntersectGeometry& right) = delete;

        virtual ~MdtIntersectGeometry();

        MuonStationIntersect intersection(const MuonGM::MuonDetectorManager* detMgr,
                                          const Amg::Vector3D& pos, const Amg::Vector3D& dir) const override;

        const Amg::Transform3D& transform() const { return m_transform; }

        std::shared_ptr<const TrkDriftCircleMath::MdtChamberGeometry> mdtChamberGeometry() const;
        const Identifier& chamberId() const { return m_chid; }

    private:
        double tubeLength(const MuonGM::MdtReadoutElement* detElMl0,
                          const MuonGM::MdtReadoutElement* detElMl1,
                          const int ml, const int layer, const int tube) const;
        void init(const MuonGM::MuonDetectorManager* detMgr, MsgStream& msg);
        void fillDeadTubes(const MuonGM::MdtReadoutElement* mydetEl, MsgStream& msg);

        Identifier m_chid{};
        Amg::Transform3D m_transform;
        std::shared_ptr<TrkDriftCircleMath::MdtChamberGeometry> m_mdtGeometry{};
        IdentifierHash m_hashMl0;
        IdentifierHash m_hashMl1;
        const MdtCondDbData* m_dbData{nullptr};
        const IMuonIdHelperSvc* m_idHelperSvc{nullptr};
        std::set<Identifier> m_deadTubesML{};
        std::vector<Identifier> m_deadTubes{};
    };

}  // namespace Muon

#endif
