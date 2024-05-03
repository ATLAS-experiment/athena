/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// AlgTool used for MuonClusterOnTrack object production
///////////////////////////////////////////////////////////////////

#include "MuonClusterOnTrackCreator.h"

#include <cmath>
#include <sstream>

#include "MuonPrepRawData/RpcPrepData.h"
#include "MuonPrepRawData/TgcPrepData.h"
#include "MuonPrepRawData/sTgcPrepData.h"
#include "MuonRIO_OnTrack/RpcClusterOnTrack.h"
#include "MuonRIO_OnTrack/TgcClusterOnTrack.h"
#include "MuonRIO_OnTrack/MMClusterOnTrack.h"
#include "MuonRIO_OnTrack/sTgcClusterOnTrack.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkEventPrimitives/LocalDirection.h"
#include "TrkSurfaces/Surface.h"

#define SIG_VEL 4.80000  // ns/m
#define C_VEL 3.33564    // ns/m

namespace Muon {

    //================================================================================
    MuonClusterOnTrackCreator::MuonClusterOnTrackCreator(const std::string& ty, const std::string& na, const IInterface* pa) :
        AthAlgTool(ty, na, pa) {
        // algtool interface - necessary!
        declareInterface<IMuonClusterOnTrackCreator>(this);
        declareInterface<IRIO_OnTrackCreator>(this);

    }

