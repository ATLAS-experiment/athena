/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonStationIntersectCond/MdtIntersectGeometry.h"

#include <iostream>

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GeoModelUtilities/GeoGetIds.h"
#include "MuonCondData/MdtCondDbData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "TrkDriftCircleMath/MdtChamberGeometry.h"
#include "TrkDriftCircleMath/MdtId.h"
#include "GeoModelKernel/throwExcept.h"
// maxNTubesPerLayer is included via MdtChamberGeometry.h -> DriftCircle.h

namespace Muon {

    MdtIntersectGeometry::MdtIntersectGeometry(MsgStream& msg, const Identifier& chid, const IMuonIdHelperSvc* idHelperSvc,
                                               const MuonGM::MuonDetectorManager* detMgr, const MdtCondDbData* dbData) :
        m_chid(chid), m_dbData(dbData), m_idHelperSvc(idHelperSvc) {
        init(detMgr, msg);
    }

    MdtIntersectGeometry::~MdtIntersectGeometry() = default;

    MuonStationIntersect MdtIntersectGeometry::intersection(const MuonGM::MuonDetectorManager* detMgr,
                                                            const Amg::Vector3D& pos, const Amg::Vector3D& dir ) const {
        MuonStationIntersect intersect;
        if (!m_mdtGeometry) {
            MsgStream log(Athena::getMessageSvc(), "MdtIntersectGeometry");
            log << MSG::WARNING << "MdtIntersectGeometry::intersection() - MdtIntersectGeometry not correctly initialized "
                << m_idHelperSvc->mdtIdHelper().print_to_string(m_chid) << endmsg;
            return intersect;
        }

        Amg::Vector3D lpos = transform() * pos;
        Amg::Vector3D ldir = (transform().linear() * dir).unit();

        double dxdy = std::abs(ldir.y()) > 0.001 ? ldir.x() / ldir.y() : 1000.;

        double lineAngle = std::atan2(ldir.z(), ldir.y());
        TrkDriftCircleMath::LocVec2D linePos(lpos.y(), lpos.z());
        TrkDriftCircleMath::Line line(linePos, lineAngle);
        const TrkDriftCircleMath::DCVec dcs = m_mdtGeometry->tubesPassedByLine(line);

        MuonStationIntersect::TubeIntersects intersects;

        const MuonGM::MdtReadoutElement* detElMl0 = m_hashMl0.is_valid() ? detMgr->getMdtReadoutElement (m_hashMl0) : nullptr;
        const MuonGM::MdtReadoutElement* detElMl1 = m_hashMl1.is_valid() ? detMgr->getMdtReadoutElement (m_hashMl1) : nullptr;

        TrkDriftCircleMath::DCCit dit = dcs.begin();
        TrkDriftCircleMath::DCCit dit_end = dcs.end();
        for (; dit != dit_end; ++dit) {
            const TrkDriftCircleMath::MdtId& mdtId = dit->id();

            double xint = dxdy * (dit->position().x() - lpos.y()) + lpos.x();
            Identifier tubeid = m_idHelperSvc->mdtIdHelper().channelID(m_chid, mdtId.ml() + 1, mdtId.lay() + 1, mdtId.tube() + 1);
            if (m_deadTubesML.find(m_idHelperSvc->mdtIdHelper().multilayerID(tubeid)) != m_deadTubesML.end()) {
                if (std::find(m_deadTubes.begin(), m_deadTubes.end(), tubeid) != m_deadTubes.end()) continue;
            }
            double distWall = std::abs(xint) - 0.5 * tubeLength(detElMl0, detElMl1,
                                                                mdtId.ml(), mdtId.lay(), mdtId.tube());
            intersects.emplace_back(tubeid, dit->dr(), distWall);
        }
        intersect.setTubeIntersects(std::move(intersects));

        return intersect;
    }

