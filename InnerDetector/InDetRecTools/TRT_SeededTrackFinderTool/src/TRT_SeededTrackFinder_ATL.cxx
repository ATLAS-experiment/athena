/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   Implementation file for class InDet::TRT_SeededTrackFinder_ATL
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// Version 10 04/12/2006 Thomas Koffas
///////////////////////////////////////////////////////////////////


#include "GaudiKernel/MsgStream.h"

#include "CLHEP/Vector/ThreeVector.h"

//Track
//
#include "TrkSurfaces/PerigeeSurface.h"
#include "InDetRIO_OnTrack/SiClusterOnTrack.h"
#include "InDetPrepRawData/SiCluster.h"

//The main Input file
//
#include "TRT_SeededTrackFinderTool/TRT_SeededTrackFinder_ATL.h"

//SCT Geometry
//
#include "InDetIdentifier/TRT_ID.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "InDetRIO_OnTrack/TRT_DriftCircleOnTrack.h"
#include "TrkTrack/TrackInfo.h"

// For SiCombinatorialTrackFinder_xk
#include "SiSPSeededTrackFinderData/SiCombinatorialTrackFinderData_xk.h"

//Space point seed finding tool

#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include "TrkEventPrimitives/FitQuality.h"

//ReadHandle
#include "StoreGate/ReadHandle.h"

#include <cmath>
#include <iostream>
#include <iomanip>
#include <utility>
#include <string_view>

namespace{
  double
  getRadius(const Trk::SpacePoint* pPoint){
    return pPoint->globalPosition().perp();
  }
  double
  getZ(const Trk::SpacePoint* pPoint){
    return pPoint->globalPosition().z();
  }
  
  double 
  thetaFromSpacePoints(const Trk::SpacePoint* pPoint0, const Trk::SpacePoint* pPoint1 ){
    const double deltaR = getRadius(pPoint1) - getRadius(pPoint0);
    const double deltaZ = getZ(pPoint1) - getZ(pPoint0);
    return std::atan2(deltaR,deltaZ);
  }
  
}

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::TRT_SeededTrackFinder_ATL::TRT_SeededTrackFinder_ATL
(const std::string& t,const std::string& n,const IInterface* p)
  : AthAlgTool(t,n,p)
{
  declareInterface<ITRT_SeededTrackFinder>(this);
}

InDet::TRT_SeededTrackFinder_ATL::~TRT_SeededTrackFinder_ATL()= default;;


///////////////////////////////////////////////////////////////////
// Initialisation
///////////////////////////////////////////////////////////////////

StatusCode 
InDet::TRT_SeededTrackFinder_ATL::initialize(){

  ATH_MSG_DEBUG("Initializing TRT_SeededTrackFinder_ATL");
  magneticFieldInit();
  ATH_CHECK( m_fieldCondObjInputKey.initialize());

  // Get propagator tool
  ATH_CHECK(m_proptool.retrieve());
  
  // Get updator tool
  ATH_CHECK(m_updatorTool.retrieve());
  
  // Get detector elements road maker tool
  ATH_CHECK(m_roadmaker.retrieve());

  // Get seeds maker tool
  ATH_CHECK(m_seedmaker.retrieve());

  // Get combinatorial track finder tool
  ATH_CHECK( m_tracksfinder.retrieve());

  // Set track quality cuts for track finding tool
  setTrackQualityCuts();

  ATH_CHECK( detStore()->retrieve(m_trtId, "TRT_ID"));
  
  // Get output print level
  if(msgLvl(MSG::DEBUG)){ dumpconditions(msg(MSG::DEBUG));  msg(MSG::DEBUG) << endmsg;}

  //initlialize readhandlekey
  ATH_CHECK(m_caloClusterROIKey.initialize(m_searchInCaloROI));

  return StatusCode::SUCCESS;

}

///////////////////////////////////////////////////////////////////
// Finalize
///////////////////////////////////////////////////////////////////

