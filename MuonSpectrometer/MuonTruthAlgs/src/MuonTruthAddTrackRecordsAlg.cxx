/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTruthAddTrackRecordsAlg.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "EventPrimitives/EventPrimitivesCovarianceHelpers.h"
#include "xAODTruth/TruthVertex.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkParameters/TrackParameters.h"

namespace {

    constexpr float dummy_val = -1.;

    struct RecordPars {
        RecordPars() = default;
        RecordPars(Amg::Vector3D&& _pos, Amg::Vector3D&&_mom):
            pos{std::move(_pos)},
            mom{std::move(_mom)}{}
        RecordPars(const CLHEP::Hep3Vector& _pos, const CLHEP::Hep3Vector& _mom):
            pos{_pos.x(), _pos.y(), _pos.z()},
            mom{_mom.x(), _mom.y(), _mom.z()}{
        }
        const Amg::Vector3D pos{Amg::Vector3D::Zero()};
        const Amg::Vector3D mom{Amg::Vector3D::Zero()};

        std::string record_name{};
        size_t idx{0};
        const Trk::TrackingVolume* volume{nullptr};
    };

}  // namespace

namespace Muon {

    template <typename T>
    StatusCode MuonTruthAddTrackRecordsAlg::fillWriteDecorator(SG::WriteDecorHandleKeyArray<xAOD::TruthParticleContainer, T>& writeKey,
                                                             const std::string& keyName) const{
        for (const SG::ReadHandleKey<TrackRecordCollection>& recordKey : m_trackRecords){
            const std::string fullKey = recordKey.key() + "_" + keyName; 
            writeKey.emplace_back(m_muonTruth, fullKey);
        }
        return writeKey.initialize();
    }

    // Initialize method:
    StatusCode MuonTruthAddTrackRecordsAlg::initialize() {
        ATH_CHECK(m_muonTruth.initialize());
        ATH_CHECK(m_trackRecords.initialize());

        ATH_CHECK(fillWriteDecorator(m_muonSnapShotX, "x"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotY, "y"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotZ, "z"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotPx, "px"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotPy, "py"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotPz, "pz"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotMtc, "is_matched"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotEX, "x_extr"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotEY, "y_extr"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotEZ, "z_extr"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotEPx, "px_extr"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotEPy, "py_extr"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotEPz, "pz_extr"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotEcov, "cov_extr"));
        ATH_CHECK(fillWriteDecorator(m_muonSnapShotEis, "is_extr"));

        ATH_CHECK(m_extrapolator.retrieve());
        return StatusCode::SUCCESS;
    }

    // Execute method:
    StatusCode MuonTruthAddTrackRecordsAlg::execute(const EventContext& ctx) const {
        // skip if no input data found
        SG::ReadHandle muonTruthContainer(m_muonTruth, ctx);
        ATH_CHECK(muonTruthContainer.isPresent());

        summaryDecors myDecors;
        myDecors.xDecor = m_muonSnapShotX.makeHandles(ctx);
        myDecors.yDecor = m_muonSnapShotY.makeHandles(ctx);
        myDecors.zDecor = m_muonSnapShotZ.makeHandles(ctx);
        myDecors.pxDecor = m_muonSnapShotPx.makeHandles(ctx);
        myDecors.pyDecor = m_muonSnapShotPy.makeHandles(ctx);
        myDecors.pzDecor = m_muonSnapShotPz.makeHandles(ctx);
        myDecors.matchedDecor = m_muonSnapShotMtc.makeHandles(ctx);
        myDecors.exDecor = m_muonSnapShotEX.makeHandles(ctx);
        myDecors.eyDecor = m_muonSnapShotEY.makeHandles(ctx);
        myDecors.ezDecor = m_muonSnapShotEZ.makeHandles(ctx);
        myDecors.epxDecor = m_muonSnapShotEPx.makeHandles(ctx);
        myDecors.epyDecor = m_muonSnapShotEPy.makeHandles(ctx);
        myDecors.epzDecor = m_muonSnapShotEPz.makeHandles(ctx);
        myDecors.ecovDecor = m_muonSnapShotEcov.makeHandles(ctx);
        myDecors.eisDecor = m_muonSnapShotEis.makeHandles(ctx);

        // loop over truth coll
        for (const xAOD::TruthParticle* truthParticle : *muonTruthContainer) {
            ATH_CHECK(addTrackRecords(ctx, *truthParticle, myDecors));
        }
        return StatusCode::SUCCESS;
    }
    
