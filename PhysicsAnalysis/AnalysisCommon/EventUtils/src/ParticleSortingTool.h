///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ParticleSortingTool.h
// Header file for class ParticleSortingTool
// Author: Karsten Koeneke <karsten.koeneke@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef EVENTUTILS_PARTICLESORTINGTOOL_H
#define EVENTUTILS_PARTICLESORTINGTOOL_H 1

// FrameWork includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "CxxUtils/fpcompare.h"

// EDM inlcudes
#include "xAODBase/IParticle.h"
#include "xAODBase/IParticleContainer.h"
#include "AthContainers/ConstDataVector.h"

// STL includes
#include <vector>
#include <string>
#include <cmath>

class ParticleSortingTool
  : virtual public ::DerivationFramework::IAugmentationTool,
            public ::AthAlgTool
{

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:

  // Copy constructor:

  /// Constructor with parameters:
  ParticleSortingTool( const std::string& type,
                       const std::string& name,
                       const IInterface* parent );

  /// Destructor:
  virtual ~ParticleSortingTool();

  /// Athena algtool's initialize
  virtual StatusCode  initialize() override;

  /// Athena algtool's finalize
  virtual StatusCode  finalize() override;


  /// Implement the method from the ISkimmingTool interface
  virtual StatusCode addBranches() const final override;



// Private methods
private:

  /// Helper method that implements the call to the right sort function
  StatusCode doSort( xAOD::IParticleContainer* cont ) const;

  /// Helper method to sort a ConstDataVector
  template<class CONTAINERTYPE>
  StatusCode doSortConst( ConstDataVector<CONTAINERTYPE>* cont ) const;

  /// The method to compare the particle's pt
  bool comparePt( const xAOD::IParticle* partA, const xAOD::IParticle* partB ) const;

  /// The method to compare the particle's eta
  bool compareEta( const xAOD::IParticle* partA, const xAOD::IParticle* partB ) const;

  /// The method to compare the particle's phi
  bool comparePhi( const xAOD::IParticle* partA, const xAOD::IParticle* partB ) const;

  /// The method to compare the particle's mass
  bool compareMass( const xAOD::IParticle* partA, const xAOD::IParticle* partB ) const;

  /// The method to compare the particle's energy
  bool compareEnergy( const xAOD::IParticle* partA, const xAOD::IParticle* partB ) const;

  /// The method to compare the particle's rapidity
  bool compareRapidity( const xAOD::IParticle* partA, const xAOD::IParticle* partB ) const;

  /// The method to compare an auxdata member of the particle
  bool compareAuxData( const xAOD::IParticle* partA, const xAOD::IParticle* partB ) const;

  /// Method to compare two doubles
  inline bool compareDouble( double a, double b ) const;


  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
private:

  /// Input container name
  StringProperty m_inCollKey{ this, "InputContainer", "", "Input container name" };

  /// The name of the output container (with SG::VIEW_ELEMENTS) with the sorted copy of input objects
  StringProperty m_outCollKey{ this, "OutputContainer", "", "The name of the output container (with SG::VIEW_ELEMENTS) with the sorted copy of input objects" };

  /// Define by what parameter to sort (default: 'pt')
  StringProperty m_sortVar{ this, "SortVariable", "pt", "Define by what parameter to sort (default: 'pt'; allowed: 'pt', 'eta', 'phi', 'm', 'e', 'rapidity')" };

  /// Define if the container should be sorted in a descending order (default=true)
  BooleanProperty m_sortDescending{ this, "SortDescending", true, "Define if the container should be sorted in a descending order (default=true)" };

  /// Internal identifier for the type of sorting
  int m_sortID{0};

  /// Internal event counter
  mutable std::atomic<unsigned long> m_nEventsProcessed{0};

};


inline bool ParticleSortingTool::compareDouble( double a, double b ) const
{
  if ( m_sortID < 0 ) { return CxxUtils::fpcompare::greater(a,b); }
  else { return CxxUtils::fpcompare::less(a,b); }
}


template<class CONTAINERTYPE>
StatusCode ParticleSortingTool::doSortConst( ConstDataVector<CONTAINERTYPE>* cont ) const
{
  if ( !cont ) {
    ATH_MSG_ERROR("No ConstDataVector to be sorted");
    return StatusCode::FAILURE;
  }
  // Actually do the sorting, using a C++11 lambda function construct
  // to be able to use the member function here
  if ( std::abs(m_sortID) == 1 ) {
    cont->sort( [this](const xAOD::IParticle* a, const xAOD::IParticle* b) {
                  return this->comparePt(a,b);
                } );
  }
  else if ( std::abs(m_sortID) == 2 ) {
    cont->sort( [this](const xAOD::IParticle* a, const xAOD::IParticle* b) {
                  return this->compareEta(a,b);
                } );
  }
  else if ( std::abs(m_sortID) == 3 ) {
    cont->sort( [this](const xAOD::IParticle* a, const xAOD::IParticle* b) {
                  return this->comparePhi(a,b);
                } );
  }
  else if ( std::abs(m_sortID) == 4 ) {
    cont->sort( [this](const xAOD::IParticle* a, const xAOD::IParticle* b) {
                  return this->compareMass(a,b);
                } );
  }
  else if ( std::abs(m_sortID) == 5 ) {
    cont->sort( [this](const xAOD::IParticle* a, const xAOD::IParticle* b) {
                  return this->compareEnergy(a,b);
                } );
  }
  else if ( std::abs(m_sortID) == 6 ) {
    cont->sort( [this](const xAOD::IParticle* a, const xAOD::IParticle* b) {
                  return this->compareRapidity(a,b);
                } );
  }
  else if ( std::abs(m_sortID) == 7 ) {
    cont->sort( [this](const xAOD::IParticle* a, const xAOD::IParticle* b) {
                  return this->compareAuxData(a,b);
                } );
  }

  return StatusCode::SUCCESS;
}



#endif //> !EVENTUTILS_PARTICLESORTINGTOOL_H