StatusCode 
InDet::TRT_SeededTrackFinder_ATL::finalize(){
   return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Dumps relevant information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream&  
InDet::TRT_SeededTrackFinder_ATL::dump( MsgStream& out ) const{
  out<<"\n";
  return dumpconditions(out);
}

///////////////////////////////////////////////////////////////////
// Dumps conditions information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& 
InDet::TRT_SeededTrackFinder_ATL::dumpconditions( MsgStream& out ) const{
  auto paddedName=[](const auto & str)->std::string{
    const auto n = 62-std::size(str);
    std::string result(n,' '); //padding
    result+='|';
    result+='\n';
    return std::string(str)+result;
  };
  
  auto format = [](MsgStream& m)->MsgStream&{
    m<<std::setw(12)<<std::setprecision(5);
    return m;
  };
  
  constexpr std::string_view  fieldmode[9] ={"NoField"       ,"ConstantField","SolenoidalField",
			     "ToroidalField" ,"Grid3DField"  ,"RealisticField" ,
			     "UndefinedField","AthenaField"  , "?????"         };

  int mode = m_fieldprop.magneticFieldMode();
  if(mode<0 || mode>8 ) mode = 8;
  
  constexpr auto horizontalRule{
    "|-----------------------------------------------------------------------------------------|\n"
  }; 
  
  constexpr auto padding{"                                                  |\n"};
  out<<horizontalRule;
  out<<"| Tool for propagation    | "<<paddedName(m_proptool.type());
  out<<"| Tool for updator        | "<<paddedName(m_updatorTool.type());
  out<<"| Tool for road maker     | "<<paddedName(m_roadmaker.type());
  out<<"| Tool for seed maker     | "<<paddedName(m_seedmaker.type());
  out<<"| Tool for track finding  | "<<paddedName(m_tracksfinder.type());
  out<<"| Magnetic field mode     | "<<paddedName(fieldmode[mode]);
  out<<"| Min pT of track (MeV)   | "<<format<<m_pTmin<<padding;
  out<<"| Max Xi2 for cluster     | "<<format<<m_xi2max<<padding;
  out<<"| Max Xi2 for outlayer    | "<<format<<m_xi2maxNoAdd<<padding;
  out<<"| Max Xi2 for link        | "<<format<<m_xi2maxlink<<padding;
  out<<"| Min number of clusters  | "<<std::setw(12)<<m_nclusmin<<padding;
  out<<"| Max number holes        | "<<std::setw(12)<<m_nholesmax<<padding;
  out<<"| Max holes  gap          | "<<std::setw(12)<<m_dholesmax<<padding;
  out<<horizontalRule;
  return out;
}


///////////////////////////////////////////////////////////////////
// Dumps relevant information into the ostream
///////////////////////////////////////////////////////////////////

std::ostream& 
InDet::TRT_SeededTrackFinder_ATL::dump( std::ostream& out ) const{
  return out;
}

///////////////////////////////////////////////////////////////////
// Initiate track finding tool
///////////////////////////////////////////////////////////////////

std::unique_ptr<InDet::ITRT_SeededTrackFinder::IEventData>
InDet::TRT_SeededTrackFinder_ATL::newEvent(const EventContext& ctx, SiCombinatorialTrackFinderData_xk& combinatorialData) const
{

  ///Get the seeds
  auto event_data_p = std::make_unique<InDet::TRT_SeededTrackFinder_ATL::EventData>(combinatorialData,
                                                                                  m_seedmaker->newEvent());
  // New event for track finder tool
  m_tracksfinder->newEvent(ctx, event_data_p->combinatorialData());

  // Print event information
  if(msgLvl(MSG::DEBUG)) {
     dumpconditions(msg(MSG::DEBUG)); 
     msg(MSG::DEBUG) << endmsg;
  }

  //  Get the calo ROI:
  if(m_searchInCaloROI ) {
    SG::ReadHandle<ROIPhiRZContainer> calo_rois(m_caloClusterROIKey, ctx);
    if (!calo_rois.isValid()) {
       ATH_MSG_FATAL("Failed to get EM Calo cluster collection " << m_caloClusterROIKey );
    }
    event_data_p->setCaloClusterROIEM(*calo_rois);
  }
  return event_data_p;
}

///////////////////////////////////////////////////////////////////////
std::unique_ptr<InDet::ITRT_SeededTrackFinder::IEventData>
InDet::TRT_SeededTrackFinder_ATL::newRegion(const EventContext& ctx, SiCombinatorialTrackFinderData_xk& combinatorialData,
                                            const std::vector<IdentifierHash>& listOfPixIds,
                                            const std::vector<IdentifierHash>& listOfSCTIds) const{
  auto event_data_p = std::make_unique<InDet::TRT_SeededTrackFinder_ATL::EventData>(combinatorialData,
                                                      m_seedmaker->newRegion(listOfPixIds,listOfSCTIds));

  // New event for track finder tool
  m_tracksfinder->newEvent(ctx, event_data_p->combinatorialData());


  // Print event information
  if(msgLvl(MSG::DEBUG)) {
     dumpconditions(msg(MSG::DEBUG)); 
     msg(MSG::DEBUG) << endmsg;
  }

  return event_data_p;
}

///////////////////////////////////////////////////////////////////
// Finalize track finding tool for given event
///////////////////////////////////////////////////////////////////

void 
InDet::TRT_SeededTrackFinder_ATL::endEvent(InDet::ITRT_SeededTrackFinder::IEventData &virt_event_data) const{
  //event_data cannot be const if passed to m_tracksfinder->endEvent
  InDet::TRT_SeededTrackFinder_ATL::EventData &event_data = EventData::getPrivateEventData(virt_event_data);
  // End event for track finder tool
  m_tracksfinder->endEvent(event_data.combinatorialData());

}

///////////////////////////////////////////////////////////////////
// Main method for back tracking through the Si ID
// starting from an intial track segment
///////////////////////////////////////////////////////////////////

std::list<Trk::Track*> 
InDet::TRT_SeededTrackFinder_ATL::getTrack(const EventContext& ctx,
 InDet::ITRT_SeededTrackFinder::IEventData &virt_event_data, const Trk::TrackSegment& tS) const{
  //non-const because findTrack alters event_data
  InDet::TRT_SeededTrackFinder_ATL::EventData &event_data = EventData::getPrivateEventData(virt_event_data);
  // return list, will be copied by value (fix!)
  std::list<Trk::Track*> aSiTrack;

  // protection for having the expected type of segment parameters
  if ( tS.localParameters().parameterKey() != 31 ){
    ATH_MSG_WARNING("Wrong type of TRT segment, rejected !" );
    return aSiTrack;
  }

  //Get the track segment information and build the initial track parameters
  const Trk::LocalParameters&     Vp   = tS.localParameters();
  if (tS.associatedSurface().type() != Trk::SurfaceType::Line) {
    throw std::logic_error("Unhandled surface.");
  }

  const AmgSymMatrix(5)& locCov = tS.localCovariance();
  AmgSymMatrix(5) ie  = locCov;

  std::unique_ptr<Trk::TrackParameters> newPerPar =
    tS.associatedSurface().createUniqueTrackParameters(Vp.get(Trk::loc1),
                                      Vp.get(Trk::loc2),
                                      Vp.get(Trk::phi),
                                      Vp.get(Trk::theta),
                                      Vp.get(Trk::qOverP),
                                      ie);

  if(newPerPar){
    ATH_MSG_DEBUG("Initial Track Parameters created from TRT segment, ");
    ATH_MSG_VERBOSE(*newPerPar) ;
  }else{
    ATH_MSG_WARNING( "Could not get initial TRT track parameters, rejected! " );
    return aSiTrack;
  }
  constexpr auto horizontalRule{"==============================================================\n"};
  //Do the actual tracking and smoothing and get the Si track states on surface
  ATH_MSG_DEBUG(horizontalRule << "Start initial search with 3-4 SCT layer combinations ");

  if (m_searchInCaloROI && !isCaloCompatible( *newPerPar, event_data )) {
    return aSiTrack;
  }

  // Get AtlasFieldCache
  SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCondObjInputKey, ctx};
  const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};
  if (fieldCondObj == nullptr) {
      ATH_MSG_ERROR("TRT_SeededTrackFinder_ATL::getTracks: Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCondObjInputKey.key());
      return aSiTrack;
  }
  MagField::AtlasFieldCache fieldCache;
  fieldCondObj->getInitializedCache (fieldCache);

  aSiTrack = findTrack(ctx, fieldCache, event_data, *newPerPar, tS);
  if((aSiTrack.empty())&&(m_bremCorrect)){
    ATH_MSG_DEBUG(horizontalRule << "Could not create track states on surface for this track!Try to add brem correction.");
    
    std::unique_ptr<const Trk::TrackParameters> modTP = modifyTrackParameters(*newPerPar,0);
    ATH_MSG_VERBOSE("Modified TRT Track Parameters for brem. \n"<<(*modTP));
    aSiTrack = findTrack(ctx, fieldCache, event_data, *modTP, tS);
    if(aSiTrack.empty()){
      ATH_MSG_DEBUG("Could not create track states on surface for this track after all!");
      return aSiTrack;
    }
    ATH_MSG_DEBUG(horizontalRule);
  }

  //Return list of tracks (by value !?)
  return aSiTrack;
}

