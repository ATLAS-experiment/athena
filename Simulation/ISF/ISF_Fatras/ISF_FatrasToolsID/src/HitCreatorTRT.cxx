/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "HitCreatorTRT.h"

// Framework
#include "GaudiKernel/ISvcLocator.h"
#include "StoreGate/StoreGateSvc.h"
// ISF
#include "ISF_Interfaces/IParticleBroker.h"
#include "ISF_Event/ISFParticle.h"
// Tracking
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/SurfaceBounds.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkEventPrimitives/DefinedParameter.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkDetElementBase/TrkDetElementBase.h"
#include "TrkExUtils/RealQuadraticEquation.h"
// InDet
#include "InDetIdentifier/TRT_ID.h"
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"
#include "InDetSimEvent/TRTUncompressedHit.h"
#include "InDetSimEvent/TRTHitIdHelper.h"
// CLHEP
#include "CLHEP/Units/PhysicalConstants.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/RandLandau.h"

//================ Constructor =================================================
iFatras::HitCreatorTRT::HitCreatorTRT(const std::string& t,
                                      const std::string& n,
                                      const IInterface*  p ) :
  base_class(t,n,p)
{
}


//================ Initialisation =================================================

StatusCode iFatras::HitCreatorTRT::initialize()
{
  // Random number service
  ATH_CHECK ( m_randomSvc.retrieve() );
  //Get own engine with own seeds:
  m_randomEngine = m_randomSvc->GetEngine(m_randomEngineName);
  if (!m_randomEngine) {
    ATH_MSG_ERROR( "[ --- ] Could not get random engine '" << m_randomEngineName << "'" );
    return StatusCode::FAILURE;
  }

  // Tools & Services
  ATH_CHECK ( m_trtStatusSummaryTool.retrieve( DisableTool{ !m_useConditionsSvc } ) );
  // Get the TRT Identifier-helper:
  ATH_CHECK ( detStore()->retrieve(m_trtIdHelper, m_trtIdHelperName) );

  // Athena/Gaudi framework
  ATH_CHECK ( m_incidentSvc.retrieve() );
  // register to the incident service: BeginEvent for TrackCollection
  m_incidentSvc->addListener( this, IncidentType::BeginEvent );

  ATH_MSG_VERBOSE( "[ trthit ]  initialize() successful." );
  return StatusCode::SUCCESS;
}


void iFatras::HitCreatorTRT::handle( const Incident& inc ) {
  // check the incident type
  if ( inc.type() == IncidentType::BeginEvent ){
    // check if the hit collection already contains:
    // (a)     if yes ... try to retrieve it
    if ( evtStore()->contains<TRTUncompressedHitCollection>(m_collectionName) ){
      if ( (evtStore()->retrieve(m_hitColl , m_collectionName)).isFailure() )
        ATH_MSG_ERROR( "[ --- ] Unable to retrieve TRTUncompressedHitCollection " << m_collectionName);
      // (a)     if no ... try to create it
    } else {
      m_hitColl = new TRTUncompressedHitCollection( m_collectionName);
      if ( (evtStore()->record(m_hitColl, m_collectionName, true)).isFailure() ) {
        ATH_MSG_ERROR( "[ --- ] Unable to record TRTUncompressedHitCollection " << m_collectionName);
        delete m_hitColl; m_hitColl=0;
      }
    }
  }
  return;
}

