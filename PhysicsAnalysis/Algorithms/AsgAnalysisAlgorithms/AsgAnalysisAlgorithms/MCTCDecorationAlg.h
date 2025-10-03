/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#ifndef ASG_ANALYSIS_ALGORITHMS__MCTC_DECORATION_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__MCTC_DECORATION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODBase/IParticleContainer.h>



namespace CP
{
  /// \brief an algorithm to unpack and decorate MCTruthClassifier
  /// Classification information

  class MCTCDecorationAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    MCTCDecorationAlg (const std::string& name, 
                       ISvcLocator* pSvcLocator);


  public:
    virtual StatusCode initialize () override;

  public:
    virtual StatusCode execute () override;
    


    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the particle collection we run on
  private:
    SysReadHandle<xAOD::IParticleContainer> m_particlesHandle {
      this, "particles", "", "the particle collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the decoration for the MCTC classification bitmask
  private:
    Gaudi::Property<std::string> m_classificationDecoration{this, "classificationDecoration", "Classification", "the decoration for the MCTC classification bitmask"};

    /// \brief the accessor for \ref m_classificationDecoration
  private:
    std::unique_ptr<const SG::ConstAccessor<unsigned int> > m_classificationAccessor{};

    /// \brief the decoration for the promptness
  private:
    Gaudi::Property<std::string> m_isPromptDecoration{this, "isPromptDecoration", "MCTC_isPrompt", "the decoration for the promptness information"};

    /// \brief the decorator for \ref m_isPromptDecoration
  private:
    std::unique_ptr<const SG::Decorator<int> > m_isPromptDecorator{};

    /// \brief the decoration for the hadronic origin
  private:
    Gaudi::Property<std::string> m_fromHadronDecoration{this, "fromHadronDecoration", "MCTC_fromHadron", "the decoration for the hadronic origin"};

    /// \brief the decorator for \ref m_fromHadDecoration
  private:
    std::unique_ptr<const SG::Decorator<int> > m_fromHadronDecorator{};

    /// \brief the decoration for the BSM origin
  private:
    Gaudi::Property<std::string> m_fromBSMDecoration{this, "fromBSMDecoration", "MCTC_fromBSM", "the decoration for the BSM origin"};

    /// \brief the decorator for \ref m_fromBSMDecoration
  private:
    std::unique_ptr<const SG::Decorator<int> > m_fromBSMDecorator{};

    /// \brief the decoration for the tau origin
  private:
    Gaudi::Property<std::string> m_fromTauDecoration{this, "fromTauDecoration", "MCTC_fromTau", "the decoration for the tau origin"};

    /// \brief the decorator for \ref m_fromTauDecoration
  private:
    std::unique_ptr<const SG::Decorator<int> > m_fromTauDecorator{};
  };

} // namespace CP

#endif
