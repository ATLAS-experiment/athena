///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// ColumnarMETMaker.h
// Header file for class ColumnarMETMaker
// Author: T.J.Khoo<khoo@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef METUTILITIES_COLUMNAR_MET_METMAKER_H
#define METUTILITIES_COLUMNAR_MET_METMAKER_H 1

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

#include <AsgTools/PropertyWrapper.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/ParticleDef.h>
#include <ColumnarCore/StringColumn.h>
#include <ColumnarJet/JetDef.h>
#include <ColumnarMet/MetAssociation.h>
#include <ColumnarMet/MetHelpers.h>
#include <ColumnarMet/MetOutput.h>

// Forward declaration

namespace met {

  // typedefs
  typedef ElementLink<xAOD::IParticleContainer> obj_link_t;

  class ColumnarMETMaker
  : public asg::AsgTool, public columnar::ColumnarTool<>,
  virtual public IMETMaker

  {
    // This macro defines the constructor with the interface declaration
    ASG_TOOL_CLASS(ColumnarMETMaker, IMETMaker)

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:

    // Copy constructor:

    /// Constructor with parameters:
    ColumnarMETMaker(const std::string& name);

    /// Destructor:
    virtual ~ColumnarMETMaker();

    // Athena algtool's Hooks
    virtual StatusCode initialize() override final;

    virtual StatusCode rebuildMET(
      const std::string& metKey,
      xAOD::Type::ObjectType metType,
      xAOD::MissingETContainer* metCont,
      const xAOD::IParticleContainer* collection,
      xAOD::MissingETAssociationHelper& helper,
      MissingETBase::UsageHandler::Policy objScale) const override final;
    StatusCode rebuildMET(
      const std::string& metKey,
      xAOD::Type::ObjectType metType,
      columnar::MutableMetRange metCont,
      columnar::ParticleRange collection,
      columnar::MetAssociationHelper<> helper,
      MissingETBase::UsageHandler::Policy objScale) const;
    //
    virtual StatusCode rebuildMET(
      xAOD::MissingET* met,
      const xAOD::IParticleContainer* collection,
      xAOD::MissingETAssociationHelper& helper,
      MissingETBase::UsageHandler::Policy objScale) const override final;
    StatusCode rebuildMET(
      columnar::MutableMetId met,
      columnar::ParticleRange collection,
      columnar::MetAssociationHelper<> helper,
      MissingETBase::UsageHandler::Policy objScale) const;
    //
    virtual StatusCode rebuildMET(
      xAOD::MissingET* met,
      const xAOD::IParticleContainer* collection,
      xAOD::MissingETAssociationHelper& helper,
      MissingETBase::UsageHandler::Policy p,
      bool removeOverlap,
      MissingETBase::UsageHandler::Policy objScale) const override final;
    StatusCode rebuildMET(
      columnar::MutableMetId met,
      columnar::ParticleRange collection,
      columnar::MetAssociationHelper<> helper,
      MissingETBase::UsageHandler::Policy p,
      bool removeOverlap,
      MissingETBase::UsageHandler::Policy objScale) const;

    virtual StatusCode rebuildJetMET(
      const std::string& metJetKey,
      const std::string& softClusKey,
      const std::string& softTrkKey,
      xAOD::MissingETContainer* metCont,
      const xAOD::JetContainer* jets,
      const xAOD::MissingETContainer* metCoreCont,
      xAOD::MissingETAssociationHelper& helper,
      bool doJetJVT) const override final;
    StatusCode rebuildJetMET(
      const std::string& metJetKey,
      const std::string& softClusKey,
      const std::string& softTrkKey,
      columnar::MutableMetRange metCont,
      columnar::JetRange jets,
      columnar::Met1Range metCoreCont,
      columnar::MetAssociationHelper<> helper,
      bool doJetJVT) const;

    virtual StatusCode rebuildJetMET(
      const std::string& metJetKey,
      const std::string& metSoftKey,
      xAOD::MissingETContainer* metCont,
      const xAOD::JetContainer* jets,
      const xAOD::MissingETContainer* metCoreCont,
      xAOD::MissingETAssociationHelper& helper,
      bool doJetJVT) const override final;
    StatusCode rebuildJetMET(
      const std::string& metJetKey,
      const std::string& metSoftKey,
      columnar::MutableMetRange metCont,
      columnar::JetRange jets,
      columnar::Met1Range metCoreCont,
      columnar::MetAssociationHelper<> helper,
      bool doJetJVT) const;

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
      std::vector<const xAOD::IParticle*>* softConst = 0) const override final;
    StatusCode rebuildJetMET(
      columnar::MutableMetId metJet,
      columnar::MutableMetRange metCont,
      columnar::JetRange jets,
      columnar::MetAssociationHelper<> helper,
      columnar::OptMutableMetId metSoftClus,
      columnar::OptMet1Id coreSoftClus,
      columnar::OptMutableMetId metSoftTrk,
      columnar::OptMet1Id coreSoftTrk,
      bool doJetJVT,
      bool tracksForHardJets = false,
      std::vector<const xAOD::IParticle*>* softConst = 0) const;

