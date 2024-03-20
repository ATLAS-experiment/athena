/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef MET_ANALYSIS_ALGORITHMS__MET_MAKER_ALG_H
#define MET_ANALYSIS_ALGORITHMS__MET_MAKER_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <METInterface/IMETMaker.h>
#include <METInterface/IMETSystematicsTool.h>
#include <xAODBase/IParticleContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODMissingET/MissingETAuxContainer.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IMETMaker
  ///
  /// This algorithm is fairly complex for a common CP algorithm.  The
  /// main issue here is that the MET tools store temporary
  /// information on the xAOD objects that gets reset on each
  /// systematic, so a lot of actions have to happen in one go.
  /// Despite that complexity it still can't be run on its own, you
  /// always have to call \ref MetBuilderAlg afterwards to build the
  /// final MET.

  class MetMakerAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;
    


    /// \brief the maker tool
  private:
    ToolHandle<IMETMaker> m_makerTool {this, "makerTool", "METMaker", "the METMaker tool we apply"};

    /// \brief the systematics tool
  private:
    ToolHandle<IMETSystematicsTool> m_systematicsTool {this, "systematicsTool", "", "the systematics tool we apply"};

    /// \brief the name of the core MissingETContainer
  private:
    Gaudi::Property<std::string> m_metCoreName {this, "metCore", "",  "the name of the core MissingETContainer"};

    /// \brief the name of the MissingETAssociationMap
  private:
    Gaudi::Property<std::string> m_metAssociationName {this, "metAssociation", "", "the name of the core MissingETContainer"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the electron container to use
  private:
    SysReadHandle<xAOD::IParticleContainer> m_electronsHandle {
      this, "electrons", "", "the electron container to use" };

    /// \brief the selection on the input electrons
  private:
    SysReadSelectionHandle m_electronsSelection {
      this, "electronsSelection", "", "the selection on the input electrons" };

    /// \brief the key for \ref m_electronsHandle
  private:
    Gaudi::Property<std::string> m_electronsKey {this, "electronsKey", "RefEle", "the key for the electrons"};

    /// \brief the photon container to use
  private:
    SysReadHandle<xAOD::IParticleContainer> m_photonsHandle {
      this, "photons", "", "the photon container to use" };

    /// \brief the selection on the input photons
  private:
    SysReadSelectionHandle m_photonsSelection {
      this, "photonsSelection", "", "the selection on the input photons" };

    /// \brief the key for \ref m_photonsHandle
  private:
    Gaudi::Property<std::string> m_photonsKey {this, "photonsKey", "RefGamma", "the key for the photons"};

    /// \brief the muon container to use
  private:
    SysReadHandle<xAOD::IParticleContainer> m_muonsHandle {
      this, "muons", "", "the muon container to use" };

    /// \brief the selection on the input muons
  private:
    SysReadSelectionHandle m_muonsSelection {
      this, "muonsSelection", "", "the selection on the input muons" };

    /// \brief the key for \ref m_muonsHandle
  private:
    Gaudi::Property<std::string> m_muonsKey {this, "muonsKey", "Muons", "the key for the muons"};

    /// \brief the electron container to use
  private:
    SysReadHandle<xAOD::IParticleContainer> m_tausHandle {
      this, "taus", "", "the tau container to use" };

    /// \brief the selection on the input taus
  private:
    SysReadSelectionHandle m_tausSelection {
      this, "tausSelection", "", "the selection on the input taus" };

    /// \brief the key for \ref m_tausHandle
  private:
    Gaudi::Property<std::string> m_tausKey {this, "tausKey", "RefTau", "the key for the taus"};

    /// \brief the input jet collection we run on
  private:
    SysReadHandle<xAOD::JetContainer> m_jetsHandle {
      this, "jets", "", "the jet collection we use"};

  private:
    SysReadHandle<xAOD::IParticleContainer> m_invisHandle {
      this, "invisible", "", "Any particles to treat as invisible."};

    /// \brief the key for \ref m_jetsHandle
  private:
    Gaudi::Property<std::string> m_jetsKey {this, "jetsKey", "RefJet", "the key for the jets"};

    /// \brief the soft term key
  private:
    Gaudi::Property<std::string> m_softTermKey {this, "softTermKey", "PVSoftTrk", "the soft term key"};

    /// \brief whether to use track-met instead of jet-met
  private:
    Gaudi::Property<bool> m_doTrackMet {this, "doTrackMet", false, "whether to use track-met instead of jet-met"};

    /// \brief whether to do jet JVT
  private:
    Gaudi::Property<bool> m_doJetJVT {this, "doJetJVT", true, "whether to do jet JVT"};

    /// \brief the met collection we run on
  private:
    SysWriteHandle<xAOD::MissingETContainer,xAOD::MissingETAuxContainer> m_metHandle {
      this, "met", "MissingET_%SYS%", "the met collection we produce"};
  };
}

#endif
