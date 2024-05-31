/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "SimHitCreatorID.h"

// iFatras && ISF
#include "ISF_Event/ISFParticle.h"
// Tracking
#include "TrkDetElementBase/TrkDetElementBase.h"
#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackInfo.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
//CLHEP
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"
// Core
#include "StoreGate/StoreGateSvc.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "IdDictDetDescr/IdDictManager.h"
// STL
#include <iomanip>
#include <sstream>
#include <iostream>
#include <math.h>

/*=========================================================================
 *  DESCRIPTION OF FUNCTION:
 *  ==> see headerfile
 *=======================================================================*/
iFatras::SimHitCreatorID::SimHitCreatorID(const std::string& t, const std::string& n, const IInterface* p)
        : base_class(t,n,p)
{
}

/*=========================================================================
 *  DESCRIPTION OF FUNCTION:
 *  ==> see headerfile
 *=======================================================================*/
iFatras::SimHitCreatorID::~SimHitCreatorID()
{}

/*=========================================================================
 *  DESCRIPTION OF FUNCTION:
 *  ==> see headerfile
 *=======================================================================*/
StatusCode iFatras::SimHitCreatorID::initialize()
{
  ATH_MSG_VERBOSE( "[ idhit ] initialize()" );
  // Get Pixel / SCT / TRT hit creator tools
  ATH_CHECK ( m_pixelHitCreator.retrieve( DisableTool(m_pixelHitCreator.empty()) ) );
  ATH_CHECK ( m_sctHitCreator.retrieve( DisableTool(m_sctHitCreator.empty()) ) );
  ATH_CHECK ( m_trtHitCreator.retrieve( DisableTool(m_trtHitCreator.empty()) ) );
  // Get ID Helper from detector store
  ATH_CHECK ( detStore()->retrieve(m_idHelper, m_idHelperName) );
  return StatusCode::SUCCESS;
}

//================ SimHit Creation Interface  =====================================
void iFatras::SimHitCreatorID::createHits(const ISF::ISFParticle& isp, const std::vector<Trk::HitInfo>& hitVector) const {
   // iterate and assign as well the layer
   std::vector<Trk::HitInfo>::const_iterator plIter    = hitVector.begin();
   std::vector<Trk::HitInfo>::const_iterator plIterEnd = hitVector.end();
   size_t nHits = 0;
   for ( ; plIter != plIterEnd; ++plIter ){
       // decide which HitCreator to take
       // get the informations from the Simulation
       const Trk::TrackParameters*  hitParameter  = (*plIter).trackParms.get();
       double time                                = (*plIter).time;
       // -------------------------------------------------------------------------
       const Trk::TrkDetElementBase* hitDetElement =  hitParameter->associatedSurface().associatedDetectorElement();

       // initialize an unvalid one
       Identifier hitId = hitDetElement ?  hitDetElement->identify() : Identifier();
       if (m_idHelper->is_pixel(hitId)) {
         // -----------------------------------------------------------------------
         // HIT in Pixel Detector
         // -----------------------------------------------------------------------
         ATH_MSG_VERBOSE(  "[ sim ] Creating Pixel Cluster" );
         m_pixelHitCreator->createSimHit(isp,*hitParameter,time); ++nHits;
       }
       else if (m_idHelper->is_sct(hitId)) {
         // -----------------------------------------------------------------------
         // HIT in SCT Detector
         // -----------------------------------------------------------------------
           ATH_MSG_VERBOSE(  "[ sim ] Creating SCT Cluster" );
           m_sctHitCreator->createSimHit(isp,*hitParameter,time); ++nHits;
       }
       else if (m_idHelper->is_trt(hitId)) {
         // -----------------------------------------------------------------------
         // HIT in TRT Detector 
         // -----------------------------------------------------------------------
         ATH_MSG_VERBOSE(  "[ sim ] Creating TRT DriftCircle" );
         m_trtHitCreator->createSimHit(isp,*hitParameter,time); ++nHits;
       }
   }
  ATH_MSG_VERBOSE(  "[ idtrack ] **** done, " << nHits << " hits created from this track." );    
}

