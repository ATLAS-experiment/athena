/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "BeamSpotPreparatorAlg.h"


#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/UnitVectors.hpp"

using namespace Acts::UnitLiterals;



namespace MuonCombinedR4 {
    StatusCode BeamSpotPreparatorAlg::initialize()  {
        ATH_CHECK(m_beamSpotHandle.initialize(m_writeKey));
        ATH_CHECK(m_vertexKey.initialize(!m_useBeamSpot));
        ATH_CHECK(m_beamSpotKey.initialize(m_useBeamSpot));
        ATH_CHECK(m_ctxProvider.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode BeamSpotPreparatorAlg::execute(const EventContext& ctx) const {
        const InDet::BeamSpotData* beamSpotData{nullptr};
        ATH_CHECK(SG::get(beamSpotData, m_beamSpotKey, ctx));
        const xAOD::VertexContainer* vtxContainer{nullptr};
        ATH_CHECK(SG::get(vtxContainer, m_vertexKey, ctx));

        auto measCreator = m_beamSpotHandle.makeHandle(ctx, m_ctxProvider.getGeometryContext(ctx));
        if (!measCreator.ok()) {
            ATH_MSG_ERROR("Cannot create the auxilary measurement container");
            return StatusCode::FAILURE;
        }
        using ProjectorType = xAOD::AuxiliaryMeasurement::ProjectorType;
        if (vtxContainer) {
            for (const xAOD::Vertex* vertex : *vtxContainer) {
                if (vertex->vertexType() != xAOD::VxType::PriVtx ) {
                    continue;
                }
                Amg::Isometry3D surfaceTrf = Amg::getTranslate3D(vertex->position());
                Acts::Matrix<2,3> projector{Acts::Matrix<2,3>::Zero()};
                projector.row(0) = m_sigmaScaleR* Acts::makeDirectionFromPhiTheta(vertex->position().phi(), 
                                                                   90._degree);
                projector.row(1) = m_sigmaScaleZ * Amg::Vector3D::UnitZ();

                AmgSymMatrix(2) cov = projector * vertex->covariancePosition() * projector.transpose();

                auto surface = Acts::Surface::makeShared<Acts::PerigeeSurface>(surfaceTrf);
                measCreator->newMeasurement<2>(surface, ProjectorType::e2DimNoTime, cov);
            }
        } else if (beamSpotData) {
            const InDet::BeamSpotData* beamSpot{nullptr};
            ATH_CHECK(SG::get(beamSpot, m_beamSpotKey, ctx));
            Amg::Isometry3D beamSpotTrf = Amg::getTranslate3D(beamSpot->beamPos()) *
                                           Amg::getRotateY3D(beamSpot->beamTilt(0)) *
                                           Amg::getRotateX3D(beamSpot->beamTilt(1));
            AmgSymMatrix(3) beamCov{AmgSymMatrix(3)::Identity()};
            beamCov(0,0) = beamSpot->beamSigma(0);
            beamCov(1,1) = beamSpot->beamSigma(1);
            beamCov(2,2) = beamSpot->beamSigma(2);
            beamCov(0,1) = beamCov(1,0) = beamSpot->beamSigma(5);
            
            Acts::Matrix<2,3> projector{Acts::Matrix<2,3>::Zero()};
            projector(0,0) = m_sigmaScaleR* Amg::Vector3D::UnitX().dot(beamSpotTrf.linear() * Amg::Vector3D::UnitX());
            projector(0,1) = m_sigmaScaleR* Amg::Vector3D::UnitY().dot(beamSpotTrf.linear() * Amg::Vector3D::UnitY());
            projector(1,2) =  m_sigmaScaleZ;
            
            AmgSymMatrix(2) cov = projector * beamCov * projector.transpose();
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Covariance \n"<<cov
                <<",\n projector:\n"<<projector);

            auto surface = Acts::Surface::makeShared<Acts::PerigeeSurface>(beamSpotTrf);
            measCreator->newMeasurement<2>(surface, ProjectorType::e2DimNoTime, cov); 
        }        
        return StatusCode::SUCCESS;
    }
}