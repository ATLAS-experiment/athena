/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// author: cpollard@cern.ch

#ifndef PARTICLEJETTOOLS_JETPARTICLEORIGINVERTEXASSOCIATION_H
#define PARTICLEJETTOOLS_JETPARTICLEORIGINVERTEXASSOCIATION_H

#include "ParticleJetTools/JetParticleAssociation.h"
#include "AsgDataHandles/ReadDecorHandle.h"

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


    private:
        double m_coneSizeFitPar1;
        double m_coneSizeFitPar2;
        double m_coneSizeFitPar3;
        float  m_dzCut;
        bool   m_useMinZ0Vertex;
};

#endif
