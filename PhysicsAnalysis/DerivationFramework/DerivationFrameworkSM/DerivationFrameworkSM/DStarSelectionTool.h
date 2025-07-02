/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//==================================================
// Selection of D*+ -> pi+ + D0
//==================================================

#ifndef DERIVATIONFRAMEWORK_DStarSelectionTool_H
#define DERIVATIONFRAMEWORK_DStarSelectionTool_H

#include <vector>
#include <string>

#include "TLorentzVector.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticle.h"

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"

#include <AsgTools/PropertyWrapper.h>



namespace DerivationFramework {

  class DStarSelectionTool : public extends<AthAlgTool,IAugmentationTool> {
    
    public: 
    using base_class::base_class;
    
    StatusCode initialize() override;
    
    virtual StatusCode addBranches() const override;
    
  private:
    SG::ReadHandleKey<xAOD::VertexContainer> m_inputVtxContainerName{this, "InputVtxContainerName", ""};
    Gaudi::Property<double> m_deltaMassMax{this,"DeltaMassMax" ,200.,"invariant mass difference range"};
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackKey{this, "TrackContainer", "InDetTrackParticles"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackDecoKey{ this, "PassTrackDstarKey", m_trackKey, "trackPassDstar"};
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_vertexDecoKey{ this, "PassVertexDstarKey", m_inputVtxContainerName, "passed_Dstar"};
    
    const double m_pionMass = ParticleConstants::chargedPionMassInMeV;
    const double m_kaonMass = 493.677;
    std::string m_hypoName; //!< name of the mass hypothesis prefix for decorations
  }; 
}

#endif // DERIVATIONFRAMEWORK_DStarSelectionTool_H
