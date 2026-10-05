/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_ITAUTOOLBASE_H
#define TAURECTOOLS_ITAUTOOLBASE_H

#include "AsgTools/IAsgTool.h"
#include "xAODTau/TauJet.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODPFlow/PFOContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODParticleEvent/ParticleContainer.h"
#include <boost/dynamic_bitset.hpp>

#ifndef XAOD_ANALYSIS
#include "CaloEvent/CaloConstCellContainer.h"
#endif

/**
 * @brief The base class for all tau tools.
 * 
 * @author Lukasz Janyst
 * @author Justin Griffiths
 * Thanks to Lianyou Shan, Lorenz Hauswald
 */

class ITauToolBase : virtual public asg::IAsgTool
{
 public:

  ASG_TOOL_INTERFACE(ITauToolBase)    

  virtual ~ITauToolBase() {}

  //-----------------------------------------------------------------
  //! Tool initializer
  //-----------------------------------------------------------------
  virtual StatusCode initialize() = 0;

  //-----------------------------------------------------------------
  //! Event initializer - called at the beginning of each event
  //-----------------------------------------------------------------
  virtual StatusCode eventInitialize() = 0;

  //-----------------------------------------------------------------
  //! Execute - called for each tau candidate
  //-----------------------------------------------------------------
  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ) const = 0;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 const xAOD::VertexContainer* ) const = 0;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::VertexContainer& ) const = 0;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::TauTrackContainer& ) const = 0;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::CaloClusterContainer& ,
				 xAOD::PFOContainer& ) const = 0;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::PFOContainer& ,
				 xAOD::PFOContainer& ,
				 const xAOD::CaloClusterContainer& ) const = 0;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::PFOContainer& ,
				 xAOD::PFOContainer& ) const = 0;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::PFOContainer& ) const = 0;

  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 xAOD::ParticleContainer& ,
				 xAOD::PFOContainer& ) const = 0;

#ifdef XAOD_ANALYSIS
  // non-const version is needed in THOR
  virtual StatusCode executeDev(xAOD::TauJet& ) = 0;
#else
  // CaloCellContainer not available in AnalysisBase
  virtual StatusCode executeTool(xAOD::TauJet& ,
				 const EventContext& ,
				 CaloConstCellContainer& ,
				 boost::dynamic_bitset<>& ) const = 0;
#endif
  
  //-----------------------------------------------------------------
  //! Event finalizer - called at the end of each event
  //-----------------------------------------------------------------
  virtual StatusCode eventFinalize() = 0;

  //-----------------------------------------------------------------
  //! Finalizer
  //-----------------------------------------------------------------
  virtual StatusCode finalize() = 0;

};

#endif // TAURECTOOLS_ITAUTOOLBASE_H
