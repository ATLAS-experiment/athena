///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// METMaker.h
// Header file for class METMaker
// Author: T.J.Khoo<khoo@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef METUTILITIES_MET_METMAKER_H
#define METUTILITIES_MET_METMAKER_H 1

// STL includes
#include <string>

// FrameWork includes
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/AsgTool.h"

// METInterface includes
#include "METInterface/IMETMaker.h"

// EDM includes
#include "xAODJet/JetContainer.h"
#include "xAODPFlow/PFOContainer.h"

// Tracking Tool
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"


// Forward declaration

namespace met {

  // typedefs
  typedef ElementLink<xAOD::IParticleContainer> obj_link_t;

  class METMaker
  : public asg::AsgTool,
  virtual public IMETMaker

  {
    // This macro defines the constructor with the interface declaration
    ASG_TOOL_CLASS(METMaker, IMETMaker)

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:

    // Copy constructor:

    /// Constructor with parameters:
    METMaker(const std::string& name);

    /// Destructor:
    virtual ~METMaker();

    // Athena algtool's Hooks
    virtual StatusCode initialize() override final;

    virtual StatusCode rebuildMET(
      const std::string& metKey,
      xAOD::Type::ObjectType metType,
      xAOD::MissingETContainer* metCont,
      const xAOD::IParticleContainer* collection,
      xAOD::MissingETAssociationHelper& helper,
      MissingETBase::UsageHandler::Policy objScale) override final;
    //
    virtual StatusCode rebuildMET(
      xAOD::MissingET* met,
      const xAOD::IParticleContainer* collection,
      xAOD::MissingETAssociationHelper& helper,
      MissingETBase::UsageHandler::Policy objScale) override final;
    //
    virtual StatusCode rebuildMET(
      xAOD::MissingET* met,
      const xAOD::IParticleContainer* collection,
      xAOD::MissingETAssociationHelper& helper,
      MissingETBase::UsageHandler::Policy p,
      bool removeOverlap,
      MissingETBase::UsageHandler::Policy objScale) override final;

    virtual StatusCode rebuildJetMET(
      const std::string& metJetKey,
      const std::string& softClusKey,
      const std::string& softTrkKey,
      xAOD::MissingETContainer* metCont,
      const xAOD::JetContainer* jets,
      const xAOD::MissingETContainer* metCoreCont,
      xAOD::MissingETAssociationHelper& helper,
      bool doJetJVT) override final;

    virtual StatusCode rebuildJetMET(
      const std::string& metJetKey,
      const std::string& metSoftKey,
      xAOD::MissingETContainer* metCont,
      const xAOD::JetContainer* jets,
      const xAOD::MissingETContainer* metCoreCont,
      xAOD::MissingETAssociationHelper& helper,
      bool doJetJVT) override final;

    virtual StatusCode rebuildJetMET(
      xAOD::MissingET* metJet,
      const xAOD::JetContainer* jets,
      xAOD::MissingETAssociationHelper& helper,
      xAOD::MissingET* metSoftClus,
      const xAOD::MissingET* coreSoftClus,
      xAOD::MissingET* metSoftTrk,
      const xAOD::MissingET* coreSoftTrk,
      bool doJetJVT,
      bool tracksForHardJets = false,
      std::vector<const xAOD::IParticle*>* softConst = 0) override final;

    virtual StatusCode rebuildTrackMET(
      const std::string& metJetKey,
      const std::string& softTrkKey,
      xAOD::MissingETContainer* metCont,
      const xAOD::JetContainer* jets,
      const xAOD::MissingETContainer* metCoreCont,
      xAOD::MissingETAssociationHelper& helper,
      bool doJetJVT) override final;

    virtual StatusCode rebuildTrackMET(xAOD::MissingET* metJet,
                                       const xAOD::JetContainer* jets,
                                       xAOD::MissingETAssociationHelper& helper,
                                       xAOD::MissingET* metSoftTrk,
                                       const xAOD::MissingET* coreSoftTrk,
                                       bool doJetJVT) override final;

    virtual StatusCode markInvisible(
      const xAOD::IParticleContainer* collection,
      xAOD::MissingETAssociationHelper& helper,
      xAOD::MissingETContainer* metCont) override final;

    ///////////////////////////////////////////////////////////////////
    // Private data:
    ///////////////////////////////////////////////////////////////////
  private:

    bool acceptTrack(const xAOD::TrackParticle* trk, const xAOD::Vertex* vx) const;
    const xAOD::Vertex* getPV() const;


    SG::ReadHandleKey<xAOD::VertexContainer>  m_PVkey;

    // configurable accessors
    std::unique_ptr<SG::AuxElement::ConstAccessor<char > > m_acc_jetRejectionDec;

    // pT threshold for suppressing warnings of objects missing in association map
    float m_missObjWarningPtThreshold;

    bool m_jetCorrectPhi;
    double m_jetMinEfrac;
    double m_jetMinWeightedPt;
    std::string m_jetConstitScaleMom;
    std::string m_jetJvtMomentName;
    std::string m_jetRejectionDec;

    double m_CenJetPtCut, m_FwdJetPtCut ; // jet pt cut for central/forward jets
    double m_JvtCut, m_JvtPtMax; // JVT cut and pt region of jets to apply a JVT selection
    double m_JetEtaMax;
    double m_JetEtaForw;

    std::string m_jetSelection;
    std::string m_JvtWP;

    // Extra configurables for custom WP
    double m_customCenJetPtCut,m_customFwdJetPtCut;
    double m_customJvtPtMax;
    std::string m_customJvtWP;

    bool m_doPFlow;
    bool m_doSoftTruth;
    bool m_doConstJet;

    bool m_useGhostMuons;
    bool m_doRemoveMuonJets;
    bool m_doRemoveElecTrks;
    bool m_doRemoveElecTrksEM;
    bool m_doSetMuonJetEMScale;
    bool m_skipSystematicJetSelection;

    bool m_muEloss;
    bool m_orCaloTaggedMuon;
    bool m_greedyPhotons;
    bool m_veryGreedyPhotons;

    // muon overlap variables
    int m_jetTrkNMuOlap;
    double m_jetWidthMuOlap;
    double m_jetPsEMuOlap;
    double m_jetEmfMuOlap;
    double m_jetTrkPtMuPt;
    double m_muIDPTJetPtRatioMuOlap;

    ToolHandle<InDet::IInDetTrackSelectionTool> m_trkseltool;
    ToolHandle<IAsgSelectionTool> m_JvtTool;

    /// Default constructor:
    METMaker();

  };

} //> end namespace met
#endif //> !METUTILITIES_MET_METMAKER_H