///////////////////////////////////////////////////////////////////
// Main method for back tracking through the Si ID
// starting from initial track parameters
///////////////////////////////////////////////////////////////////

std::list<Trk::Track*> 
InDet::TRT_SeededTrackFinder_ATL::findTrack(const EventContext& ctx, MagField::AtlasFieldCache& fieldCache,
 InDet::TRT_SeededTrackFinder_ATL::EventData &event_data,const Trk::TrackParameters & initTP,
 const Trk::TrackSegment& tS) const{
  SiCombinatorialTrackFinderData_xk& combinatorialData=event_data.combinatorialData();
  //Return list copied by value (fix!!)
  std::list<Trk::Track*> associatedSiTrack; // List of found tracks per TRT segment
  constexpr double pi2 = 2.*M_PI;
  constexpr double pi=M_PI;

  //Get the seeds
  std::list<std::pair<const Trk::SpacePoint*,const Trk::SpacePoint*> >
      SpE = m_seedmaker->find2Sp(ctx, initTP,event_data.spacePointFinderEventData());                //Get a list of SP pairs

  ATH_MSG_DEBUG("---------------> SP SEED LIST SIZE " << SpE.size() );
  if(SpE.empty()){return associatedSiTrack;}

  //
  // --------------- loop over the found seeds
  //

  //Get the track states on surface that correspond to each SP pair that came from the seeding
  std::vector<const Trk::SpacePoint*> SpVec{nullptr, nullptr};

  std::list<std::pair<const Trk::SpacePoint*,const Trk::SpacePoint*> >::iterator r,re=SpE.end();
  for(r=SpE.begin();r!=re; ++r){
    std::list<Trk::Track*>         aTracks   ; // List of tracks found per seed
    std::list<Trk::Track*>         cTracks   ; // List of cleaned tracks found per seed
    event_data.noise().reset();  //Initiate the noise production tool at the beginning of each seed

    //
    // --------------- filter SP to improve prediction, scale errors
    //

    //Get the track parameters to use to get the detector elements for each SP pair
    std::pair<const Trk::SpacePoint*, const Trk::SpacePoint*>& pSP = *r;
    if (pSP.first != pSP.second){
      ATH_MSG_DEBUG( "----> Seed Pair: SP 1 " << (pSP.first)->r() << " SP 2 " << (pSP.second)->r() );
    } else {
      if(msgLvl(MSG::DEBUG)) {
        msg(MSG::DEBUG) << "----> Seed Single: SP 1 " << (pSP.first)->r() << "\n";
        msg(MSG::DEBUG) << "      Will not process for the time being ! A special module is needed\n" ;
        msg(MSG::DEBUG) << "      to deal with late conversion (no search and stablized input fit ).\n";
        msg(MSG::DEBUG) << "      Current version is unstable in the SP updates and gets unpredictable results." << endmsg;
      }
      continue;
    }

    ///List of space points in the current seed, starting from the one at the smaller radius
    SpVec[0]=(pSP.second); 
    SpVec[1]=(pSP.first);
    if(!newClusters(SpVec,event_data)) {
      ATH_MSG_DEBUG( "Seed SPs already used by a single track. Ignore..." );
      continue;
    }
    if(!newSeed(SpVec,event_data)) {
      ATH_MSG_DEBUG( "Seed SPs already used by other tracks. Ignore..." );
      continue;
    }

    //
    // ----------------Check the SP seed if field is ON
    //
    if(fieldCache.solenoidOn()){
      bool seedGood = checkSeed(SpVec,tS,initTP);
      if(!seedGood && m_propR) {
	      ATH_MSG_DEBUG( "Seed not consistent with TRT segment. Ignore..." );
	      continue;
      }
    }

    //
    // ----------------Get new better track parameters using the SP seed
    //
    ATH_MSG_DEBUG( "Get better track parameters using the seed" );
    double newTheta = thetaFromSpacePoints(SpVec[0], SpVec[1]);
    const AmgVector(5)& iv = initTP.parameters();
    double newPhi = iv[2];
    //Protect for theta and phi being out of range
    if     (newTheta > pi) newTheta = fmod(newTheta+pi,pi2)-pi;
    else if(newTheta <-pi) newTheta = fmod(newTheta-pi,pi2)+pi;
    if(newTheta<0.){ newTheta = -newTheta; newPhi+=pi; }
    if     (newPhi > pi) newPhi = fmod(newPhi+pi,pi2)-pi;
    else if(newPhi <-pi) newPhi = fmod(newPhi-pi,pi2)+pi;
    if(newTheta<0.27) {
      ATH_MSG_DEBUG("Pseudorapidity greater than 2.Ignore" );
      continue;
    }

    const AmgSymMatrix(5) * vCM = initTP.covariance();
    AmgSymMatrix(5) nvCM;
     nvCM<<
      m_errorScale[0]*m_errorScale[0]*(*vCM)(0,0),0.,0.,0.,0.,
      0.,m_errorScale[1]*m_errorScale[1]*(*vCM)(1,1),0.,0.,0.,
      0.,0.,m_errorScale[2]*m_errorScale[2]*(*vCM)(2,2),0.,0.,
      0.,0.,0.,m_errorScale[3]*m_errorScale[3]*(*vCM)(3,3),0.,
      //cppcheck-suppress constStatement
      0.,0.,0.,0.,m_errorScale[4]*m_errorScale[4]*(*vCM)(4,4);


    //New intial track parameters saved as MeasuredAtaStraightLine
    std::unique_ptr<const Trk::TrackParameters> niTP =
      initTP.associatedSurface().createUniqueTrackParameters(
        iv[0], iv[1], newPhi, newTheta, iv[4], nvCM);

    if (niTP) {
      ATH_MSG_DEBUG("Initial Track Parameters created and scaled from TRT segment, ");
      ATH_MSG_VERBOSE(*niTP);
    } else {
      ATH_MSG_DEBUG("Could not get initial TRT track parameters! " );
      continue;
    }

    //
    // ----------------Propagate through the SP seed
    //
    ATH_MSG_DEBUG( "Propagating through the seed" );
    bool outl = false;

    //update with first SP
    ATH_MSG_DEBUG( "Update with 1st SP from seed" );
    std::unique_ptr<const Trk::TrackParameters> upTP = getTP(fieldCache, pSP.first,*niTP,outl,event_data);
    //If no track parameters are found, go to the next seed
    if(!upTP){
      ATH_MSG_DEBUG( "Extrapolation through seed failed!Seed bogus.Move to next seed" );
      continue;
    }
    //Not good if SP pair has outliers. Clean up the memory and move to next seed
    if(outl){
      ATH_MSG_DEBUG("Seed with outliers. Will not process!");
      continue;
    }

    //update with second SP ?
    if (pSP.first != pSP.second) {
      //update with second SP
      ATH_MSG_DEBUG( "Update with 2nd SP from seed" );
      std::unique_ptr<const Trk::TrackParameters> newTP = getTP(fieldCache, pSP.second,*upTP,outl,event_data);
      //If no track parameters are found, go to the next seed
      if(!newTP){
	      ATH_MSG_DEBUG( "Extrapolation through seed failed!Seed bogus.Move to next seed" );
	      continue;
      }
      //Not good if SP pair has outliers. Clean up the memory and move to next seed
      if(outl){
	      ATH_MSG_DEBUG("Seed with outliers.Will not process!");
	      continue;
      }
      // copy the newTP to upTP
      upTP = std::move(newTP);
    }

    //Protect the road maker tool when the momentum is too low since the particle will spiral inside the tracker
    if(upTP->momentum().perp()<m_pTmin){
      ATH_MSG_DEBUG("Low pT.Stop! ");
      continue;
    }

    //
    // --------------- get Si detector element road
    //
    const Trk::PerigeeSurface persurf (Amg::Vector3D(0,0,0));

    //Get track parameters at the end of SCT to start backwards propagation
    auto per = m_proptool->propagate(ctx,*upTP,persurf,Trk::oppositeMomentum,false,
                                     m_fieldprop,Trk::nonInteracting); //Propagate
    if(!per){
      ATH_MSG_DEBUG("No extrapolated track parameters!");
      continue;
    }

    if(msgLvl(MSG::VERBOSE)) {
      msg(MSG::VERBOSE) << "Perigee after SP updates at same surface" << endmsg;
      msg(MSG::VERBOSE) << *per << endmsg;
    }

    //Get list of InDet Elements
    std::vector<const InDetDD::SiDetectorElement*> DE;
    m_roadmaker->detElementsRoad(ctx, fieldCache, *per,Trk::alongMomentum,DE,event_data.roadMakerData());
    if( int(DE.size()) < m_nclusmin){ //Not enough detector elements to satisfy the minimum number of clusters requirement. Stop
      ATH_MSG_DEBUG( "Too few detector elements, not expected" );
      continue;
    }

    //
    // --------------- Cast it to measured parameters at 2nd SP with diagonal error matrix
    //
    const AmgVector(5)& piv = upTP->parameters();

    //Get the intial Error matrix, diagonalize it and scale the errors
    std::unique_ptr<const Trk::TrackParameters> mesTP{};
    const AmgSymMatrix(5)* pvCM  = upTP->covariance();
    if(pvCM){
      AmgSymMatrix(5) pnvCM;
      pnvCM<<
        m_errorScale[0]*m_errorScale[0]*(*pvCM)(0,0),0.,0.,0.,0.,
        0.,m_errorScale[1]*m_errorScale[1]*(*pvCM)(1,1),0.,0.,0.,
        0.,0.,m_errorScale[2]*m_errorScale[2]*(*pvCM)(2,2),0.,0.,
        0.,0.,0.,m_errorScale[3]*m_errorScale[3]*(*pvCM)(3,3),0.,
        //cppcheck-suppress constStatement
        0.,0.,0.,0.,m_errorScale[4]*m_errorScale[4]*(*pvCM)(4,4);

      mesTP = upTP->associatedSurface().createUniqueTrackParameters(piv[0],piv[1],piv[2],piv[3],piv[4],pnvCM);
      if(mesTP){
	      ATH_MSG_DEBUG( "Initial Track Parameters at 1st SP created and scaled from TRT segment, " );
	      ATH_MSG_VERBOSE( *mesTP );
      }else{
        continue;
      }
    }else{
      continue;
    }

    //
    // --------------- Get the Si extensions using the combinatorial track finding tool
    //
    std::vector<Amg::Vector3D> Gp;
    aTracks = m_tracksfinder->getTracks(combinatorialData, *mesTP, SpVec, Gp, DE, m_trackquality, ctx);
    if(aTracks.empty()) {
      ATH_MSG_DEBUG("No tracks found by the combinatorial track finder!");
    }

    //
    // --------------- Drop spurious pixel hits
    //
    cTracks=cleanTrack(aTracks);

    //
    // --------------- Add tracks in the track multimap and in the overall list of TRT segment associated tracks
    //
    std::list<Trk::Track*>::iterator t = cTracks.begin();
    while(t!=cTracks.end()) {
      if(!isNewTrack((*t),event_data)) {
        delete (*t);
        cTracks.erase(t++);
      } else {
        clusterTrackMap((*t),event_data);
        associatedSiTrack.push_back((*t++));
      }
    }
  }  ///end of loop over seeds for this TRT segment

  return  associatedSiTrack;
}

