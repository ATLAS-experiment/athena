/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// author: cpollard@cern.ch

#ifndef PARTICLEJETTOOLS_JETPARTICLEORIGINVERTEXASSOCIATION_H
#define PARTICLEJETTOOLS_JETPARTICLEORIGINVERTEXASSOCIATION_H

#include "ParticleJetTools/JetParticleAssociation.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/WriteDecorHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include <vector>
#include <string>

class JetParticleOriginVertexAssociation : public JetParticleAssociation {
    ASG_TOOL_CLASS(JetParticleOriginVertexAssociation, IJetDecorator)
    public:

        JetParticleOriginVertexAssociation(const std::string& name);

        virtual const std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >*
            match(const xAOD::JetContainer&, const xAOD::IParticleContainer&) const override;

        inline double coneSize(double pt) const {
            return (m_coneSizeFitPar1 + exp(m_coneSizeFitPar2 + m_coneSizeFitPar3*pt));
        }
        StatusCode initialize() override final;

    private:
        double m_coneSizeFitPar1;
        double m_coneSizeFitPar2;
        double m_coneSizeFitPar3;
        float  m_dzCut;
        bool   m_useMinZ0Vertex;
        bool   m_dzCut_bool;
        Gaudi::Property< std::string > m_prefix{this,"prefix","btagIp_",""};
        SG::ReadHandleKey< xAOD::TrackParticleContainer > m_TrackContainerKey {this,"TrackContainer","InDetTrackParticles","Key for the input track collection"};
        SG::WriteHandleKey< xAOD::TrackParticleContainer > m_outDuplicatedTrackContainerKey{this,"DuplicatedTrackContainer","DuplicatedTrks","Key for creating copy of track container"};
        SG::ReadHandleKey< xAOD::TrackParticleContainer > m_readDuplicatedTrackContainerKey{this,"DuplicatedTrackContainer","DuplicatedTrks","Key for reading copy of track container"};

        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_d0 {this, "ByVertex1_d0", "ByVertex1_d0", "d0 of tracks"};
        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_z0 {this, "ByVertex1_z0SinTheta", "ByVertex1_z0SinTheta", "z0SinTheta of tracks"};
        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_d0_sigma {this, "ByVertex1_d0Uncertainty", "ByVertex1_d0Uncertainty", "d0Uncertainty of tracks"};
        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_z0_sigma {this, "ByVertex1_z0SinThetaUncertainty", "ByVertex1_z0SinThetaUncertainty", "z0SinThetaUncertainty of tracks"};
        SG::WriteDecorHandleKey< xAOD::IParticleContainer > m_dec_DupTrk_link {this, "ByVertex1_DupTrk_link", "ByVertex1_DupTrk_link", "duplicated track links"};
        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_track_pos {this, "ByVertex1_trackDisplacement","ByVertex1_trackDisplacement","trackDisplacement of tracks" };
        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_track_mom {this, "ByVertex1_trackMomentum","ByVertex1_trackMomentum","trackMomentum of tracks" };
        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_trk_origin_vtx_idx {this, "ByVertex1_TrkOriginVertex_idx", "ByVertex1_TrkOriginVtx_idx", "origin vertex of track index"};
        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_invalid {
        this, "ByVertex1_invalidIp", "ByVertex1_invalidIp", "flag for invalid impact parameter"
        };

};

#endif
