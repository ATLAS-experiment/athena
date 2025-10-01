/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  SystObjectUnioniserAlg:
  This alg takes a set of systematic variation containers which have
  been filtered, then using the links set by SystObjectLinkerAlg
  constructs new containers holding the objects that are in any
  of the filtered variation containers.

  This is templated to support recording the union containers as
  the concrete type, not least because xAOD::IParticleContainer has
  no CollectionProxy to support writing with AsgxAODNTupleMakerAlg.

*/

/// @author Teng Jian Khoo


// Always protect against multiple includes!
#ifndef CP_SYSTUNIONISERALG
#define CP_SYSTUNIONISERALG

#include <vector>
#include <unordered_map>
#include <utility>
#include <memory>

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include "PATInterfaces/SystematicSet.h"
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/ISysHandleBase.h>
#ifdef XAOD_STANDALONE
#include <AsgTools/PropertyWrapper.h>
#endif

#include <AthContainers/DataVector.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODBase/IParticleContainer.h>
#include <xAODJet/Jet.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/Electron.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/Photon.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODMuon/Muon.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODTau/TauJet.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODTau/DiTauJet.h>
#include <xAODTau/DiTauJetContainer.h>

class EventContext;

namespace CP
{
  typedef ElementLink<xAOD::IParticleContainer> iplink_t;

  // We need separate template parameters for the object type
  // and container, as DataVector<xAOD::Blah_v1> does not have
  // a classID, where BlahContainer does.
  template<class T, class C>
  class SystObjectUnioniserAlg : public EL::AnaReentrantAlgorithm
  {
    /// \brief The standard constructor
public:
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute(const EventContext& ctx) const override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:

    // Members for configurable properties

    /// \brief Setup list of systematics on which to iterate
    CP::SysListHandle m_systematicsList {this};

    /// \brief Setup syst-aware input container handles
    CP::SysReadHandle<C>
      m_inputHandle{ this, "input", "",   "Container to read" };

    /// \brief Setup syst-aware output container handles, templated to generate appropriate type
    CP::SysWriteHandle<ConstDataVector<C> >
      m_outputHandle{this, "output", "",  "Collection holding all objects passing any systematic"};

    /// \brief The systematics link decoration name template
    Gaudi::Property<std::string> m_syst_decor_pattern
      { this, "systVarLink_template", "systVarLink_%SYS%", "Pattern to be substituted with systematics names"};

    /// \brief Build a map of syst hash to ConstAccessors to read the systematics links
    std::unordered_map<size_t,SG::ConstAccessor<iplink_t> >
      m_syst_link_acc;
  };

  // Define the following classes concretely for source code configuration.
  template class SystObjectUnioniserAlg<xAOD::Jet,xAOD::JetContainer>;
  template class SystObjectUnioniserAlg<xAOD::Electron,xAOD::ElectronContainer>;
  template class SystObjectUnioniserAlg<xAOD::Photon,xAOD::PhotonContainer>;
  template class SystObjectUnioniserAlg<xAOD::Muon,xAOD::MuonContainer>;
  template class SystObjectUnioniserAlg<xAOD::TauJet,xAOD::TauJetContainer>;
  template class SystObjectUnioniserAlg<xAOD::DiTauJet,xAOD::DiTauJetContainer>;

  // Trivial subclassing for more convenient python configuration
  // Otherwise genConf creates a horribly mangled templated name
  // even if typedef'ed
  class SystJetUnioniserAlg final : public SystObjectUnioniserAlg<xAOD::Jet,xAOD::JetContainer>
  {
  public:
    SystJetUnioniserAlg(const std::string &name,ISvcLocator *pSvcLocator)
      : SystObjectUnioniserAlg<xAOD::Jet,xAOD::JetContainer>(name, pSvcLocator) {};
  };

  class SystElectronUnioniserAlg final : public SystObjectUnioniserAlg<xAOD::Electron,xAOD::ElectronContainer>
  {
  public:
    SystElectronUnioniserAlg(const std::string &name,ISvcLocator *pSvcLocator)
      : SystObjectUnioniserAlg<xAOD::Electron,xAOD::ElectronContainer>(name, pSvcLocator) {};
  };

  class SystPhotonUnioniserAlg final : public SystObjectUnioniserAlg<xAOD::Photon,xAOD::PhotonContainer>
  {
  public:
    SystPhotonUnioniserAlg(const std::string &name,ISvcLocator *pSvcLocator)
      : SystObjectUnioniserAlg<xAOD::Photon,xAOD::PhotonContainer>(name, pSvcLocator) {};
  };

  class SystMuonUnioniserAlg final : public SystObjectUnioniserAlg<xAOD::Muon,xAOD::MuonContainer>
  {
  public:
    SystMuonUnioniserAlg(const std::string &name,ISvcLocator *pSvcLocator)
      : SystObjectUnioniserAlg<xAOD::Muon,xAOD::MuonContainer>(name, pSvcLocator) {};
  };

  class SystTauUnioniserAlg final : public SystObjectUnioniserAlg<xAOD::TauJet,xAOD::TauJetContainer>
  {
  public:
    SystTauUnioniserAlg(const std::string &name,ISvcLocator *pSvcLocator)
      : SystObjectUnioniserAlg<xAOD::TauJet,xAOD::TauJetContainer>(name, pSvcLocator) {};
  };

  class SystDiTauUnioniserAlg final : public SystObjectUnioniserAlg<xAOD::DiTauJet,xAOD::DiTauJetContainer>
  {
  public:
    SystDiTauUnioniserAlg(const std::string &name,ISvcLocator *pSvcLocator)
      : SystObjectUnioniserAlg<xAOD::DiTauJet,xAOD::DiTauJetContainer>(name, pSvcLocator) {};
  };

}

#include "AsgAnalysisAlgorithms/SystObjectUnioniserAlg.icc"

#endif
