/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// author: cpollard@cern.ch

#ifndef PARTICLEJETTOOLS_JETPARTICLEASSOCIATION_H
#define PARTICLEJETTOOLS_JETPARTICLEASSOCIATION_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "xAODJet/JetContainer.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "JetInterface/IJetDecorator.h"

#include <vector>
#include <string>


class JetParticleAssociation : public asg::AsgTool,
                               virtual public IJetDecorator {
    ASG_TOOL_CLASS(JetParticleAssociation, IJetDecorator)

    public:

        JetParticleAssociation(const std::string& name);

        virtual StatusCode initialize() override;
        virtual StatusCode decorate(const xAOD::JetContainer& jets) const override;

        // obvs to be provided by the deriving class
        virtual const std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >*
            match(const xAOD::JetContainer&, const xAOD::IParticleContainer&) const = 0;
        // Mario: adding this overload because we have an extra argument
        virtual const std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >*
            match(SG::ReadDecorHandleKey<xAOD::IParticleContainer>, const xAOD::JetContainer&, const xAOD::IParticleContainer&) const = 0;
        
    private:

        // note
        // if m_particleKey is "", then an empty container will be written to the OutputDecoration.
        Gaudi::Property<std::string> m_jetContainerName{this, "JetContainer", "", "Jet collection name"};
        Gaudi::Property<float> m_ptMinimum{this, "MinimumJetPt", 0.0, "minimum pt to consider for association"};
        SG::ReadHandleKey<xAOD::IParticleContainer> m_particleKey{this, "InputParticleContainer", "", "Input particle collection name"};
        SG::WriteDecorHandleKey<xAOD::JetContainer> m_decKey{this, "OutputDecoration", "", "Output decoration name"};
        SG::WriteDecorHandleKey<xAOD::JetContainer> m_passPtKey{this, "PassPtFlag", "", "Name for decoration indicating we passed pt threshold"};
        //Mario adding the track-vertex association
        SG::ReadDecorHandleKey<xAOD::IParticleContainer> m_trk_origin_vtx { this, "btagIp_TrkOriginVtx", "InDetTrackParticles.btagIp_TrkOriginVtx", "Decoration for vertex matching to track" }; 
};

#endif