    double MdtIntersectGeometry::tubeLength(const MuonGM::MdtReadoutElement* detElMl0,
                                            const MuonGM::MdtReadoutElement* detElMl1,
                                            const int ml, const int layer, const int tube) const {
#ifndef NDEBUG
        if (ml < 0 || ml > 1){
            THROW_EXCEPTION(__func__<<"() got called with ml="<<ml<<" which is definetly out of range");
        }
        if (layer < 0 || layer > 3) {
            THROW_EXCEPTION(__func__<<"() got called with layer="<<layer<<" which is definetly out of range");
        }  
        if (tube < 0 || tube >= int(MdtIdHelper::maxNTubesPerLayer)){
            THROW_EXCEPTION(__func__<<"() got called with tube="<<tube<<" which is definetly out of range");
        }
#endif
        // shift by one to account for MuonGeoModel scheme
        int theTube = tube + 1;
        int theLayer = layer + 1;
        // handle case where first ml is dead
        if (ml == 1 && !detElMl1) return detElMl0->getActiveTubeLength(theLayer, theTube);
        if (ml == 0)
            return detElMl0->getActiveTubeLength(theLayer, theTube);
        else
            return detElMl1->getActiveTubeLength(theLayer, theTube);
    }

    void MdtIntersectGeometry::init(const MuonGM::MuonDetectorManager* detMgr, MsgStream& msg) {
        /* calculate chamber geometry
           it takes as input:
             distance between the first and second tube in the chamber within a layer along the tube layer (tube distance)
             distance between the first tube in the first layer and the first tube in the second layer along the tube layer (tube stagering)
             distance between the first and second layer perpendicular to the tube layers (layer distance)
             position of the first hit in ml 0 and ml 1 (2D in plane)
             total number of multilayers
             total number of layers
             total number of tubes per layer for each multilayer
             an identifier uniquely identifying the chamber
        */

        // get id
        int eta = m_idHelperSvc->mdtIdHelper().stationEta(m_chid);
        int phi = m_idHelperSvc->mdtIdHelper().stationPhi(m_chid);
        int name = m_idHelperSvc->mdtIdHelper().stationName(m_chid);
        // get detEL for first ml (always there)
        Identifier firstIdml0 = m_idHelperSvc->mdtIdHelper().channelID(name, eta, phi, 1, 1, 1);
        Identifier firstIdml1;

        const MuonGM::MdtReadoutElement* detElMl0 = detMgr->getMdtReadoutElement(firstIdml0);
        const MuonGM::MdtReadoutElement* detElMl1 = nullptr;

        if (!detElMl0) {
            msg << MSG::WARNING << "MdtIntersectGeometry::init() - failed to get readout element for ML0" << endmsg;
            return;
        }

        // number of multilayers in chamber
        int nml = detElMl0->nMDTinStation();

        // treament of chambers with two ml
        if (nml == 2) {
            firstIdml1 = m_idHelperSvc->mdtIdHelper().channelID(name, eta, phi, 2, 1, 1);
            detElMl1 = detMgr->getMdtReadoutElement(firstIdml1);
        }

        // if one of the two ml is dead treat the chamber as a single ML station
        // if both are dead give a WARNING
        // check status of the two multilayers using the MdtCondDbData if it exists
        // otherwise (i.e. online) they are treated as both good by default
        bool goodMl0{false}, goodMl1{false};
        if (m_dbData) {
            goodMl0 = m_dbData->isGoodMultilayer(firstIdml0);
            goodMl1 = detElMl1 ? m_dbData->isGoodMultilayer(firstIdml1) : false;
        } else {
            goodMl0 = true;
            goodMl1 = true;
        }
        int firstMlIndex = 1;
        if (goodMl0 && !goodMl1) {
            nml = 1;
            detElMl1 = nullptr;
        } else if (!goodMl0 && goodMl1) {
            nml = 1;
            // swap detEl1 and detEl0
            detElMl0 = detElMl1;
            detElMl1 = nullptr;
            firstIdml0 = firstIdml1;
            firstMlIndex = 2;
        } else if (!goodMl0 && !goodMl1) {
            msg << MSG::WARNING << "MdtIntersectGeometry::init() - neither multilayer is good" << endmsg;
            return;
        }
        m_transform = detElMl0->GlobalToAmdbLRSTransform();

        // number of layers and tubes
        int nlay = detElMl0->getNLayers();
        int ntube0 = detElMl0->getNtubesperlayer();
        int ntube1 = detElMl1 ? detElMl1->getNtubesperlayer() : 0;

        // position first tube in ml 0 and 1
        Amg::Vector3D firstTubeMl0 = transform() * (detElMl0->tubePos(firstIdml0));
        Amg::Vector3D firstTubeMl1 = detElMl1 ? transform() * (detElMl1->tubePos(firstIdml1)) : Amg::Vector3D{0., 0., 0.};

        TrkDriftCircleMath::LocVec2D firstTube0(firstTubeMl0.y(), firstTubeMl0.z());
        TrkDriftCircleMath::LocVec2D firstTube1(firstTubeMl1.y(), firstTubeMl1.z());

        // position second tube in ml 0
        Identifier secondIdml0 = m_idHelperSvc->mdtIdHelper().channelID(name, eta, phi, firstMlIndex, 1, 2);
        Amg::Vector3D secondTubeMl0 = transform() * (detElMl0->tubePos(secondIdml0));

                      fillDeadTubes(detElMl0, msg);
        if (detElMl1) fillDeadTubes(detElMl1, msg);

        // position first tube in second layer ml 0
        Identifier firstIdml0lay1 = m_idHelperSvc->mdtIdHelper().channelID(name, eta, phi, firstMlIndex, 2, 1);
        Amg::Vector3D firstTubeMl0lay1 = transform() * (detElMl0->tubePos(firstIdml0lay1));

        double tubeDist = (secondTubeMl0 - firstTubeMl0).y();      // distance between tube in a given layer
        double tubeStage = (firstTubeMl0lay1 - firstTubeMl0).y();  // tube stagering distance
        double layDist = (firstTubeMl0lay1 - firstTubeMl0).z();    // distance between layers

        m_mdtGeometry = std::make_unique<TrkDriftCircleMath::MdtChamberGeometry>(
            m_chid, m_idHelperSvc, nml, nlay, ntube0, ntube1, firstTube0, firstTube1, tubeDist, tubeStage, layDist, detElMl0->center().theta());

        // finally if the first ml is dead, configure the MdtChamberGeometry accordingly
        if (!goodMl0 && goodMl1) m_mdtGeometry->isSecondMultiLayer(true);

                      m_hashMl0 = detElMl0->detectorElementHash();
        if (detElMl1) m_hashMl1 = detElMl1->detectorElementHash();
    }
    std::shared_ptr<const TrkDriftCircleMath::MdtChamberGeometry> MdtIntersectGeometry::mdtChamberGeometry() const { return m_mdtGeometry; }
    void MdtIntersectGeometry::fillDeadTubes(const MuonGM::MdtReadoutElement* mydetEl, MsgStream& msg) {
        if ((mydetEl->getStationName()).find("BMG") != std::string::npos) {
            PVConstLink cv = mydetEl->getMaterialGeom();  // it is "Multilayer"
            int nGrandchildren = cv->getNChildVols();
            if (nGrandchildren <= 0) return;

            std::vector<int> tubes;
            geoGetIds([&](int id) { tubes.push_back(id); }, &*cv);
            std::sort(tubes.begin(), tubes.end());

            Identifier detElId = mydetEl->identify();

            int name = m_idHelperSvc->mdtIdHelper().stationName(detElId);
            int eta = m_idHelperSvc->mdtIdHelper().stationEta(detElId);
            int phi = m_idHelperSvc->mdtIdHelper().stationPhi(detElId);
            int ml = m_idHelperSvc->mdtIdHelper().multilayer(detElId);

            std::vector<int>::iterator it = tubes.begin();
            for (int layer = 1; layer <= mydetEl->getNLayers(); layer++) {
                for (int tube = 1; tube <= mydetEl->getNtubesperlayer(); tube++) {
                    int want_id = layer * MdtIdHelper::maxNTubesPerLayer + tube;
                    if (it != tubes.end() && *it == want_id) {
                        ++it;
                    } else {
                        it = std::lower_bound(tubes.begin(), tubes.end(), want_id);
                        if (it != tubes.end() && *it == want_id) {
                            ++it;
                        } else {
                            Identifier deadTubeId = m_idHelperSvc->mdtIdHelper().channelID(name, eta, phi, ml, layer, tube);
                            Identifier deadTubeMLId = m_idHelperSvc->mdtIdHelper().multilayerID(deadTubeId);
                            m_deadTubes.push_back(deadTubeId);
                            m_deadTubesML.insert(deadTubeMLId);
                            if (msg.level() == MSG::VERBOSE)
                                msg << MSG::VERBOSE << " MdtIntersectGeometry: adding dead tube (" << tube << "), layer(" << layer
                                    << "), phi(" << phi << "), eta(" << eta << "), name(" << name << ") and adding multilayerId("
                                    << deadTubeMLId << ")." << endmsg;
                        }
                    }
                }
            }
        }
    }

}  // namespace Muon