    virtual StatusCode rebuildTrackMET(
      const std::string& metJetKey,
      const std::string& softTrkKey,
      xAOD::MissingETContainer* metCont,
      const xAOD::JetContainer* jets,
      const xAOD::MissingETContainer* metCoreCont,
      xAOD::MissingETAssociationHelper& helper,
      bool doJetJVT) const override final;
    StatusCode rebuildTrackMET(
      const std::string& metJetKey,
      const std::string& softTrkKey,
      columnar::MutableMetRange metCont,
      columnar::JetRange jets,
      columnar::Met1Range metCoreCont,
      columnar::MetAssociationHelper<> helper,
      bool doJetJVT) const;

    virtual StatusCode rebuildTrackMET(xAOD::MissingET* metJet,
                                       const xAOD::JetContainer* jets,
                                       xAOD::MissingETAssociationHelper& helper,
                                       xAOD::MissingET* metSoftTrk,
                                       const xAOD::MissingET* coreSoftTrk,
                                       bool doJetJVT) const override final;
    StatusCode rebuildTrackMET(columnar::MutableMetId metJet,
                                       columnar::MutableMetRange metCont,
                                       columnar::JetRange jets,
                                       columnar::MetAssociationHelper<> helper,
                                       columnar::MutableMetId metSoftTrk,
                                       columnar::Met1Id coreSoftTrk,
                                       bool doJetJVT) const;

    virtual StatusCode markInvisible(
      const xAOD::IParticleContainer* collection,
      xAOD::MissingETAssociationHelper& helper,
      xAOD::MissingETContainer* metCont) const override final;
    StatusCode markInvisible(
      columnar::ParticleRange collection,
      columnar::MetAssociationHelper<> helper,
      columnar::MutableMetRange metCont) const;

    ///////////////////////////////////////////////////////////////////
    // Private data:
    ///////////////////////////////////////////////////////////////////
  private:

    bool acceptTrack(const xAOD::TrackParticle* trk, const xAOD::Vertex* vx) const;
    const xAOD::Vertex* getPV() const;


    SG::ReadHandleKey<xAOD::VertexContainer>  m_PVkey;

    // pT threshold for suppressing warnings of objects missing in association map
    float m_missObjWarningPtThreshold;

    bool m_jetCorrectPhi{};
    double m_jetMinEfrac{};
    double m_jetMinWeightedPt{};
    std::string m_jetConstitScaleMom;
    std::string m_jetJvtMomentName;
    std::string m_jetRejectionDec;

    double m_CenJetPtCut{}, m_FwdJetPtCut{} ; // jet pt cut for central/forward jets
    double m_JvtCut{}, m_JvtPtMax{}; // JVT cut and pt region of jets to apply a JVT selection
    double m_JetEtaMax{};
    double m_JetEtaForw{};

    std::string m_jetSelection;
    std::string m_JvtWP;

    // Extra configurables for custom WP
    double m_customCenJetPtCut{},m_customFwdJetPtCut{};
    double m_customJvtPtMax{};
    std::string m_customJvtWP;

    bool m_doPFlow{};
    bool m_doSoftTruth{};
    bool m_doConstJet{};

    bool m_useGhostMuons{};
    bool m_doRemoveMuonJets{};
    bool m_doRemoveElecTrks{};
    bool m_doRemoveElecTrksEM{};
    bool m_doSetMuonJetEMScale{};
    bool m_skipSystematicJetSelection{};

    bool m_muEloss{};
    bool m_orCaloTaggedMuon{};
    bool m_greedyPhotons{};
    bool m_veryGreedyPhotons{};

    // muon overlap variables
    int m_jetTrkNMuOlap{};
    double m_jetWidthMuOlap{};
    double m_jetPsEMuOlap{};
    double m_jetEmfMuOlap{};
    double m_jetTrkPtMuPt{};
    double m_muIDPTJetPtRatioMuOlap{};

    ToolHandle<InDet::IInDetTrackSelectionTool> m_trkseltool;
    ToolHandle<IAsgSelectionTool> m_JvtTool;

    SG::ReadHandleKey<xAOD::JetContainer> m_jetContainer{this, "JetContainer", "", "Name of input jet container (required if JVT decisions computed by internal tool)"};

    /// Default constructor:
    ColumnarMETMaker();