///////////////////////////////////////////////////////////////////
// update track parameters using SP
///////////////////////////////////////////////////////////////////

std::unique_ptr<const Trk::TrackParameters>
InDet::TRT_SeededTrackFinder_ATL::getTP(MagField::AtlasFieldCache& fieldCache, const Trk::SpacePoint* SP,
                                        const Trk::TrackParameters& startTP,
                                        bool& outl,
                                        InDet::TRT_SeededTrackFinder_ATL::EventData &event_data) const{
  std::unique_ptr<const Trk::TrackParameters> iTP{};
  outl = false;
  const Trk::Surface&  surf  = SP->associatedSurface();//Get the associated surface
  Trk::PropDirection  dir   = Trk::oppositeMomentum;   //Propagate backwards i.e. opposite momentum when filtering
  Trk::ParticleHypothesis  part  = Trk::nonInteracting;//Choose a non interacting particle
  auto  eTP   = m_proptool->propagate(Gaudi::Hive::currentContext(),
                                      startTP,surf,dir,false,m_fieldprop,part); //Propagate

  if(!eTP){
    ATH_MSG_DEBUG( "Extrapolation to Si element failed");
    ATH_MSG_VERBOSE( surf );
    return nullptr;
  }else{
    //Based on the hit information update the track parameters and the error matrix
    Trk::FitQualityOnSurface*   sct_fitChi2 = nullptr;
    std::unique_ptr<const Trk::TrackParameters> uTP = m_updatorTool->addToState(*eTP,SP->localParameters(),SP->localCovariance(),sct_fitChi2);
    if(!uTP) { //The updator failed
      if (sct_fitChi2) {
	      ATH_MSG_DEBUG( "Updator returned no update, but a DitQuality object, a leak !");
	      delete sct_fitChi2;
      }
      event_data.noise().production(-1,1,*eTP);
     
      iTP = addNoise(event_data.noise(),*eTP,0);
      ATH_MSG_DEBUG("The updator failed! Count an outlier ");
      outl = true;
    }else{
      //Keep as a measurement only if fit chi2 less than 25.Otherwise outlier
      float outlierCut = m_outlierCut;
      if(!fieldCache.solenoidOn()) outlierCut = 1000000.; // Increase the outlier chi2 cut if solenoid field is OFF
      if( sct_fitChi2->chiSquared() < outlierCut && std::abs(uTP->parameters()[Trk::theta]) > 0.17 ){
        ATH_MSG_DEBUG("Update worked, will update return track parameters, chi2: "<<(sct_fitChi2->chiSquared()));
        event_data.noise().production(-1,1,*uTP);
        iTP = addNoise(event_data.noise(),*uTP,0);
      }else{
        ATH_MSG_DEBUG("Outlier, did not satisfy cuts, chi2: "<<(sct_fitChi2->chiSquared())<<" "<<std::abs(uTP->parameters()[Trk::theta]));
        event_data.noise().production(-1,1,*eTP);
        iTP = addNoise(event_data.noise(),*eTP,0);
        outl = true;
      }
      // Clean up
      delete sct_fitChi2;
    }
  }
  return iTP;
}

