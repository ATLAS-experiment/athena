/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// author: cpollard@cern.ch

#ifndef PARTICLEJETTOOLS_JETPARTICLEORIGINVERTEXASSOCIATION_H
#define PARTICLEJETTOOLS_JETPARTICLEORIGINVERTEXASSOCIATION_H

#include "ParticleJetTools/JetParticleAssociation.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

#include <vector>
#include <string>

class JetParticleOriginVertexAssociation : public JetParticleAssociation {
    ASG_TOOL_CLASS(JetParticleOriginVertexAssociation, IJetDecorator)
    public:

        JetParticleOriginVertexAssociation(const std::string& name);

        virtual StatusCode initialize() override;

        virtual const std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >*
            match(const xAOD::JetContainer&, const xAOD::IParticleContainer&) const override;

        inline double coneSize(double pt) const {
            return (m_coneSizeFitPar1 + exp(m_coneSizeFitPar2 + m_coneSizeFitPar3*pt));
        }


    private:
        double m_coneSizeFitPar1;
        double m_coneSizeFitPar2;
        double m_coneSizeFitPar3;
        float  m_dzCut;
        bool   m_useMinZ0Vertex;
        SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_trkOriginKey{
            this, "TrackOriginVertexDecoration", m_particleKey, "btagIp_ByVertex_TrkOriginVtx", "Track origin vertices"};
        SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_z0SinThetaKey{
            this, "TrackZ0SinThetaDecoration", m_particleKey, "btagIp_ByVertex_z0SinTheta", "Track z0 sin(theta) per vertex"};
};

#endif
