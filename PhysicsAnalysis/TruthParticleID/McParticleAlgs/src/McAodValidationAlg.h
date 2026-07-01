///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// McAodValidationAlg.h 
// Header file for class McAodValidationAlg
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef MCPARTICLEALGS_MCAODVALIDATIONALG_H 
#define MCPARTICLEALGS_MCAODVALIDATIONALG_H 

// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h" //for ToolHandleArray

// STL includes
#include <string>

// Forward declaration
class ITruthParticleValidationTool;

class McAodValidationAlg : public AthAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /// Constructor with parameters: 
  McAodValidationAlg( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~McAodValidationAlg(); 


  // Athena algorithm's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) override;
  virtual StatusCode  finalize() override;

  /////////////////////////////////////////////////////////////////// 
  // Protected methods: 
  /////////////////////////////////////////////////////////////////// 
 protected: 

  /// Default constructor: 
  McAodValidationAlg();

  /////////////////////////////////////////////////////////////////// 
  // Protected data: 
  /////////////////////////////////////////////////////////////////// 
 protected: 

  typedef ToolHandleArray<ITruthParticleValidationTool> IValidationTools_t;
  /** Validation tools to be ran on the events
   */
  IValidationTools_t m_valTools;

  // Containers
  
  /// Location of the TruthParticleContainer to read
  StringProperty m_truthParticlesName;
  
  // switches
  
  /// Random generator seed
  unsigned int m_seed;
  
}; 

#endif //> MCPARTICLEALGS_MCAODVALIDATIONALG_H