///////////////////////////////////////////////////////////////////
// Add noise to track parameters
// 0-Filtering, 1-Smoothing
///////////////////////////////////////////////////////////////////

std::unique_ptr<const Trk::TrackParameters> 
InDet::TRT_SeededTrackFinder_ATL::addNoise(const SiNoise_bt & noise, const Trk::TrackParameters& P1, int isSmooth) const{
  ATH_MSG_DEBUG( "Adding noise to track parameters... " );
  const double covAzim=noise.covarianceAzim();
  const double covPola=noise.covariancePola();
  const double covIMom=noise.covarianceIMom();
  const double corIMom=noise.correctionIMom();
  //Get the noise augmented parameters and the 15 lower half covariance matrix elements from the first input track parameters
  const AmgVector(5)& Vp1 = P1.parameters();
  double M[5]={Vp1[0],Vp1[1],Vp1[2],Vp1[3],Vp1[4]};
  if(!isSmooth){
    M[4] *= corIMom;
  }else{
    M[4] /= corIMom;
  }
  const AmgSymMatrix(5)* C = P1.covariance();
  if(C){
    AmgSymMatrix(5) nC;
    nC = (*C);
    nC(2,2)+=covAzim;
    nC(3,3)+=covPola;
    nC(4,4)+=covIMom;
    return P1.associatedSurface().createUniqueTrackParameters(M[0],M[1],M[2],M[3],M[4],nC);
  }
  return nullptr;
}