    StatusCode MuonTruthAddTrackRecordsAlg::addTrackRecords(const EventContext& ctx, const xAOD::TruthParticle& truthParticle, summaryDecors& myDecors) const {          

        // first loop over track records, store parameters at the different positions
        const xAOD::TruthVertex* vertex = truthParticle.prodVtx();
        const Trk::TrackingGeometry* trackingGeometry = m_extrapolator->trackingGeometry();

        std::vector<RecordPars> parameters;
        if (vertex) {
            parameters.emplace_back(Amg::Vector3D{vertex->x(), vertex->y(), vertex->z()},
                                    Amg::Vector3D{truthParticle.px(), truthParticle.py(), truthParticle.pz()});
        }
        else {parameters.emplace_back();}

        size_t idx{0};
        for (SG::ReadHandle<TrackRecordCollection>& trackRecordCollection : m_trackRecords.makeHandles(ctx)) {
            ATH_CHECK(trackRecordCollection.isPresent());
    
            const std::string r_name = trackRecordCollection.key();
            float& x = myDecors.xDecor[idx](truthParticle);
            float& y = myDecors.yDecor[idx](truthParticle);
            float& z = myDecors.zDecor[idx](truthParticle);
            float& px = myDecors.pxDecor[idx](truthParticle);
            float& py = myDecors.pyDecor[idx](truthParticle);
            float& pz = myDecors.pzDecor[idx](truthParticle);

            char& found_truth = myDecors.matchedDecor[idx](truthParticle);

            x = y = z = px = py = pz = dummy_val;
            found_truth = false;

            // Need to always make these, to avoid crashes later
            float& ex = myDecors.exDecor[idx](truthParticle);
            float& ey = myDecors.eyDecor[idx](truthParticle);
            float& ez = myDecors.ezDecor[idx](truthParticle);
            float& epx = myDecors.epxDecor[idx](truthParticle);
            float& epy = myDecors.epyDecor[idx](truthParticle);
            float& epz = myDecors.epzDecor[idx](truthParticle);

            myDecors.ecovDecor[idx](truthParticle) = std::vector<float>{};

            myDecors.eisDecor[idx](truthParticle) = false;
            ex = ey = ez = epx = epy = epz = dummy_val;

            /// loop over collection and find trackRecord with the same bar code. If we find a record, we save position and momentum
            for (const TrackRecord& trackRecord : *trackRecordCollection) {
                if (!HepMC::is_sim_descendant(&trackRecord,&truthParticle)) continue;
                CLHEP::Hep3Vector pos = trackRecord.GetPosition();
                CLHEP::Hep3Vector mom = trackRecord.GetMomentum();
                ATH_MSG_VERBOSE("Found associated  " << r_name << " pt " << mom.perp() << " position: r " << pos.perp() << " z " << pos.z());
                x = pos.x(); y = pos.y(); z = pos.z();
                px = mom.x(); py = mom.y(); pz = mom.z();
                parameters.emplace_back(pos, mom);
                found_truth = true;
                break;
            }

            /// Make sure that the parameter vector has the same size
            if (!found_truth) {parameters.emplace_back();}
            /// Save record name
            RecordPars& r_pars = parameters.back();
            r_pars.record_name = r_name;
            r_pars.idx = idx++;

            /// Save tracking volume
            if (!found_truth) continue;
            const Trk::TrackingVolume* volume = nullptr;
            if (r_name == "CaloEntryLayer")
                volume = trackingGeometry->trackingVolume("InDet::Containers::InnerDetector");
            else if (r_name == "MuonEntryLayer")
                volume = trackingGeometry->trackingVolume("Calo::Container");
            else if (r_name == "MuonExitLayer")
                volume = trackingGeometry->trackingVolume("Muon::Containers::MuonSystem");
            else {
                ATH_MSG_WARNING("no destination surface for track record "<<r_name);
            }
            r_pars.volume = volume;
        }

        /// Second loop, extrapolate between the points
        AmgSymMatrix(5) cov{AmgSymMatrix(5)::Identity()};
        cov(0, 0) = cov(1, 1) = 1e-3;
        cov(2, 2) = cov(3, 3) = 1e-6;
        cov(4, 4) = 1e-3 / truthParticle.p4().P();
        for (unsigned int i = 0; i < parameters.size() - 1; ++i) {
            const RecordPars& start_pars = parameters[i];
            const RecordPars& end_pars = parameters[i+1];
            /// If the track record is named, then we should have a volume
            if ( (!start_pars.record_name.empty() && !start_pars.volume) || !end_pars.volume) continue;
            Trk::CurvilinearParameters pars(start_pars.pos, start_pars.mom, truthParticle.charge(), cov);

            const Trk::TrackingVolume* volume = end_pars.volume;
            const std::string& r_name = end_pars.record_name;    

            float& ex = myDecors.exDecor[end_pars.idx](truthParticle);
            float& ey = myDecors.eyDecor[end_pars.idx](truthParticle);
            float& ez = myDecors.ezDecor[end_pars.idx](truthParticle);
            float& epx = myDecors.epxDecor[end_pars.idx](truthParticle);  
            float& epy = myDecors.epyDecor[end_pars.idx](truthParticle);
            float& epz = myDecors.epzDecor[end_pars.idx](truthParticle);
            
            std::vector<float>& covMat = myDecors.ecovDecor[end_pars.idx](truthParticle);

            std::unique_ptr<Trk::TrackParameters> exPars{
                m_extrapolator->extrapolateToVolume(ctx, pars, *volume, Trk::alongMomentum, Trk::muon)};
            if (! exPars || !exPars->covariance() || !Amg::hasPositiveDiagElems(*exPars->covariance())) {
                ATH_MSG_VERBOSE("Extrapolation to "<<r_name<<" failed. ");
                continue;
            }
            myDecors.eisDecor[end_pars.idx](truthParticle) = true;
            ex = exPars->position().x();
            ey = exPars->position().y();
            ez = exPars->position().z();
            epx = exPars->momentum().x();
            epy = exPars->momentum().y();
            epz = exPars->momentum().z();
            Amg::compress(*exPars->covariance(), covMat);

            const double errorp = Amg::error(*exPars->covariance(), Trk::qOverP);
            ATH_MSG_VERBOSE( " Extrapolated to " << r_name << std::endl
                << " truth: r " << end_pars.pos.perp() << " z "
                << end_pars.pos.z() << " p "
                << end_pars.mom.mag() << std::endl
                << " extrp: r " << exPars->position().perp() << " z "
                << exPars->position().z() << " p "
                << exPars->momentum().mag() << " res p "
                << (end_pars.mom.mag() -
                    exPars->momentum().mag())
                << " error " << errorp << " cov "
                << (*exPars->covariance())(Trk::qOverP, Trk::qOverP)
                << " pull p "
                << (end_pars.mom.mag() -
                    exPars->momentum().mag()) /
                     errorp);
        }
        return StatusCode::SUCCESS;
    }
    
}  // namespace Muon
