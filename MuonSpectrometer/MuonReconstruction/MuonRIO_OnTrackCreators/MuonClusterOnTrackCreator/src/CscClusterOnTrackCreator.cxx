/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>

#include "CscClusterOnTrackCreator.h"

#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "MuonPrepRawData/CscClusterStatus.h"
#include "MuonPrepRawData/CscPrepData.h"
#include "MuonPrepRawData/CscStripPrepDataCollection.h"
#include "MuonRIO_OnTrack/CscClusterOnTrack.h"
#include "TrkEventPrimitives/LocalDirection.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkRIO_OnTrack/ErrorScalingCast.h"
#include "TrkSurfaces/Surface.h"

using Amg::Transform3D;
using Amg::Vector3D;
using Muon::CscStripPrepData;
using Muon::CscStripPrepDataCollection;
using Muon::CscStripPrepDataContainer;

namespace Muon {

    CscClusterOnTrackCreator::CscClusterOnTrackCreator(const std::string& ty, const std::string& na, const IInterface* pa) :
        AthAlgTool(ty, na, pa) {
        // algtool interface - necessary!
        declareInterface<IMuonClusterOnTrackCreator>(this);
        declareInterface<IRIO_OnTrackCreator>(this);
    }

    CscClusterOnTrackCreator::~CscClusterOnTrackCreator() = default;

    /////////////////////////////////
    StatusCode CscClusterOnTrackCreator::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        if (!m_idHelperSvc->hasCSC()) {
            ATH_MSG_ERROR("The given detector layout does not contain any CSC chamber, "<<
                          "there must be something wrong in the configuration,"
                          <<" since the CscClusterOnTrackCreator cannot be needed.");
            return StatusCode::FAILURE;
        }