///////////////////////////////////////////////////////////////////
// Get new theta estimate using the SPs from the seed
///////////////////////////////////////////////////////////////////

bool 
InDet::TRT_SeededTrackFinder_ATL::checkSeed
     (std::vector<const Trk::SpacePoint*>& vsp,const Trk::TrackSegment& tS,const Trk::TrackParameters & tP) const{
  bool isGood = true;
  int nEC = 0; double gz = 0.;
  ///Get TRT segment track parameters
  const AmgVector(5)& pTS=tP.parameters();
  ///Process only if endcap-transition region
  if(std::abs(std::log(std::tan(pTS[3]/2.)))>0.8){
    ///Find the global z position of first endcap hit on TRT segment
    for(int it=0; it<int(tS.numberOfMeasurementBases()); it++){
      //Check if it is a pseudo measurement and move on
      if ( dynamic_cast<const Trk::PseudoMeasurementOnTrack*>(tS.measurement(it)) ) continue;
      const InDet::TRT_DriftCircleOnTrack* trtcircle = dynamic_cast<const InDet::TRT_DriftCircleOnTrack*>(tS.measurement(it));
      if(trtcircle){
        Identifier id=trtcircle->detectorElement()->identify();
        int isB = m_trtId->barrel_ec(id);
        if(isB==2 || isB==-2) nEC++;
        if(nEC==1){
          gz = trtcircle->globalPosition().z();
          break;
        }
      }
    }
    double tanTheta = std::tan(thetaFromSpacePoints(vsp[0], vsp[1]));
    ///Propagate at the z position of 1st endcap hit on TRT segment
    double propR = getRadius(vsp[1]) + (gz-getZ(vsp[1]))*tanTheta;
    if(propR<620. || propR>1010.) isGood=false;
    double zIn = gz-propR/tanTheta;
    if(zIn>300.) isGood = false;
  }
  return isGood;
}

///////////////////////////////////////////////////////////////////
//  Calculate initial track parameters for back tracking
//  using TRT New Tracking segments
///////////////////////////////////////////////////////////////////

std::unique_ptr<const Trk::TrackParameters>
InDet::TRT_SeededTrackFinder_ATL::modifyTrackParameters(const Trk::TrackParameters& TP, int mode){
  ///The mode corresponds to whether the track parameters are modified before the seed (0) or before the pixel propagation (1)  
  ///Get the track parameters
  const AmgVector(5)& pV = TP.parameters();
  double ip[5] = {pV[0], pV[1], pV[2], pV[3], pV[4]};

  ///Correct inverse momentum and covariance. Inverse momentum halved, i.e. momentum doubled
  const double & q = ip[4]; //no need to take std::abs, its squared next
  const double covarianceIMom = 0.25*q*q;
  ip[4] *= 0.5;

  const AmgSymMatrix(5) * CM = TP.covariance();
  AmgSymMatrix(5)        nM = (*CM);

  if(mode==0){ //Modification initiated before seed propagation
    nM(4,4) = 10.*(0.1*((*CM)(4,4))+covarianceIMom);
  }
  if(mode==1){  //Modification initiated before pixel propagation
    nM(4,4)+=covarianceIMom;
  }
  return TP.associatedSurface().createUniqueTrackParameters(ip[0], ip[1], ip[2], ip[3], ip[4], nM);  
}

///////////////////////////////////////////////////////////////////
// Set track quality cuts
///////////////////////////////////////////////////////////////////

void  
InDet::TRT_SeededTrackFinder_ATL::setTrackQualityCuts(){
  // Integer cuts
  //
  m_trackquality.setIntCut   ("MinNumberOfClusters",  m_nclusmin   );
  m_trackquality.setIntCut   ("MinNumberOfWClusters", m_nwclusmin  );
  m_trackquality.setIntCut   ("MaxNumberOfHoles"   ,  m_nholesmax  );
  m_trackquality.setIntCut   ("MaxHolesGap"        ,  m_dholesmax  );
  if( m_useassoTool ) m_trackquality.setIntCut   ("UseAssociationTool",1);
  else                m_trackquality.setIntCut   ("UseAssociationTool",0);

  // Double cuts
  //
  m_trackquality.setDoubleCut("pTmin"              ,m_pTmin      );
  m_trackquality.setDoubleCut("MaxXi2forCluster"   ,m_xi2max     );
  m_trackquality.setDoubleCut("MaxXi2forOutlier"   ,m_xi2maxNoAdd);
  m_trackquality.setDoubleCut("MaxXi2forSearch"    ,m_xi2maxlink );
}

///////////////////////////////////////////////////////////////////
// Clusters-track multimap production
///////////////////////////////////////////////////////////////////

