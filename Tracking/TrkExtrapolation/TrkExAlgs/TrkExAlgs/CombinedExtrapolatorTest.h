/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CombinedExtrapolatorTest.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXALGS_COMBINEDEXTRAPOLATORTEST_H
#define TRKEXALGS_COMBINEDEXTRAPOLATORTEST_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/SystemOfUnits.h"
#include <string>

#include "TrkExInterfaces/IExtrapolator.h"


namespace Trk 
{
  class Surface;
  class TrackingVolume;
  class TrackingGeometry;

  /** @class CombinedExtrapolatorTest

     The ExtrapolatorTest Algorithm runs a number of n test extrapolations
     from randomly distributed Track Parameters to geometry outer boundary and back to perigee

      @author Sarka Todorova <sarka.todorova@cern.ch>
  */  

  class CombinedExtrapolatorTest : public AthAlgorithm
    {
    public:

       /** Standard Athena-Algorithm Constructor */
       CombinedExtrapolatorTest(const std::string& name, ISvcLocator* pSvcLocator);
       /** Default Destructor */
       ~CombinedExtrapolatorTest();

       /** standard Athena-Algorithm method */
       StatusCode          initialize();
       /** standard Athena-Algorithm method */
       StatusCode          execute();
       /** standard Athena-Algorithm method */
       StatusCode          finalize();

    private:
      /** The Extrapolator to be retrieved */
      ToolHandle<IExtrapolator> m_extrapolator
      {this, "Extrapolator", "Trk::Extrapolator/AtlasExtrapolator"};

      /** Random Number setup */
      Rndm::Numbers* m_gaussDist = nullptr;
      Rndm::Numbers* m_flatDist = nullptr;

      DoubleProperty m_sigmaD0{this, "StartPerigeeSigmaD0", 17.*Gaudi::Units::micrometer};
      DoubleProperty m_minZ0{this, "StartPerigeeMinZ0", -25000.};
      DoubleProperty m_maxZ0{this, "StartPerigeeMaxZ0", +25000.};
      DoubleProperty m_minP{this, "StartPerigeeMinP", 0.5*Gaudi::Units::GeV};
      DoubleProperty m_maxP{this, "StartPerigeeMaxP", 50000.*Gaudi::Units::GeV};

      const Trk::TrackingVolume*   m_outerBoundary = nullptr;
      const Trk::TrackingGeometry* m_trackingGeometry = nullptr;

      IntegerProperty m_particleType{this, "ParticleType", Trk::muon,
	"the particle type for the extrap."};

      
    }; 
} // end of namespace

#endif
