///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ParticleSortingAlg.h
// Header file for class ParticleSortingAlg
// Author: Karsten Koeneke <karsten.koeneke@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef EVENTUTILS_PARTICLESORTINGALG_H
#define EVENTUTILS_PARTICLESORTINGALG_H 1

// FrameWork includes
#include "Gaudi/Interfaces/IOptionsSvc.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"

// STL includes
#include <string>

// Forward declarations
namespace DerivationFramework {
  class IAugmentationTool;
}

class ParticleSortingAlg
  : public ::AthAlgorithm
{

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
 public:

  // Copy constructor:

  /// Constructor with parameters:
  ParticleSortingAlg( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor:
  virtual ~ParticleSortingAlg();

  /// Athena algorithm's initalize hook
  virtual StatusCode initialize() override;

  /// Athena algorithm's execute hook
  virtual StatusCode execute() override;

  /// Athena algorithm's finalize hook
  virtual StatusCode finalize() override;


private:
  // The update handlers

  /// This internal method will realize if a user sets the 'InputContainer' property
  void setupInputContainer( Gaudi::Details::PropertyBase& /*prop*/ );

  /// This internal method will realize if a user sets the 'OutputContainer' property
  void setupOutputContainer( Gaudi::Details::PropertyBase& /*prop*/ );

  /// This internal method will realize if a user sets the 'SortVariable' property
  void setupSortVar( Gaudi::Details::PropertyBase& /*prop*/ );

  /// This internal method will realize if a user sets the 'SortDeceding' property
  void setupSortDescending( Gaudi::Details::PropertyBase& /*prop*/ );



  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
 private:
  /// The job options service (will be used to forward this algs properties to
  /// the private tool)
  ServiceHandle<Gaudi::Interfaces::IOptionsSvc> m_jos{
    this, "JobOptionsSvc", "JobOptionsSvc", "The JobOptionService instance." };

  /// The ToolHandle to the private ParticleSortingTool
  ToolHandle<DerivationFramework::IAugmentationTool> m_tool{
    this, "SortingTool", "ParticleSortingTool/ParticleSortingTool", "The private ParticleSortingTool" };


  /// Input container name
  StringProperty m_inCollKey{this, "InputContainer", "", &ParticleSortingAlg::setupInputContainer,
    "Input container name" };

  /// This boolean is true if the user sets the 'InputContainer' property
  bool m_setInCollKey{false};

  /// The name of the output container (with SG::VIEW_ELEMENTS) with the sorted copy of input objects
  StringProperty m_outCollKey{this, "OutputContainer", "", &ParticleSortingAlg::setupOutputContainer,
    "The name of the output container (with SG::VIEW_ELEMENTS) with the sorted copy of input objects" };

  /// This boolean is true if the user sets the 'OutputContainer' property
  bool m_setOutCollKey{false};

  /// Define by what parameter to sort (default: 'pt')
  StringProperty m_sortVar{this, "SortVariable", "pt", &ParticleSortingAlg::setupSortVar,
    "Define by what parameter to sort (default: 'pt'; allowed: 'pt', 'eta', 'phi', 'm', 'e', 'rapidity')" };

  /// This boolean is true if the user sets the 'SortVariable' property
  bool m_setSortVar{false};

  /// Define if the container should be sorted in a descending order (default=true)
  BooleanProperty m_sortDescending{this, "SortDescending", true, &ParticleSortingAlg::setupSortDescending,
    "Define if the container should be sorted in a descending order (default=true)" };

  /// This boolean is true if the user sets the 'SortDescending' property
  bool m_setSortDescending{false};

  /// Internal event counter
  unsigned long m_nEventsProcessed{0};

};



///////////////////////////////////////////////////////////////////
// Inline methods:
///////////////////////////////////////////////////////////////////

/// This internal method will realize if a user sets the 'InputContainer' property
inline void ParticleSortingAlg::setupInputContainer( Gaudi::Details::PropertyBase& /*prop*/ ) {
  m_setInCollKey = true;
  return;
}

/// This internal method will realize if a user sets the 'OutputContainer' property
inline void ParticleSortingAlg::setupOutputContainer( Gaudi::Details::PropertyBase& /*prop*/ ) {
  m_setOutCollKey = true;
  return;
}

/// This internal method will realize if a user sets the 'SortVariable' property
inline void ParticleSortingAlg::setupSortVar( Gaudi::Details::PropertyBase& /*prop*/ )
{
  m_setSortVar = true;
  return;
}

/// This internal method will realize if a user sets the 'SortDeceding' property
inline void ParticleSortingAlg::setupSortDescending( Gaudi::Details::PropertyBase& /*prop*/ )
{
  m_setSortDescending = true;
  return;
}


#endif //> !EVENTUTILS_PARTICLESORTINGALG_H