void iFatras::HitCreatorTRT::createSimHit(const ISF::ISFParticle& isp, const Trk::TrackParameters& pars, double time ) const
{
  // get surface and DetElement base
  const Trk::Surface &hitSurface = pars.associatedSurface();
  const Trk::TrkDetElementBase* detElementBase = hitSurface.associatedDetectorElement();
  // the detector element cast -> needed
  const InDetDD::TRT_BaseElement* hitTrtDetElement = dynamic_cast<const InDetDD::TRT_BaseElement*>((detElementBase));
  //if cast fails, return 0
  if ( !hitTrtDetElement ) return;
  // checks for detector element and surface
  if (hitTrtDetElement) {
    // get the real identifier
    Identifier hitId = hitSurface.associatedDetectorElementIdentifier();
    // check conditions of the straw
    if (m_useConditionsSvc &&  !m_trtStatusSummaryTool->get_status(hitId) ){
      ATH_MSG_VERBOSE("[ trthit ] Straw is not active ( says ConditionsTool). Ignore. ");
      return;
    }
    // create entry/exit point
    const Amg::Vector3D& gpos1 = pars.position();
    Amg::Vector3D gpos2 = gpos1 + pars.momentum().unit();
    // get the rotation
    const Amg::Transform3D& sTransform = hitSurface.transform();
    // create the hep points
    Amg::Vector3D lpoint1(sTransform.inverse() * gpos1);
    Amg::Vector3D lpoint2(sTransform.inverse() * gpos2);

    // new direction in 3D frame of cylinder
    Amg::Vector3D direc((lpoint2 - lpoint1).unit());

    // get line and circle constants
    double k = (direc.y())/(direc.x());
    double d = (lpoint2.x()*lpoint1.y() - lpoint1.x()*lpoint2.y())/(direc.x());
    double R = hitSurface.bounds().r();

    // and solve the qaudratic equation
    Trk::RealQuadraticEquation pquad(1 + k*k, 2*k*d, d*d - R*R);
    if (pquad.solutions != Trk::none){
      double t1 = (pquad.first  - lpoint1.x())/direc.x();
      double t2 = (pquad.second - lpoint1.x())/direc.x();
      // the solutions in the 3D frame of the cylinder : local coordination
      Amg::Vector3D sol1raw( (lpoint1 + t1 * direc) );
      Amg::Vector3D sol2raw( (lpoint1 + t2 * direc) );

      ATH_MSG_VERBOSE("[ trthit ] Straw center : " << hitSurface.center());
      ATH_MSG_VERBOSE("[ trthit ] Entry / exit : " << sol1raw << ", " << sol2raw );

      //!< @todo: fill information, decide which one is pre/post step point
      const double mass               = isp.mass();
      const double mom2               = pars.momentum().mag2();
      const float  kineticEnergy      = sqrt(mom2+mass*mass)-mass;
      const float  energyDepositInKeV = 0.; // only used for TRT hits directly created by photons

      int barrel_endcap = 0;
      int ispos = 0;
      switch ( m_trtIdHelper->barrel_ec(hitId) ) {
      case -2:  barrel_endcap = 1; ispos = 1; break;
      case -1:  barrel_endcap = 0; ispos = 0; break;
      case  1:  barrel_endcap = 0; ispos = 1; break;
      case  2:  barrel_endcap = 1; ispos = 0; break;
      default:
        ATH_MSG_WARNING("[ trthit ] Problem estimating barrel_endcap and ispos" ); return;
      }

      const int ringwheel = m_trtIdHelper->layer_or_wheel(hitId);
      const int phisector = m_trtIdHelper->phi_module(hitId);
      const int layer     = m_trtIdHelper->straw_layer(hitId);
      const int istraw    = m_trtIdHelper->straw(hitId);

      const TRTHitIdHelper* hitid_helper = TRTHitIdHelper::GetHelper();
      const int hitID = hitid_helper->buildHitId( barrel_endcap, ispos, ringwheel, phisector,layer,istraw);

      ATH_MSG_VERBOSE("[ trthit ] Add a TRTUncompressedHit hit to the collection. ");
      // fill into the HitCollection
      HepMcParticleLink partLink(HepMC::uniqueID(isp), 0,
                                 HepMcParticleLink::IS_POSITION,
                                 HepMcParticleLink::IS_ID);
      m_hitColl->Emplace( hitID,
                          partLink,
                          isp.pdgCode(),
                          (float) kineticEnergy, (float) energyDepositInKeV,
                          (float) sol1raw.x(), (float) sol1raw.y(), sol1raw.z(),
                          (float) sol2raw.x(), (float) sol2raw.y(), sol2raw.z(),
                          (float) time );
    }

  }
}
