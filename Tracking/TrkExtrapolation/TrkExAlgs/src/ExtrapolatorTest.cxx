/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ExtrapolatorTest.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Tracking
#include "TrkExAlgs/ExtrapolatorTest.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "EventPrimitives/EventPrimitives.h"
#include "TrkGeometry/MagneticFieldProperties.h"
// std
#include <cmath>

//================ Constructor =================================================

Trk::ExtrapolatorTest::ExtrapolatorTest(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name,pSvcLocator) {}

//================ Destructor =================================================

Trk::ExtrapolatorTest::~ExtrapolatorTest()
{
  delete m_gaussDist;
  delete m_flatDist;
  // cleanup of the surfaces
  for (const auto& surfaceTriple : m_referenceSurfaceTriples) {
    for (const auto* surface : surfaceTriple) {
        delete surface;
    }
  }
}


//================ Initialisation =================================================

StatusCode Trk::ExtrapolatorTest::initialize()
{
  // Code entered here will be executed once at program start.
  
  ATH_MSG_INFO(" initialize()");

  ATH_CHECK( m_extrapolator.retrieve() );
  ATH_CHECK( m_propagator.retrieve() );

  m_magFieldProperties = new Trk::MagneticFieldProperties();

  if (m_referenceSurfaceRadius.size() == m_referenceSurfaceHalflength.size()) {
     // assign the size
     m_referenceSurfaces = m_referenceSurfaceRadius.size();
     // loop over it and create the 
     std::vector<double>::iterator radiusIter     = m_referenceSurfaceRadius.begin();
     std::vector<double>::iterator radiusIterEnd  = m_referenceSurfaceRadius.end();
     std::vector<double>::iterator halfZIter      = m_referenceSurfaceHalflength.begin();

     for ( ; radiusIter != radiusIterEnd; ++radiusIter, ++halfZIter){
          // create the Surface triplet
          std::vector< const Trk::Surface*> surfaceTriplet;
             surfaceTriplet.push_back(new Trk::DiscSurface(Amg::Transform3D(Amg::Translation3D(0.,0.,*halfZIter)),0.,*radiusIter));
             surfaceTriplet.push_back(new Trk::CylinderSurface(Amg::Transform3D(),*radiusIter, *halfZIter));
             surfaceTriplet.push_back(new Trk::DiscSurface(Amg::Transform3D(Amg::Translation3D(0.,0.,-(*halfZIter))),0.,*radiusIter));
	     
	     ATH_MSG_INFO("Creating surfaces: R " << *radiusIter << " Z " << *halfZIter);
	     m_referenceSurfaceTriples.push_back(surfaceTriplet);

	     m_referenceSurfaceNegativeBoundary.push_back(atan2(*radiusIter,-(*halfZIter)));
	     m_referenceSurfacePositiveBoundary.push_back(atan2(*radiusIter,(*halfZIter)));

     }
  }

  m_gaussDist = new Rndm::Numbers(randSvc(), Rndm::Gauss(0.,1.));
  m_flatDist  = new Rndm::Numbers(randSvc(), Rndm::Flat(0.,1.));

  msg(MSG::INFO) << "initialize() successful in " << endmsg;

  if( m_eventsPerExecute > 0 ){
    m_perigees.reserve(m_eventsPerExecute);
    for( int i=0;i<m_eventsPerExecute;++i ) m_perigees.push_back(generatePerigee());
  }

  return StatusCode::SUCCESS;
}

//================ Finalisation =================================================

StatusCode Trk::ExtrapolatorTest::finalize()
{
  // Code entered here will be executed once at the end of the program run.
  return StatusCode::SUCCESS;
}

Trk::Perigee Trk::ExtrapolatorTest::generatePerigee() {
   // generate with random number generator
   double d0 = m_gaussDist->shoot() * m_sigmaD0;
   double z0 = m_gaussDist->shoot() * m_sigmaZ0;
   double phi = m_minPhi + (m_maxPhi-m_minPhi)* m_flatDist->shoot();
   double eta = m_minEta + m_flatDist->shoot()*(m_maxEta-m_minEta);
   double theta = 2.*atan(exp(-eta));
   double p = m_minP + m_flatDist->shoot()*(m_maxP-m_minP);
   double charge = (m_flatDist->shoot() > 0.5 ) ? -1. : 1.;
   double qOverP = charge/(p);
   const Trk::PerigeeSurface perSurface;
   return {d0, z0, phi, theta, qOverP,perSurface}; 
}

//================ Execution ====================================================

StatusCode Trk::ExtrapolatorTest::execute()
{
  
  if( m_eventsPerExecute <= 0 ) runTest(generatePerigee());
  else                          for( int i=0;i<m_eventsPerExecute;++i ) runTest(m_perigees[i]);
  return StatusCode::SUCCESS;
}


void Trk::ExtrapolatorTest::runTest( const Trk::Perigee& initialPerigee ) {
   Trk::PropDirection propagationDirection = m_direction > 0 ? Trk::alongMomentum : oppositeMomentum;
  
   ATH_MSG_VERBOSE("Starting from : "       << initialPerigee );
   ATH_MSG_VERBOSE(" ---> with direction: " << propagationDirection );



   std::vector< std::vector< const Trk::Surface* > >::const_iterator surfaceTripleIter    = m_referenceSurfaceTriples.begin();
   std::vector< std::vector< const Trk::Surface* > >::const_iterator surfaceTripleIterEnd = m_referenceSurfaceTriples.end();

   std::vector<double>::iterator negRefIter = m_referenceSurfaceNegativeBoundary.begin();
   std::vector<double>::iterator posRefIter = m_referenceSurfacePositiveBoundary.begin();

   double theta = initialPerigee.parameters()[Trk::theta];

   const EventContext& ctx = Gaudi::Hive::currentContext();
   for (int refSurface = 0 ; surfaceTripleIter != surfaceTripleIterEnd; ++surfaceTripleIter, ++negRefIter, ++posRefIter ){
       // decide which reference surface to take
       refSurface = theta < (*posRefIter) ? 2 : 1;
       refSurface = theta > (*negRefIter) ? 0 : 1;

       const Trk::Surface* destinationSurface = (*surfaceTripleIter)[refSurface];

       const auto* destParameters =
         m_useExtrapolator
           ? m_extrapolator->extrapolate(
               ctx,
               initialPerigee,
               *destinationSurface,
               propagationDirection,
               false,
               static_cast<Trk::ParticleHypothesis>(m_particleType.value())).release()
           :

           m_propagator
             ->propagate(ctx,
                         initialPerigee,
                         *destinationSurface,
                         propagationDirection,
                         false,
                         *m_magFieldProperties,
                         static_cast<Trk::ParticleHypothesis>(m_particleType.value()))
             .release();

       if (destParameters) {
	 // intersection output
	 ATH_MSG_VERBOSE(" [ intersection ] with surface at (x,y,z) = " <<
			 destParameters->position().x() << ", " <<
			 destParameters->position().y() << ", " <<
			 destParameters->position().z() );           
       } else if (!destParameters)
	 ATH_MSG_DEBUG(" Extrapolation not successful! " );

       delete destParameters;
   }
}

//============================================================================================