void  InDet::TRT_SeededTrackFinder_ATL::clusterTrackMap(Trk::Track* Tr,
                                                        InDet::TRT_SeededTrackFinder_ATL::EventData &event_data)
{
  DataVector<const Trk::MeasurementBase>::const_iterator
    m  = Tr->measurementsOnTrack()->begin(),
    me = Tr->measurementsOnTrack()->end  ();

  for(; m!=me; ++m) {
    const Trk::PrepRawData* prd = ((const Trk::RIO_OnTrack*)(*m))->prepRawData();
    if(prd) event_data.clusterTrack().insert(std::make_pair(prd,Tr));
  }
}

///////////////////////////////////////////////////////////////////
// New clusters comparison with clusters associated with track
// Reject seeds that all SPs belong to one and the same track
///////////////////////////////////////////////////////////////////

bool 
InDet::TRT_SeededTrackFinder_ATL::newClusters(const std::vector<const Trk::SpacePoint*>& Sp,
                                    InDet::TRT_SeededTrackFinder_ATL::EventData &event_data){
  const Trk::PrepRawData* prd   [ 40];
  const Trk::Track*       trk[2][200];
  std::multimap<const Trk::PrepRawData*,const Trk::Track*>::const_iterator
     t[40],te = event_data.clusterTrack().end();
  std::vector<const Trk::SpacePoint*>::const_iterator s=Sp.begin(),se=Sp.end();
  int n = 0;

  //If at least one of the clusters in the seed is not used by a track the seed is good
  for(; s!=se; ++s) {
     if((*s)->clusterList().first ) {
       prd[n] = (*s)->clusterList().first;
       t  [n] = event_data.clusterTrack().find(prd[n]); 
       if(t[n]==te) return true; 
       ++n;
     }
     if((*s)->clusterList().second) {
       prd[n] = (*s)->clusterList().second;
       t  [n] = event_data.clusterTrack().find(prd[n]); 
       if(t[n]==te) return true; 
       ++n;
     }
     if(n==40) break;
  }
  if(!n) return false;

  //Array of pointers to the different tracks that the first used cluster belongs to
  int m = 0;
  auto & pTracks=t[0];
  for(; pTracks!=te; ++pTracks) {
    if (m==30) return false;
    if( (*pTracks).first != prd[0] ) break;
    trk[0][m++] = (*pTracks).second;
    if(m==200) break;
  }

  //For a seed to be declared bad, all other clusters should belong to the same track as that of the first used cluster
  int in=0, ou=1;
  for(int i=1; i!=n; ++i) {
    int l = 0; //Number of tracks that share the same clusters
    auto & pTheseTracks=t[i];
    for(; pTheseTracks!=te; ++pTheseTracks) {
      if( (*pTheseTracks).first != prd[i] ) break;
      for(int j=0; j!=m; ++j) {
  	    if((*pTheseTracks).second == trk[in][j]) {trk[ou][l++]= trk[in][j]; 
  	      break;
  	    }
      }
    }
    if(l==0) return true; //At least one of the seed clusters belongs to a track different from that of the first used clusters
    m=l; 
    if(in==0) {
      in=1; ou=0;
    } else {
      in=0; ou=1;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////
// New clusters comparison with clusters associated with track
// Reject seeds that all SPs have been already used by other tracks
///////////////////////////////////////////////////////////////////

 bool 
 InDet::TRT_SeededTrackFinder_ATL::newSeed(const std::vector<const Trk::SpacePoint*>& Sp,
                                InDet::TRT_SeededTrackFinder_ATL::EventData &event_data){
  const Trk::PrepRawData* prd   [ 40];
  const Trk::Track*       trk[2][200];
  std::multimap<const Trk::PrepRawData*,const Trk::Track*>::const_iterator
     tt,t[40],te = event_data.clusterTrack().end();

  std::vector<const Trk::SpacePoint*>::const_iterator s=Sp.begin(),se=Sp.end();
  int n  = 0;
  int nc = 0;
  for(; s!=se; ++s) {
     if((*s)->clusterList().first ) {
       prd[n] = (*s)->clusterList().first;
       t  [n] = event_data.clusterTrack().find(prd[n]); if(t[n]!=te) ++nc; ++n;
     }
     if((*s)->clusterList().second) {
       prd[n] = (*s)->clusterList().second;
       t  [n] = event_data.clusterTrack().find(prd[n]); if(t[n]!=te) ++nc; ++n;
     }
     if(n>=40) break;
  }
  if(!nc) return true;
  if(n==nc) {
    int m = 0;
    for(tt=t[0]; tt!=te; ++tt) {
      if( (*tt).first != prd[0] ) break;
      trk[0][m++] = (*tt).second;
      if(m==200) break;
    }
    int in=0, ou=1, i=1;
    for(; i!=n; ++i) {
      int l = 0;
      for(tt=t[i]; t[i]!=te; ++tt) {
	      if( (*tt).first != prd[i] ) break;
	      for(int j=0; j!=m; ++j) {
	        if((*tt).second == trk[in][j]) {
	          trk[ou][l++]= trk[in][j]; 
	          break;
	        }
	      }
      }
      if(l==0) break;
      m=l; 
      if(in==0) {
        in=1; 
        ou=0;
      } else {
        in=0;
        ou=1;
      }
    }
    if(i==n) return false;
  }

  //if(!(*Sp.rbegin())->clusterList().second) return true;

  int h = 0;
  for(int i=0; i!=n; ++i) {
    for(tt=t[i]; t[i]!=te; ++tt) {
      if( (*tt).first != prd[i] ) break;
      if((*tt).second->trackStateOnSurfaces()->size() >= 10) {
        ++h; 
        break;
      }
    }
  }
  return h == 0;
}

///////////////////////////////////////////////////////////////////
// Test is it new track
///////////////////////////////////////////////////////////////////

bool 
InDet::TRT_SeededTrackFinder_ATL::isNewTrack(Trk::Track* Tr,
                             InDet::TRT_SeededTrackFinder_ATL::EventData &event_data){

  const Trk::PrepRawData* prd   [100];
  std::multimap<const Trk::PrepRawData*,const Trk::Track*>::const_iterator
     ti,t[100],te = event_data.clusterTrack().end();
  int n = 0    ;
  DataVector<const Trk::MeasurementBase>::const_iterator
    m  = Tr->measurementsOnTrack()->begin(),
    me = Tr->measurementsOnTrack()->end  ();

  for(; m!=me; ++m) {
    const Trk::PrepRawData* pr = ((const Trk::RIO_OnTrack*)(*m))->prepRawData();
    if(pr) {
      prd[n] =pr;
      t[n] = event_data.clusterTrack().find(prd[n]); 
      if(t[n]==te) return true; 
      ++n;
    }
  }

  int nclt = n;

  for(int i=0; i!=n; ++i) {
    int nclmax = 0;
    for(ti=t[i]; ti!=te; ++ti) {
      if( (*ti).first != prd[i] ) break;
      int ncl = (*ti).second->trackStateOnSurfaces()->size();
      if(ncl > nclmax) nclmax = ncl;
    }
    if(nclt > nclmax) return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////
// Eliminate spurious pixel hits
///////////////////////////////////////////////////////////////////

std::list<Trk::Track*> 
InDet::TRT_SeededTrackFinder_ATL::cleanTrack(std::list<Trk::Track*> lTrk) const{
  std::list<Trk::Track*> cleanSiTrack; // List of clean Si tracks per TRT segment
  std::list<Trk::Track*>::const_iterator it    = lTrk.begin();
  std::list<Trk::Track*>::const_iterator itEnd = lTrk.end();
  for (; it != itEnd ; ++it){
    int nPixHits = 0;  //Number of Pixel PRDs
    int nSctHits = 0;  //Number of SCT PRDs
    double pixR = 0.;  //Radial position of last pixel PRD
    double sctR = 0.;  //Radial position of first SCT PRD

    const Trk::TrackStates* newtsos = (*it)->trackStateOnSurfaces();
    if(!newtsos) continue;
    Trk::TrackStates::const_iterator itp, itpe=newtsos->end();
    for(itp=newtsos->begin(); itp!=itpe; ++itp){
      ///Concentrate on the Si component of the track
      const InDet::SiClusterOnTrack* clus = dynamic_cast<const InDet::SiClusterOnTrack*>((*itp)->measurementOnTrack());
      if(clus && ((*itp)->type(Trk::TrackStateOnSurface::Measurement))){  //Count the number of hits used in the track
        const InDet::SiCluster* RawDataClus=dynamic_cast<const InDet::SiCluster*>(clus->prepRawData());
        if(RawDataClus==nullptr){
	        ATH_MSG_DEBUG( "Si Cluster without PrepRawData!!!" );
          continue;
        }
        if(RawDataClus->detectorElement()->isPixel()){
          nPixHits++;
          pixR = RawDataClus->globalPosition().perp();
        }
        if((RawDataClus->detectorElement()->isSCT()) && (nSctHits==0)) {
          sctR = RawDataClus->globalPosition().perp();
          nSctHits++;
          break;
        }
      }
    }

    ///Throw out any spurious pixel hits.Need to rebuild the vector of track states on surface from scratch, since it's const in EDM
    if(nPixHits==1 && (sctR-pixR)>200.){
      auto cltsos = std::make_unique<Trk::TrackStates>();
      auto fq = (*it)->fitQuality()->uniqueClone();
      // copy track Si states into track
      Trk::TrackStates::const_iterator p_tsos;
      for(p_tsos=newtsos->begin()+nPixHits;p_tsos!=newtsos->end();++p_tsos){
        cltsos->push_back( (*p_tsos)->clone() );
      }
      ///Construct the new track
      Trk::TrackInfo info;
 //     info.setPatternRecognitionInfo(Trk::TrackInfo::TRTSeededTrackFinder);
      Trk::Track* nTrack = new Trk::Track(info, std::move(cltsos), std::move(fq));
      cleanSiTrack.push_back(nTrack);
      delete (*it);
    }else{
      cleanSiTrack.push_back((*it));
    }
  }

  return cleanSiTrack;
}


///////////////////////////////////////////////////////////////////
// Test is it track with calo seed compatible
///////////////////////////////////////////////////////////////////
bool 
InDet::TRT_SeededTrackFinder_ATL::isCaloCompatible(const Trk::TrackParameters& Tp,
  const InDet::TRT_SeededTrackFinder_ATL::EventData &event_data) const{
   if(!event_data.caloClusterROIEM()) return false;
   const AmgVector(5)& Vp = Tp.parameters();
   const double F = Vp[2];
   ROIPhiRZContainer::const_iterator roi_iter = event_data.caloClusterROIEM()->lowerPhiBound( F, m_phiWidth);
   return roi_iter != event_data.caloClusterROIEM()->end();
}


///////////////////////////////////////////////////////////////////
// MagneticFieldProperties production
///////////////////////////////////////////////////////////////////
void  
InDet::TRT_SeededTrackFinder_ATL::magneticFieldInit(){
 if(m_fieldmode == "NoField") m_fieldprop = Trk::MagneticFieldProperties(Trk::NoField  );
 else if(m_fieldmode == "MapSolenoid") m_fieldprop = Trk::MagneticFieldProperties(Trk::FastField);
 else m_fieldprop = Trk::MagneticFieldProperties(Trk::FullField);
}
