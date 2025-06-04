///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ParticleSelectionAlg.h
// Header file for class ParticleSelectionAlg
// Author: Karsten Koeneke <karsten.koeneke@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef EVENTUTILS_PARTICLESELECTIONALG_H
#define EVENTUTILS_PARTICLESELECTIONALG_H 1

// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
// #include "GaudiKernel/ServiceHandle.h"
// #include "AthenaBaseComps/AthAlgorithm.h"
#include "AthAnalysisBaseComps/AthAnalysisAlgorithm.h"
#include "xAODBase/IParticleContainer.h"
#include "PATCore/IAsgSelectionTool.h"

// STL includes
#include <string>
#include <vector>

#include "ExpressionEvaluation/ExpressionParserUser.h"


class ParticleSelectionAlg
  : public ExpressionParserUser<::AthAnalysisAlgorithm>
{

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
 public:

  // Copy constructor:

  /// Constructor with parameters:
  ParticleSelectionAlg( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor:
  virtual ~ParticleSelectionAlg();


  /// Athena algorithm's initalize hook
  virtual StatusCode  initialize() override;

  /// Athena algorithm's beginRun hook
  /// (called once before running over the events, after initialize)
  virtual StatusCode  start() override;

  /// Athena algorithm's execute hook
  virtual StatusCode  execute() override;

  /// Athena algorithm's finalize hook
  virtual StatusCode  finalize() override;

 private:
  /// Private function to perform the actualy work
  template<class CONT, class AUXCONT>
  StatusCode selectParticles(const xAOD::IParticleContainer* inContainer,
                             const std::vector<int>& resultVec) const;


  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
 private:
  /// The list of IAsgSelectionTools
  ToolHandleArray<IAsgSelectionTool> m_selTools;

  /// Name of the EventInfo object
  Gaudi::Property<std::string> m_evtInfoName{this, "EventInfo", "EventInfo", "Input container name"};

  /// Input container name
  Gaudi::Property<std::string> m_inCollKey{this, "InputContainer", "", "Input container name"};

  /// Output collection name (deep copies of the original ones)
  Gaudi::Property<std::string> m_outCollKey{this, "OutputContainer", "", "The name of the output container with the deep copy of selected xAOD::IParticles"};

  /// Decide if we want to write a fully-split AuxContainer such that we can remove any variables
  Gaudi::Property<bool> m_writeSplitAux{this, "WriteSplitOutputContainer", true, "Decide if we want to write a fully-split AuxContainer such that we can remove any variables"};

  /// Defines the ownership policy of the output container
  Gaudi::Property<std::string> m_outOwnPolicyName{this, "OutputContainerOwnershipPolicy", "VIEW_ELEMENTS", "Defines the ownership policy of the output container"};

  /// The selection string that will select which xAOD::IParticles to keep from
  /// an xAOD::IParticleContainer
  Gaudi::Property<std::string> m_selection{this, "Selection", "", "The selection string that defines which xAOD::IParticles to select from the container"};

  /// If true (default: false), do the bookkeeping of how many particles passed
  /// which selection cuts
  Gaudi::Property<bool> m_doCutFlow{this, "DoCutBookkeeping", false, "If true, do the bookkeeping of how many particles passed which selection cuts"};

  /// The name of the resulting xAOD::CutBookkeeperContainer.
  /// If an empty name is given (default), the name of the algorithm instance is used.
  Gaudi::Property<std::string> m_cutBKCName{this, "CutBookkeeperContainer", name(), "The name of the resulting xAOD::CutBookkeeperContainer"};



  /// @name Internal members
  /// @{

  /// Internal event counter
  unsigned long m_nEventsProcessed{0};

  /// The internally used translation for the ownership policy
  SG::OwnershipPolicy m_outOwnPolicy{SG::VIEW_ELEMENTS};

  /// An enumaration for the actual container type
  enum contType_t {
    UNKNOWN,
    PHOTON,
    ELECTRON,
    MUON,
    TAU,
    JET,
    PARITCLEFLOW,
    NEUTRALPARTICLE,
    TRACKPARTICLE,
    TRUTHPARTICLE,
    COMPOSITEPARTICLE,
    PARTICLE,
    CALOCLUSTER
  };

  /// The variable that holds the value that we find for the input container
  contType_t m_contType{UNKNOWN};

  /// The starting index of where in the CutBookkeeperContainer our new CutBookkeepers start
  std::size_t m_cutBKStartIdx{0};

  /// The list of pairs of the tool index of the AsgSelectionTools and the
  /// starting index of the corresponding CutBookKeeper inside the CutBookkeeperContainer.
  std::vector<std::size_t> m_selToolIdxOffset;

  /// Store the index of the CutBookKeeper in the CutBookkeeperContainer for the
  /// selection using the ExpressionParser
  std::size_t m_idxSelParster{0};

  /// @}

};

// Include the templated code here. This must be done from this header file.
#include "ParticleSelectionAlg.icc"


#endif //> !EVENTUTILS_PARTICLESELECTIONALG_H