    columnar::MutableMetAccessor<columnar::ObjectColumn> m_outputMetHandle {*this, "OutputMET"};
    columnar::Met1Accessor<columnar::ObjectColumn> m_inputMetHandle {*this, "METCore", {.addMTDependency=true}};
    columnar::ColumnAccessor<columnar::ContainerId::metAssociation,columnar::ObjectColumn> m_metAssocHandle {*this, "MetAssoc", {.addMTDependency=true}};
    columnar::ParticleAccessor<columnar::ObjectColumn> m_particlesHandle {*this, "Particles"};
    columnar::JetAccessor<columnar::ObjectColumn> m_jetsHandle {*this, "Jets"};
    columnar::ElectronAccessor<columnar::ObjectColumn> m_electronsHandle {*this, "Electrons"};
    columnar::PhotonAccessor<columnar::ObjectColumn> m_photonsHandle {*this, "Photons"};
    columnar::MuonAccessor<columnar::ObjectColumn> m_muonsHandle {*this, "Muons"};

    columnar::MutableMetAccessor<std::string> m_outputMetNameAcc {*this, "name"};
    columnar::MetHelpers::MapLookupAccessor<columnar::ContainerId::mutableMet> m_outputMetMapAcc {*this};
    columnar::MetHelpers::MetMomentumAccessors<columnar::ContainerId::mutableMet> m_outputMetMomAcc {*this};

    columnar::Met1Accessor<std::string> m_inputMetNameAcc {*this, "name"};
    columnar::MetHelpers::MapLookupAccessor<columnar::ContainerId::met1> m_inputMetMapAcc {*this};
    columnar::MetHelpers::MetMomentumAccessors<columnar::ContainerId::met1> m_inputMetMomAcc {*this};
    columnar::Met1Accessor<MissingETBase::Types::bitmask_t> m_inputMetSourceAcc {*this, "source"};

    columnar::MetAssocationAccessors<> m_assocAcc {*this};

    columnar::MetHelpers::InputMomentumAccessors<> m_inputMomAcc {*this};
    Gaudi::Property<std::string> m_inputPreselectionName {this, "inputPreselection", ""};
    std::optional<columnar::ParticleAccessor<char>> m_inputPreselectionAcc;
    columnar::ParticleAccessor<columnar::RetypeColumn<xAOD::Muon::MuonType,std::uint16_t>> m_inputMuonTypeAcc {*this, "muonType", {.isOptional = true}};
    columnar::MetHelpers::ObjectTypeAccessor<columnar::ContainerId::particle> m_inputObjTypeAcc {*this, "objectType"};

    columnar::MetHelpers::ObjectWeightDecorator<> m_outputMetWeightDecRegular {*this, "", true};

    columnar::MetHelpers::InputMomentumAccessors<columnar::ContainerId::jet> m_jetMomAcc {*this};
    columnar::JetAccessor<float> m_acc_emf {*this, "EMFrac"};
    columnar::JetAccessor<float> m_acc_psf {*this, "PSFrac"};
    columnar::JetAccessor<float> m_acc_width {*this, "Width"};
    columnar::JetAccessor<std::vector<int>> m_acc_trkN {*this, "NumTrkPt500"};
    columnar::JetAccessor<std::vector<float>> m_acc_trksumpt {*this, "SumPtTrkPt500"};
    columnar::JetAccessor<std::vector<float>> m_acc_sampleE {*this, "EnergyPerSampling"};
        
    std::optional<columnar::MetHelpers::InputMomentumAccessors<columnar::ContainerId::jet>> m_jetConstitScaleMomAcc;
    std::optional<columnar::MetHelpers::InputMomentumAccessors<columnar::ContainerId::jet>> m_jetConstitScaleMomFixedAcc;
    std::optional<columnar::JetAccessor<char>> m_acc_jetRejectionDec;

    columnar::MetHelpers::ObjectWeightDecorator<columnar::ContainerId::mutableMet,columnar::ContainerId::jet> m_jetOutputMetWeightDecRegular {*this, "", true};
    columnar::MetHelpers::ObjectWeightDecorator<columnar::ContainerId::mutableMet,columnar::ContainerId::jet> m_jetOutputMetWeightDecSoft {*this, "Soft", false};

    columnar::ElectronAccessor<columnar::RetypeColumn<double,float>> m_electronPtAcc {*this, "pt"};

    Gaudi::Property<unsigned> m_columnarOperation {this, "columnarOperation", 0};
    Gaudi::Property<std::string> m_columnarTermName {this, "columnarTermName", ""};
    Gaudi::Property<unsigned> m_columnarParticleType {this, "columnarParticleType", 0};
    Gaudi::Property<std::string> m_columnarJetKey {this, "columnarJetKey", ""};
    Gaudi::Property<std::string> m_columnarSoftClusKey {this, "columnarSoftClusKey", ""};
    Gaudi::Property<bool> m_columnarDoJetJVT {this, "columnarDoJetJVT", false};
    void callEvents (columnar::EventContextRange events) const override;
  };

} //> end namespace met
#endif //> !METUTILITIES_MET_METMAKER_H
