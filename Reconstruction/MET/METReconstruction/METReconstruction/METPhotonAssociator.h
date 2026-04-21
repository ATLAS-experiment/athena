///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// METPhotonAssociator.h 
// Header file for class METPhotonAssociator
//
//  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
// Author: P Loch, S Resconi, TJ Khoo, AS Mete
/////////////////////////////////////////////////////////////////// 
#ifndef METRECONSTRUCTION_METPHOTONASSOCIATOR_H
#define METRECONSTRUCTION_METPHOTONASSOCIATOR_H 1

// METReconstruction includes
#include "METReconstruction/METEgammaAssociator.h"
#include "xAODEgamma/PhotonFwd.h"

namespace met{
  class METPhotonAssociator final
    : public METEgammaAssociator
  { 
    // This macro defines the constructor with the interface declaration
    ASG_TOOL_CLASS(METPhotonAssociator, IMETAssocToolBase)


    /////////////////////////////////////////////////////////////////// 
    // Public methods: 
    /////////////////////////////////////////////////////////////////// 
    public: 

    // Constructor with name
    METPhotonAssociator(const std::string& name);
    ~METPhotonAssociator() = default;

    // AsgTool Hooks
    virtual StatusCode initialize() override;

    /////////////////////////////////////////////////////////////////// 
    // Private data: 
    /////////////////////////////////////////////////////////////////// 
    protected: 

    StatusCode executeTool(xAOD::MissingETContainer* metCont, xAOD::MissingETAssociationMap* metMap, const EventContext& ctx) const final;

    private:
 
    /// Default constructor: 
    METPhotonAssociator();

    SG::ReadHandleKey<xAOD::PhotonContainer> m_phContKey{this, "InputCollection", "Photons", "photons input key"};

  }; 

}

#endif //> !METRECONSTRUCTION_METPHOTONASSOCIATOR_H
