/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ExtrapolatorTest.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXALGS_TRKEXTRAPOLATORTEST_H
#define TRKEXALGS_TRKEXTRAPOLATORTEST_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/SystemOfUnits.h"
#include <string>
#include <vector>

#include "TrkParameters/TrackParameters.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkExInterfaces/IPropagator.h"

namespace Trk 
{

  class Surface;
  class MagneticFieldProperties;
  /** @class ExtrapolatorTest

     The ExtrapolatorTest Algorithm runs a number of n test extrapolations
     from randomly distributed Track Parameters to reference surfcas within
    
     - a) the Inner Detector if DetFlags.ID_On()
     - b) the Calorimeter if DetFlags.Calo_On()
     - c) the Muon System if DetFlags.Muon_On()

     It is the TestAlgorithm for 'the' Extrapolator instance provided 
     to client algorithms.

      @author  Andreas Salzburger <Andreas.Salzburger@cern.ch>
  */  

  class ExtrapolatorTest : public AthAlgorithm
    {
    public:

       /** Standard Athena-Algorithm Constructor */
       ExtrapolatorTest(const std::string& name, ISvcLocator* pSvcLocator);
       /** Default Destructor */
       ~ExtrapolatorTest();

       /** standard Athena-Algorithm method */
       StatusCode          initialize();
       /** standard Athena-Algorithm method */
       StatusCode          execute();
       /** standard Athena-Algorithm method */
       StatusCode          finalize();

    private:

      void runTest( const Trk::Perigee& perigee );

      Trk::Perigee generatePerigee();
      
      /** The Extrapolator to be retrieved */
      ToolHandle<IExtrapolator> m_extrapolator
	{this, "Extrapolator", "Trk::Extrapolator/AtlasExtrapolator"};
      PublicToolHandle<IPropagator> m_propagator
	{this, "Propagator", "Trk::RungeKuttaPropagator/RungeKuttaPropagator"};
      MagneticFieldProperties*  m_magFieldProperties = nullptr; //!< magnetic field properties

      /** Random Number setup */
      Rndm::Numbers* m_gaussDist = nullptr;
      Rndm::Numbers* m_flatDist = nullptr;

      DoubleProperty m_sigmaD0
	{this, "StartPerigeeSigmaD0", 17.*Gaudi::Units::micrometer};
      DoubleProperty m_sigmaZ0
	{this, "StartPerigeeSigmaZ0", 50.*Gaudi::Units::micrometer};
      DoubleProperty m_minPhi{this, "StartPerigeeMinPhi", -M_PI};
      DoubleProperty m_maxPhi{this, "StartPerigeeMaxPhi", M_PI};
      DoubleProperty m_minEta{this, "StartPerigeeMinEta", -3.};
      DoubleProperty m_maxEta{this, "StartPerigeeMaxEta", 3.};
      DoubleProperty m_minP
	{this, "StartPerigeeMinP", 0.5*Gaudi::Units::GeV};
      DoubleProperty m_maxP
	{this, "StartPerigeeMaxP", 50000*Gaudi::Units::GeV};

      IntegerProperty m_direction{this, "StartDirection", 1};
      IntegerProperty m_particleType
	{this, "ParticleType", 2, "the particle type for the extrap."};

      /** member variables for algorithm properties: */
      unsigned int m_referenceSurfaces = 0;

      DoubleArrayProperty m_referenceSurfaceRadius
	{this, "ReferenceSurfaceRadius", {}};
      DoubleArrayProperty m_referenceSurfaceHalflength
	{this, "ReferenceSurfaceHalfZ", {}};
      std::vector< std::vector<const Surface*> >       m_referenceSurfaceTriples;

      std::vector<double>                              m_referenceSurfaceNegativeBoundary;
      std::vector<double>                              m_referenceSurfacePositiveBoundary;

      std::vector<Trk::Perigee> m_perigees;

      IntegerProperty m_eventsPerExecute{this, "EventsPerExecute", -1};
      BooleanProperty m_useExtrapolator{this, "UseExtrapolator", false};
    }; 
} // end of namespace

#endif