        // get error scaling tool
        //
        ATH_CHECK(m_cscErrorScalingKey.initialize(!m_cscErrorScalingKey.key().empty())); 
        ATH_CHECK(m_stripFitter.retrieve());
        ATH_CHECK(m_clusterFitter.retrieve());
        ATH_CHECK(m_clusterUtilTool.retrieve());
        return StatusCode::SUCCESS;
    }

    ////////////////////////////////////////////////////////
    MuonClusterOnTrack* CscClusterOnTrackCreator::createRIO_OnTrack(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP) const {
        MuonClusterOnTrack* MClT = nullptr;

        // check whether PrepRawData has detector element, if not there print warning
        const Trk::TrkDetElementBase* EL = RIO.detectorElement();
        if (!EL) {
            ATH_MSG_WARNING("RIO does not have associated detectorElement!, cannot produce ROT");
            return nullptr;
        }

        // MuClusterOnTrack production
        //
        // in RIO_OnTrack the local param and cov should have the same dimension
        Trk::LocalParameters locpar(RIO.localPosition());
        if (RIO.localCovariance().cols() > 1) {
            ATH_MSG_VERBOSE("Making 2-dim local parameters");
        } else {
            Trk::DefinedParameter radiusPar(RIO.localPosition().x(), Trk::locX);
            locpar = Trk::LocalParameters(radiusPar);
            ATH_MSG_VERBOSE("Making 1-dim local parameters");
        }

        Amg::Vector2D lp;
        double positionAlongStrip = 0;

        if (!EL->surface(RIO.identify()).globalToLocal(GP, GP, lp)) {
            Amg::Vector3D lpos = RIO.detectorElement()->surface(RIO.identify()).transform().inverse() * GP;
            ATH_MSG_WARNING("Extrapolated GlobalPosition not on detector surface! Distance " << lpos.z());
            lp[Trk::locX] = lpos.x();
            lp[Trk::locY] = lpos.y();
        }
        positionAlongStrip = lp[Trk::locY];

        // Error matrix production - expect more intelligent code here.
        //
        Amg::MatrixX loce = RIO.localCovariance();

        if (m_idHelperSvc->isCsc(RIO.identify()) && !m_cscErrorScalingKey.key().empty()) {
            SG::ReadCondHandle<RIO_OnTrackErrorScaling> error_scaling(m_cscErrorScalingKey);
            loce = Trk::ErrorScalingCast<MuonEtaPhiRIO_OnTrackErrorScaling>(
                       *error_scaling)
                       ->getScaledCovariance(std::move(loce), Trk::distPhi);
            ATH_MSG_VERBOSE("CSC: new cov(0,0) is " << loce(0, 0));
        }

        if (m_idHelperSvc->isCsc(RIO.identify())) {
            // cast to CscPrepData
            const CscPrepData* MClus = dynamic_cast<const CscPrepData*>(&RIO);
            if (!MClus) {
                ATH_MSG_WARNING("RIO not of type CscPrepData, cannot create ROT");
                return nullptr;
            }

            // current not changing CscClusterStatus but passing status of RIO
            MClT = new CscClusterOnTrack(MClus, std::move(locpar), std::move(loce),
                                         positionAlongStrip, MClus->status(), MClus->timeStatus(), MClus->time());
        }

        return MClT;
    }

    /////////////////////////////////////////////////////////////////

    MuonClusterOnTrack* CscClusterOnTrackCreator::createRIO_OnTrack(const Trk::PrepRawData& RIO, 
                                                                    const Amg::Vector3D& GP,
                                                                    const Amg::Vector3D& GD) const {
        if (!m_idHelperSvc->isCsc(RIO.identify())) {
            ATH_MSG_WARNING("CscClusterOnTrackCreator::createRIO_OnTrack is called by the other muon tech");
            return nullptr;
        }

        MuonClusterOnTrack* MClT = nullptr;
        // check whether PrepRawData has detector element, if not there print warning
        const Trk::TrkDetElementBase* EL = RIO.detectorElement();
        if (!EL) {
            ATH_MSG_WARNING("RIO does not have associated detectorElement!, cannot produce ROT");
            return MClT;
        }
        // cast to CscPrepData : Moved to the front to avoid any memory allocation before return;
        const CscPrepData* MClus = dynamic_cast<const CscPrepData*>(&RIO);
        if (!MClus) {
            ATH_MSG_WARNING("RIO not of type CscPrepData, cannot create ROT");
            return MClT;
        }

        // MuClusterOnTrack production
        //
        // in RIO_OnTrack the local param and cov should have the same dimension
        Trk::LocalParameters locpar = Trk::LocalParameters(RIO.localPosition());
        if (RIO.localCovariance().cols() > 1 ||
            (m_idHelperSvc->isTgc(RIO.identify()) && m_idHelperSvc->tgcIdHelper().isStrip(RIO.identify()))) {
            ATH_MSG_VERBOSE("Making 2-dim local parameters");
        } else {
            Trk::DefinedParameter radiusPar(RIO.localPosition().x(), Trk::locX);
            locpar = Trk::LocalParameters(radiusPar);
            ATH_MSG_VERBOSE("Making 1-dim local parameters");
        }

        Amg::Vector2D lp{Amg::Vector2D::Zero()};
        double positionAlongStrip = 0;

        if (!EL->surface(RIO.identify()).globalToLocal(GP, GP, lp)) {
            Amg::Vector3D lpos = RIO.detectorElement()->surface(RIO.identify()).transform().inverse() * GP;
            ATH_MSG_WARNING("Extrapolated GlobalPosition not on detector surface! Distance " << lpos.z());
            lp[Trk::locX] = lpos.x();
            lp[Trk::locY] = lpos.y();
        }
        positionAlongStrip = lp[Trk::locY];

        // Error matrix production - expect more intelligent code here.
        //
        Amg::MatrixX loce = RIO.localCovariance();

        if (m_idHelperSvc->isCsc(RIO.identify()) && !m_cscErrorScalingKey.key().empty()) {
            SG::ReadCondHandle<RIO_OnTrackErrorScaling> error_scaling(m_cscErrorScalingKey);
            loce = Trk::ErrorScalingCast<MuonEtaPhiRIO_OnTrackErrorScaling>(
                       *error_scaling)
                       ->getScaledCovariance(std::move(loce), Trk::distPhi);
            ATH_MSG_VERBOSE("CSC: new cov(0,0) is " << loce(0, 0));
        }

        // postion Error is re-estimate only for precision fit cluster (eta)
        if (MClus->status() != Muon::CscStatusUnspoiled && MClus->status() != Muon::CscStatusSplitUnspoiled) {
            // current not changing CscClusterStatus but passing status of RIO
            MClT = new CscClusterOnTrack(MClus, std::move(locpar), std::move(loce), positionAlongStrip, MClus->status(), MClus->timeStatus(), MClus->time());

        } else {
            const MuonGM::CscReadoutElement* ele = MClus->detectorElement();
            Transform3D globalToLocal = ele->transform(MClus->identify()).inverse();
            Vector3D d(globalToLocal * GD);
            double tantheta = d.x() / d.z();

            std::vector<ICscClusterFitter::Result> results, results0;
            results = m_clusterUtilTool->getRefitCluster(MClus, tantheta);
            results0 = m_clusterUtilTool->getRefitCluster(MClus, 0);

            if (results.empty() || results0.empty()) {
                ATH_MSG_VERBOSE("No fit result");
                return new CscClusterOnTrack(MClus, std::move(locpar), std::move(loce),
                                             positionAlongStrip, MClus->status(), MClus->timeStatus(), MClus->time());
            }
            ICscClusterFitter::Result res, res0;
            res = results[0];
            res0 = results0[0];  // result at normal angle to make error blown correctly in case of cosmic
            int fitresult = res.fitStatus;
            if (fitresult) {
                ATH_MSG_VERBOSE("  Precision fit failed which was succeeded: return="
                                << "cluStatus: " << res.clusterStatus << "fitStatus: " << res.fitStatus);
                return new CscClusterOnTrack(MClus, std::move(locpar), std::move(loce),
                                             positionAlongStrip, MClus->status(), MClus->timeStatus(), MClus->time());
            } else {
                ATH_MSG_VERBOSE("  Precision fit succeeded");
            }

            ATH_MSG_VERBOSE("  Angle from Segment:  "
                            << " :: tanangle :  " << tantheta);

            // in case that we need to scale errors for cosmic/data we want to scale only for normal part
            // v0 only scaling up intrinsic term...
            //      double errorCorrected = sqrt(res.dposition*res.dposition - res0.dposition*res0.dposition
            //                                   + m_errorScaler*m_errorScaler*res0.dposition*res0.dposition);
            // v1 scaling up total uncertainty and add up misalignment term....like sqrt( sigma_nominal^2 * alpha^2 + beta^2)
            double nominal_error = res.dposition;
            double errorCorrected =
                std::sqrt(nominal_error * nominal_error * m_errorScaler * m_errorScaler + m_errorScalerBeta * m_errorScalerBeta);
            if (errorCorrected < m_minimumError) errorCorrected = m_minimumError;

            Amg::MatrixX newloce(Amg::MatrixX(1, 1));
            newloce.setIdentity();
            newloce *= errorCorrected * errorCorrected;
            if (!m_cscErrorScalingKey.key().empty()) {
                SG::ReadCondHandle<RIO_OnTrackErrorScaling> error_scaling(m_cscErrorScalingKey);
                newloce =
                    Trk::ErrorScalingCast<MuonEtaPhiRIO_OnTrackErrorScaling>(
                        *error_scaling)
                        ->getScaledCovariance(std::move(newloce), Trk::distPhi);
            }

            ATH_MSG_VERBOSE("All: new err matrix is " << newloce);
            ATH_MSG_VERBOSE("  dpos changed ====> " << Amg::error(newloce, Trk::loc1));

            MClT = new CscClusterOnTrack(MClus, std::move(locpar), std::move(newloce),
                                         positionAlongStrip, MClus->status(), MClus->timeStatus(), MClus->time());
            ATH_MSG_VERBOSE("global postion of MClT :::: " << MClT->globalPosition());
        }

        return MClT;
    }

    ///////
    MuonClusterOnTrack* CscClusterOnTrackCreator::correct(const Trk::PrepRawData& RIO, const Trk::TrackParameters& TP) const {
        return createRIO_OnTrack(RIO, TP.position(), TP.momentum().unit());
    }

    ///////
    MuonClusterOnTrack* CscClusterOnTrackCreator::correct(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP, const Amg::Vector3D& GD) const {
        return createRIO_OnTrack(RIO, GP, GD);
    }

    /// These functions are provided from the interface
    const ToolHandle<ICscStripFitter>& CscClusterOnTrackCreator::GetICscStripFitter() const { return m_stripFitter; }
    const ToolHandle<ICscClusterFitter>& CscClusterOnTrackCreator::GetICscClusterFitter() const { return m_clusterFitter; }
    const ToolHandle<ICscClusterUtilTool>& CscClusterOnTrackCreator::GetICscClusterUtilTool() const { return m_clusterUtilTool; }

}  // namespace Muon
