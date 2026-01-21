///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// METNetSig.h
// Header file for class METNetSig
// Author: Alberto Plebani <alberto.plebani@cern.ch>, based on earlier implementation by M. Leigh and Bill Balunas
///////////////////////////////////////////////////////////////////

#ifndef METUTILITIES_MET_METNETSIG_H
#define METUTILITIES_MET_METNETSIG_H

// STL includes
#include <string>

// Asg tool includes
#include "AsgTools/AsgTool.h"
#include "AsgTools/AnaToolHandle.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/ReadHandleKey.h"

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"

// METInterface includes
#include "METInterface/IMETMaker.h"
#include "METInterface/IMETSignificance.h"

// Network handler
#include "METNetSigHandler.h"

// Forward declarations
class IMETMaker;
class METNetSigHandler;

namespace met {

  /** METNetSig tool class used to create a missing transverse momentum estimate using a pre-trained neural network.
  * The tool inherits from the same interface as METMaker and so it should be largely interchangable.
  * However, it does not use every public method as METMaker, as it must ensure that the
  * exact variables are constructed for the network's inputs.
  * Uses an ONNX runtime environment for inference.
  */
  class METNetSig final: public asg::AsgTool, virtual public IMETMaker {

    // This macro defines the constructor with the interface declaration
    ASG_TOOL_CLASS( METNetSig, IMETMaker )

  public:

    /// Constructor
    METNetSig( const std::string& name );

    /// Destructor
    virtual ~METNetSig() override;

    /// Athena algtool's initialize
    virtual StatusCode initialize() override;

    /// Adds a collection of objects to the core MET container, must be done for photons, elections and muons.
    virtual StatusCode rebuildMET( const std::string& metKey,
                           xAOD::Type::ObjectType metType,
                           xAOD::MissingETContainer* metCont,
                           const xAOD::IParticleContainer* collection,
                           xAOD::MissingETAssociationHelper& helper,
                           MissingETBase::UsageHandler::Policy objScale ) const override;

    /// Adds multiple jet definitions to seperate MET containers and creates the collection of input features for the neural network
    virtual StatusCode rebuildJetMET( const std::string& metJetKey,
                              const std::string& softTrkKey,
                              xAOD::MissingETContainer* metCont,
                              const xAOD::JetContainer* jets,
                              const xAOD::MissingETContainer* metCoreCont,
                              xAOD::MissingETAssociationHelper& helper,
                              bool doJetJVT = false) const override;

    /// Uses ONNX runtime to propagate the input features created in rebuildJetMET through the trained network
    virtual StatusCode evaluateNNMETSig( xAOD::MissingETContainer* metCont,
                  float& met_x, float& met_y, float& sigma_x, float& sigma_y ) const;


    /// Unsuported method inherited from METMaker. Please do not use.
    virtual StatusCode rebuildMET( xAOD::MissingET* met,
                           const xAOD::IParticleContainer* collection,
                           xAOD::MissingETAssociationHelper& helper,
                           MissingETBase::UsageHandler::Policy objScale ) const override;

    /// Unsuported method inherited from METMaker. Please do not use.
    virtual StatusCode rebuildMET( xAOD::MissingET* met,
                           const xAOD::IParticleContainer* collection,
                           xAOD::MissingETAssociationHelper& helper,
                           MissingETBase::UsageHandler::Policy p,
                           bool removeOverlap,
                           MissingETBase::UsageHandler::Policy objScale ) const override;

    /// Unsuported method inherited from METMaker. Please do not use.
    virtual StatusCode rebuildJetMET( const std::string& metJetKey,
                              const std::string& softClusKey,
                              const std::string& softTrkKey,
                              xAOD::MissingETContainer* metCont,
                              const xAOD::JetContainer* jets,
                              const xAOD::MissingETContainer* metCoreCont,
                              xAOD::MissingETAssociationHelper& helper,
                              bool doJetJVT ) const override;

    /// Unsuported method inherited from METMaker. Please do not use.
    virtual StatusCode rebuildJetMET( xAOD::MissingET* metJet,
                              const xAOD::JetContainer* jets,
                              xAOD::MissingETAssociationHelper& helper,
                              xAOD::MissingET* metSoftClus,
                              const xAOD::MissingET* coreSoftClus,
                              xAOD::MissingET* metSoftTrk,
                              const xAOD::MissingET* coreSoftTrk,
                              bool doJetJVT,
                              bool tracksForHardJets = false,
                              std::vector<const xAOD::IParticle*>* softConst = 0 ) const override;

    /// Unsuported method inherited from METMaker. Please do not use.
    virtual StatusCode rebuildTrackMET( const std::string& metJetKey,
                                const std::string& softTrkKey,
                                xAOD::MissingETContainer* metCont,
                                const xAOD::JetContainer* jets,
                                const xAOD::MissingETContainer* metCoreCont,
                                xAOD::MissingETAssociationHelper& helper,
                                bool doJetJVT ) const override;

    /// Unsuported method inherited from METMaker. Please do not use.
    virtual StatusCode rebuildTrackMET( xAOD::MissingET* metJet,
                                const xAOD::JetContainer* jets,
                                xAOD::MissingETAssociationHelper& helper,
                                xAOD::MissingET* metSoftTrk,
                                const xAOD::MissingET* coreSoftTrk,
                                bool doJetJVT ) const override;

    /// Unsuported method inherited from METMaker. Please do not use.
    virtual StatusCode markInvisible( const xAOD::IParticleContainer* collection,
                              xAOD::MissingETAssociationHelper& helper,
                              xAOD::MissingETContainer* metCont ) const override;


  private:

    // Class properties
    Gaudi::Property<std::string> m_netSigLocation{this, "NetworkLocation", "", "Location of NN file to use for METNetSig"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey", "EventInfo", "Event info key"};
    SG::ReadHandleKey<xAOD::VertexContainer> m_pvContainerKey{this, "PVContainerKey", "PrimaryVertices", "Primary vertex container"};
    SG::ReadHandleKey<xAOD::JetContainer> m_jetContainerKey{this, "JetContainer", "", "Name of input jet container"};

    // Default constructor
    METNetSig();

    std::unique_ptr<METNetSigHandler> m_metnetsighandler;
    ToolHandle<IMETMaker> m_metmaker_loose{this, "METMakerLoose",     "", "METMaker for Loose WP. Do not configure manually except for expert usage." };
    ToolHandle<IMETMaker> m_metmaker_tight{this, "METMakerTight",     "", "METMaker for Tight WP. Do not configure manually except for expert usage." };
    ToolHandle<IMETMaker> m_metmaker_tghtr{this, "METMakerTighter",   "", "METMaker for Tighter WP. Do not configure manually except for expert usage." };
    ToolHandle<IMETMaker> m_metmaker_tenac{this, "METMakerTenacious", "", "METMaker for Tenacious WP. Do not configure manually except for expert usage." };

    // Private methods
    StatusCode addMETFinal( const std::string& WP_name, xAOD::MissingETContainer* met_container, std::vector<std::string>& name_vec, std::vector<float>& val_vec ) const;
    StatusCode addMETTerm( const std::string& WP_name, xAOD::MissingET* met, std::vector<std::string>& name_vec, std::vector<float>& val_vec ) const;
    StatusCode addInputValue( const std::string& var_name, float value,std::vector<std::string>& name_vec, std::vector<float>& val_vec ) const;

    StatusCode copyMETContainer( xAOD::MissingETContainer* new_container, const xAOD::MissingETContainer* old_container) const;

  };

}

#endif