    //================================================================================
    StatusCode MuonClusterOnTrackCreator::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());

        ATH_CHECK(m_clusterBuilderToolMM.retrieve(DisableTool{m_calibToolNSW.empty()}));
        ATH_CHECK(m_calibToolNSW.retrieve(DisableTool{m_calibToolNSW.empty()}));
        return StatusCode::SUCCESS;
    }

    //================================================================================
    MuonClusterOnTrack* MuonClusterOnTrackCreator::createRIO_OnTrack(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP) const

    {
        MuonClusterOnTrack* MClT = nullptr;

        // check whether PrepRawData has detector element, if not there print warning
        const Trk::TrkDetElementBase* EL = RIO.detectorElement();
        if (!EL) {
            ATH_MSG_WARNING("RIO does not have associated detectorElement!, cannot produce ROT");
            return nullptr;
        }

        // in RIO_OnTrack the local param and cov should have the same dimension
        Trk::LocalParameters locpar(RIO.localPosition());

        if (RIO.localCovariance().cols() != RIO.localCovariance().rows()) {
            ATH_MSG_WARNING("Rows and colums not equal!");
            if (m_idHelperSvc->isRpc(RIO.identify())) {
               ATH_MSG_WARNING("RPC hit with (r,c)=" << RIO.localCovariance().rows() << "," << RIO.localCovariance().cols());
            }
        }

        if (RIO.localCovariance().cols() > 1 || (m_idHelperSvc->isTgc(RIO.identify()) && m_idHelperSvc->measuresPhi(RIO.identify()))) {
            ATH_MSG_VERBOSE("Making 2-dim local parameters: " << m_idHelperSvc->toString(RIO.identify()));
        } else {
            Trk::DefinedParameter radiusPar(RIO.localPosition().x(), Trk::locX);
            locpar = Trk::LocalParameters(radiusPar);
            ATH_MSG_VERBOSE("Making 1-dim local parameters: " << m_idHelperSvc->toString(RIO.identify()));
        }

        Amg::Vector2D lp{Amg::Vector2D::Zero()};
        double positionAlongStrip{0};
        double positionAlongZ{0};

        const Trk::Surface& rio_surface = EL->surface(RIO.identify());
        if (!rio_surface.globalToLocal(GP, GP, lp)) {
            Amg::Vector3D lpos = rio_surface.transform().inverse() * GP;
            ATH_MSG_WARNING("Extrapolated GlobalPosition not on detector surface! Distance " << lpos.z());
            lp[Trk::locX] = lpos.x();
            lp[Trk::locY] = lpos.y();
            positionAlongZ = lpos.z();
        }

        positionAlongStrip = lp[Trk::locY];

        Amg::MatrixX loce = RIO.localCovariance();
        ATH_MSG_DEBUG("All: new err matrix is " << loce);

        if (m_idHelperSvc->isRpc(RIO.identify())) {

            //***************************
            // RPC: cast to RpcPrepData
            //***************************

            const RpcPrepData* MClus = static_cast<const RpcPrepData*>(&RIO);
            bool measphi = m_idHelperSvc->measuresPhi(RIO.identify());

            double fixedError = 1.;
            bool scale = false;
            // check whether to scale eta/phi hit
            if (m_doFixedErrorRpcEta && !measphi) {
                scale = true;
                fixedError = m_fixedErrorRpcEta;
            } else if (m_doFixedErrorRpcPhi && measphi) {
                scale = true;
                fixedError = m_fixedErrorRpcPhi;
            }
            if (scale) {
                Amg::MatrixX mat(1, 1);
                mat(0, 0) = fixedError * fixedError;
                loce = mat;
            }

            const MuonGM::RpcReadoutElement* rpc_readout_element = MClus->detectorElement();
            Amg::Vector3D posi = rpc_readout_element->stripPos(RIO.identify());

            // let's correct rpc time subtracting delay due to the induced electric signal propagation along strip
            double correct_time_along_strip = 0;
            if (measphi == 0) {
                correct_time_along_strip = rpc_readout_element->distanceToEtaReadout(GP) / 1000. * SIG_VEL;
            } else {
                correct_time_along_strip = rpc_readout_element->distanceToPhiReadout(GP) / 1000. * SIG_VEL;
            }
            if (positionAlongZ) correct_time_along_strip = 0;  // no correction if extrapolated GlobalPosition not on detector surface!

            // let's evaluate the average  delay due to the induced electric signal propagation along strip
            double av_correct_time_along_strip = 0;
            if (measphi == 0) {
                av_correct_time_along_strip = rpc_readout_element->distanceToEtaReadout(posi) / 1000. * SIG_VEL;
            } else {
                av_correct_time_along_strip = rpc_readout_element->distanceToPhiReadout(posi) / 1000. * SIG_VEL;
            }

            // let's evaluate [real TOF - nominal TOF]
            double real_TOF_onRPCgap = GP.mag() / 1000. * C_VEL;
            double nominal_TOF_onRPCgap = posi.mag() / 1000. * C_VEL;

            // let's evaluate the total time correction
            double correct_time_tot = real_TOF_onRPCgap - nominal_TOF_onRPCgap + correct_time_along_strip - av_correct_time_along_strip;

            MClT = new RpcClusterOnTrack(MClus, std::move(locpar), std::move(loce), positionAlongStrip, MClus->time() - correct_time_tot);

            ATH_MSG_DEBUG(" correct_time_along_strip " << correct_time_along_strip << " av_correct_time_along_strip "
                                                       << av_correct_time_along_strip << " real_TOF_onRPCgap " << real_TOF_onRPCgap
                                                       << " nominal_TOF_onRPCgap " << nominal_TOF_onRPCgap << " MClus->time() "
                                                       << MClus->time() << " correct_time_tot " << correct_time_tot);

        } else if (m_idHelperSvc->isTgc(RIO.identify())) {

            //***************************
            // TGC: cast to TgcPrepData
            //***************************

            const TgcPrepData* MClus = static_cast<const TgcPrepData*>(&RIO);

            // calculation of 2D error matrix for TGC phi strips
            if (m_idHelperSvc->measuresPhi(RIO.identify())) {
                int stationeta = m_idHelperSvc->stationEta(RIO.identify());
                int stripNo = m_idHelperSvc->tgcIdHelper().channel(RIO.identify());
                int gasGap = m_idHelperSvc->tgcIdHelper().gasGap(RIO.identify());

                const MuonGM::TgcReadoutElement* ele = MClus->detectorElement();

                double stripLength = ele->stripLength(gasGap, stripNo);
                double stripWidth = std::fabs(ele->stripMaxX(gasGap, stripNo, lp[Trk::locZ]) - ele->stripMinX(gasGap, stripNo, lp[Trk::locZ]));

                double localX1 = ele->stripCtrX(gasGap, stripNo, stripLength / 2.);
                double localX2 = ele->stripCtrX(gasGap, stripNo, -stripLength / 2.);
                if (stationeta > 0) {
                    localX1 = -localX1;
                    localX2 = -localX2;
                }
                Amg::MatrixX mat(2, 2);

                double phistereo = std::atan2(localX2 - localX1, stripLength);
                double Sn = std::sin(phistereo);
                double Sn2 = Sn * Sn;
                double Cs2 = 1. - Sn2;

                double V0 = stripWidth * stripWidth / 12;
                if (m_doFixedErrorTgcPhi) V0 = m_fixedErrorTgcPhi * m_fixedErrorTgcPhi;
                double V1 = stripLength * stripLength / 12;
                mat(0, 0) = (Cs2 * V0 + Sn2 * V1);
                mat.fillSymmetric(1, 0, (Sn * std::sqrt(Cs2) * (V0 - V1)));
                mat(1, 1) = (Sn2 * V0 + Cs2 * V1);
                loce = mat;
            } else {
                if (m_doFixedErrorTgcEta) {
                    Amg::MatrixX mat(1, 1);
                    mat(0, 0) = m_fixedErrorTgcEta * m_fixedErrorTgcEta;
                    loce = mat;
                }
            }

            MClT = new TgcClusterOnTrack(MClus, std::move(locpar), std::move(loce), positionAlongStrip);

        } else if (m_idHelperSvc->issTgc(RIO.identify())) {

            //***************************
            // sTGC: cast to sTgcPrepData
            //***************************

            const sTgcPrepData* MClus = static_cast<const sTgcPrepData*>(&RIO);
            Amg::Vector2D localPos(lp[Trk::locX], lp[Trk::locY]);

            // Dont make RIO On tracks for sTGC wires in inner Q1
            if (m_idHelperSvc->stgcIdHelper().channelType(MClus->identify()) == sTgcIdHelper::Wire && 
                MClus->detectorElement()->isEtaZero(MClus->identify(), lp)) {
              ATH_MSG_DEBUG("sTgcReadoutElement with isEtaZero() ?! "<<m_idHelperSvc->toString(MClus->identify()));
              return nullptr;
            }
            // Wires are already considered in the above check. Dont remove them here
            if (!rio_surface.insideBounds(localPos) && 
                 m_idHelperSvc->stgcIdHelper().channelType(MClus->identify()) != sTgcIdHelper::Wire) {
              ATH_MSG_DEBUG("sTgc measurement "<<m_idHelperSvc->toString(MClus->identify())<<" out of bounds. "
                             <<Amg::toString(localPos));
              return nullptr;
        }
            MClT = new sTgcClusterOnTrack(MClus, std::move(locpar), std::move(loce), positionAlongStrip);

        } else if (m_idHelperSvc->isMM(RIO.identify())) {

            //***************************
            // MM: cast to MMPrepData
            //***************************

            const MMPrepData* mmPRD = static_cast<const MMPrepData*>(&RIO);
            MClT = new MMClusterOnTrack(mmPRD, std::move(locpar), std::move(loce), positionAlongStrip, {}, {});
        }

        return MClT;
    }


    //================================================================================
    MuonClusterOnTrack* MuonClusterOnTrackCreator::createRIO_OnTrack(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP, const Amg::Vector3D&) const {
        return createRIO_OnTrack(RIO, GP);
    }

    //================================================================================
    MuonClusterOnTrack* MuonClusterOnTrackCreator::correct(const Trk::PrepRawData& RIO, const Trk::TrackParameters& TP) const {
        return correct(RIO, TP.position(), TP.momentum().unit());
    }

    //================================================================================
    MuonClusterOnTrack* MuonClusterOnTrackCreator::correct(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP, const Amg::Vector3D& GD) const {

        if (m_idHelperSvc->isMM(RIO.identify())) {
            // Micromegas
            return calibratedClusterMMG(RIO, GP, GD);
        }

        if (m_idHelperSvc->issTgc(RIO.identify()) && !m_idHelperSvc->measuresPhi(RIO.identify())) {
            // sTGC: calibration is currently available for strips.
            return calibratedClusterSTG(RIO, GP, GD);
        }

        // Default case
        return createRIO_OnTrack(RIO, GP, GD);
    }


    //================================================================================
    MuonClusterOnTrack* MuonClusterOnTrackCreator::calibratedClusterMMG(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP, const Amg::Vector3D& GD) const {
        const EventContext& ctx{Gaudi::Hive::currentContext()};
        // Make sure RIO has a detector element
        const MuonGM::MMReadoutElement* mmEL = static_cast<const MuonGM::MMReadoutElement*>(RIO.detectorElement());
        if (!mmEL) {
            ATH_MSG_WARNING("RIO does not have associated detectorElement! Skipping cluster calibration");
            return nullptr;
        }

        Amg::MatrixX loce = RIO.localCovariance();
        Trk::LocalParameters locpar = loce.cols() > 1 ? Trk::LocalParameters{RIO.localPosition()} : Trk::LocalParameters{Trk::DefinedParameter{RIO.localPosition().x(), Trk::locX}};

        // * Local cluster coordinates to feed to the calibration tools
        Amg::Vector2D lp{Amg::Vector2D::Zero()};

        //   get localY from the seeded position
        const Trk::PlaneSurface& rio_surface = mmEL->surface(RIO.identify());
        if (!rio_surface.globalToLocal(GP, GP, lp)) {
            Amg::Vector3D lpos = rio_surface.transform().inverse() * GP;
            ATH_MSG_WARNING("Extrapolated GlobalPosition not on detector surface! Distance " << lpos.z());
            lp[Trk::locX] = lpos.x();
            lp[Trk::locY] = lpos.y();
        }
        //   set localX from the cluster parameters
        lp[Trk::locX] = locpar[Trk::locX];

        // * B-Field correction
        //   calibrate the input strips
        const MMPrepData* mmPRD = static_cast<const MMPrepData*>(&RIO);

        std::vector<NSWCalib::CalibratedStrip> calibratedStrips;
        StatusCode sc = m_calibToolNSW->calibrateClus(ctx, mmPRD, GP, calibratedStrips);
        if (sc != StatusCode::SUCCESS) {
            ATH_MSG_WARNING("Could not calibrate the MM Cluster in the RIO on track creator");
            return nullptr;
        }

        //   calibrate the cluster position along the precision coordinate (updates lp.x())
        IMMClusterBuilderTool::RIO_Author rotAuthor = m_clusterBuilderToolMM->getCalibratedClusterPosition(ctx,
                                                                                               calibratedStrips,
                                                                             NswClustering::toLocal(*mmPRD, GD),
                                                                                                      lp, loce);

        if (rotAuthor == IMMClusterBuilderTool::RIO_Author::unKnownAuthor) {
            ATH_MSG_WARNING("Could not calibrate the MM Cluster in the RIO on track creator");
            return nullptr;
        }

        // * Correct the local cluster coordinates for as-built conditions and B-lines (returns a new 3D vector)
        Amg::Vector3D localposition3D{Amg::Vector3D::Zero()};
        if (!mmEL->spacePointPosition(RIO.identify(), lp, localposition3D)){
            ATH_MSG_WARNING("Application of final as-built parameters failed for channel "<<m_idHelperSvc->toString(RIO.identify())<<" local pos = ("<<lp.x()<<"/"<<lp.y()<<").");
        }

        //   Get the direction of the track in the local coordinate system and use it to project
        //   the actual hit position onto the nominal surface (locZ = 0), where the intersection
        //   of the track is considered. This "effective" position provides a more accurate residual.
        Trk::LocalDirection ld;
        rio_surface.globalToLocalDirection(GD, ld);
        double a_impact    = ld.angleXZ() < 0 ? -M_PI_2 - ld.angleXZ() : M_PI_2 - ld.angleXZ();
        double x_projected = localposition3D.x() - std::tan(a_impact) * localposition3D.z();

        // * Set the value of the local parameter (locX) after applying conditions
        //   The position along strip will be set from the seed (there is no better
        //   estimate than that; not used in the track fits anyway)
        locpar[Trk::locX] = x_projected;

        ATH_MSG_VERBOSE("generating MMClusterOnTrack in MMClusterBuilder");
        MMClusterOnTrack* cluster = new MMClusterOnTrack(mmPRD, std::move(locpar), std::move(loce), lp[Trk::locY], {}, {});
        cluster->setAuthor(rotAuthor);

        return cluster;
    }


    //================================================================================
    MuonClusterOnTrack* MuonClusterOnTrackCreator::calibratedClusterSTG(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP, const Amg::Vector3D& GD) const {

        // Make sure RIO has a detector element
        const MuonGM::sTgcReadoutElement* stgEL = static_cast<const MuonGM::sTgcReadoutElement*>(RIO.detectorElement());
        if (!stgEL) {
            ATH_MSG_WARNING("RIO does not have associated detectorElement! Skipping cluster calibration");
            return nullptr;
        }

        Amg::MatrixX loce = RIO.localCovariance();
        // > 1 in case we want to keep pads in the future ?
        Trk::LocalParameters locpar = loce.cols() > 1 ? Trk::LocalParameters{RIO.localPosition()} : Trk::LocalParameters{Trk::DefinedParameter{RIO.localPosition().x(), Trk::locX}};


        // * Local cluster coordinates to feed to the calibration tools
        Amg::Vector2D lp{ Amg::Vector2D::Zero() };

        //   get local y from the seeded position
        const Trk::PlaneSurface& rio_surface = stgEL->surface(RIO.identify());
        if (!rio_surface.globalToLocal(GP, GP, lp)) {
            Amg::Vector3D lpos = rio_surface.transform().inverse() * GP;
            ATH_MSG_WARNING("Extrapolated GlobalPosition not on detector surface! Distance " << lpos.z());
            lp[Trk::locX] = lpos.x();
            lp[Trk::locY] = lpos.y();
        }
        //   set local x from the cluster parameters
        lp[Trk::locX] = locpar[Trk::locX];

        // * Correct the local coordinates for as-built conditions and b-lines
        Amg::Vector3D localposition3D { Amg::Vector3D::Zero() };
        stgEL->spacePointPosition(RIO.identify(), lp[Trk::locX], lp[Trk::locY], localposition3D);

        //   Get the direction of the track in the local coordinate system and use it to project
        //   the actual hit position onto the nominal surface (locZ = 0), where the intersection
        //   of the track is considered. This "effective" position provides a more accurate residual.
        Trk::LocalDirection ld;
        rio_surface.globalToLocalDirection(GD, ld);
        double a_impact    = ld.angleXZ() < 0 ? -M_PI_2 - ld.angleXZ() : M_PI_2 - ld.angleXZ();
        double x_projected = localposition3D.x() - std::tan(a_impact) * localposition3D.z();

        // * Set the value of the local parameter (locX) after applying conditions
        //   The position along strip will be set from the seed (there is no better
        //   estimate than that; not used in the track fits anyway)
        locpar[Trk::locX] = x_projected;

        const sTgcPrepData* stgPRD = static_cast<const sTgcPrepData*>(&RIO);
        ATH_MSG_VERBOSE("generating sTgcClusterOnTrack in MuonClusterBuilder");
        MuonClusterOnTrack* cluster = new sTgcClusterOnTrack(stgPRD, std::move(locpar), std::move(loce), lp[Trk::locY]);

        return cluster;
    }
} // namespace Muon
