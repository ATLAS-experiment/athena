/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkJiveXML/TrackRetriever.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/ListItem.h"

#include "TrkTrack/Track.h"
#include "TrkTrack/TrackInfo.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"


#include "TrkParameters/TrackParameters.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkEventPrimitives/TrackStateDefs.h"
#include "TrkEventPrimitives/JacobianThetaPToCotThetaPt.h"
#include "TrkTruthData/TrackTruthKey.h"
#include "TrkEventUtils/TrackStateOnSurfaceComparisonFunction.h"
#include "AthLinks/ElementLink.h"
#include "AthContainers/DataVector.h"

#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"

#include "TrkMeasurementBase/MeasurementBase.h"

// for residuals
#include "TrkToolInterfaces/IResidualPullCalculator.h"
#include "TrkEventPrimitives/ResidualPull.h"

// for detector id
#include "AtlasDetDescr/AtlasDetectorID.h"

#include "GaudiKernel/SystemOfUnits.h"

namespace JiveXML {
  ///Namespace for all helper functions
  namespace TrackRetrieverHelpers {

    /**
     * Obtain the perigee parameters for a given track, if available, and fill
     * them in the corresponding data vectors. Perigee parameters are written
     * out in the old format using @f$ \cot\theta @f$ and @f$ q/p_T @f$
     * @return the perigee parameter object for further use
     */
    const Trk::Perigee* getPerigeeParameters( const Trk::Track* track,
					      DataVect& pt, DataVect& d0, DataVect& z0, DataVect& phi0,
					      DataVect& cotTheta, DataVect& covMatrix){

      /**
       * Get perigee parameters in old format (@f$ d_0 @f$, @f$ z_0 @f$, @f$ \phi @f$, @f$ \cot\theta @f$, @f$ q/p_T @f$),
       * whereas tracking uses (@f$ d_0 @f$, @f$ z_0 @f$, @f$ \phi @f$, @f$ \theta @f$, q/p),
       * therefore a transformation of the covariance matrix is needed
       */
      const Trk::Perigee *perigee = track->perigeeParameters();

      //return immediately if there is no perigee information
      if (!perigee) return nullptr;

      //write out p_T
      if ((perigee->parameters())[Trk::qOverP]==0) pt.emplace_back(9999.);
      else pt.push_back( (perigee->charge() > 0) ? DataType(perigee->pT()/Gaudi::Units::GeV) : DataType((-perigee->pT())/Gaudi::Units::GeV));

      d0.emplace_back((perigee->parameters())[Trk::d0]/Gaudi::Units::cm);
      z0.emplace_back(perigee->parameters()[Trk::z0]/Gaudi::Units::cm);
      phi0.emplace_back(perigee->parameters()[Trk::phi0]);

      if (perigee->parameters()[Trk::theta] == 0.) cotTheta.emplace_back(9999.);
      else cotTheta.emplace_back(1./tan(perigee->parameters()[Trk::theta]));

      // CLHEP->Eigen migration. jpt Dec'13
      // https://twiki.cern.ch/twiki/bin/viewauth/Atlas/MigrationCLHEPtoEigen
      // https://twiki.cern.ch/twiki/bin/viewauth/Atlas/MigrationToUpdatedEDM#Changes_to_TrkParameters

      /// get transformed covariance matrix
      AmgSymMatrix(5) covVert;

      const AmgSymMatrix(5)* covariance = perigee->covariance(); //perigee cannot be null here
      if (perigee && covariance) {
	// do trafo to old format
	double measuredTheta = perigee->parameters()[Trk::theta];
	double measuredQoverp = perigee->parameters()[Trk::qOverP];
	const Trk::JacobianThetaPToCotThetaPt theJac( measuredTheta, measuredQoverp );
	covVert = covariance->similarity(theJac);
      }else{
	for ( int ii=0; ii<20; ii++){ // placeholder. Do this nicer.
	  covVert(ii) = 0.;
	}
      }
      //Scale covariance matrix values to get good resolution with fixed
      //precision in JiveXML data

      const long scale = 10000;
      const double thisScale(scale/100.);
      // Migration: Now only has diagonal elements from covariance matrix ?
      covMatrix.emplace_back(covVert(0)*thisScale); // 5 elements
      covMatrix.emplace_back(covVert(1)*thisScale);
      covMatrix.emplace_back(covVert(2)*thisScale);
      covMatrix.emplace_back(covVert(3)*thisScale);
      covMatrix.emplace_back(covVert(4)*thisScale);

      // Used to be 15 elements before migration, so need to put 10 placeholders
      for ( int i=0; i<10; i++){
	covMatrix.emplace_back( 0. );
      }

      //All for perigee, return object for use by other functions
      return perigee;
    }

    /**
     * Get a list of track-State on Surfaces for measurement and
     * outlier hits, sorted using the perigee comparison functions
     * @return a std::vector of Trk::TrackStateOnSurface*
     */
    std::vector<const Trk::TrackStateOnSurface*> getTrackStateOnSurfaces( const Trk::Track* track, const Trk::Perigee* perigee, bool doHitsSorting){
      // vector for the return object
      std::vector<const Trk::TrackStateOnSurface*> TSoSVec;

      // loop over TrackStateOnSurfaces to extract interesting ones
      Trk::TrackStates::const_iterator tsos = track->trackStateOnSurfaces()->begin();
      for (; tsos!=track->trackStateOnSurfaces()->end(); ++tsos) {
	// include measurements AND outliers:
	if ((*tsos)->type(Trk::TrackStateOnSurface::Measurement) ||  (*tsos)->type(Trk::TrackStateOnSurface::Outlier) ) {
	  // add to temp vector
	  TSoSVec.push_back(*tsos);
	} // end if TSoS is measurement or outlier
      } // end loop over TSoS

      // sort the selected TSoS, if not already sorted using a comparison functor

      if (perigee) {
	//Get hold of the comparison functor
	Trk::TrackStateOnSurfaceComparisonFunction *compFunc
	  = new Trk::TrackStateOnSurfaceComparisonFunction(perigee->position(), perigee->momentum());
	if (compFunc){
	  if (doHitsSorting) {
	    //Sort track state on surface if needed
	    if (TSoSVec.size() > 2 && !is_sorted(TSoSVec.begin(), TSoSVec.end(), *compFunc))
	      std::sort(TSoSVec.begin(), TSoSVec.end(), *compFunc);
	  }
	}
	delete compFunc;
      } // end if compFunc

      //Now return that vector
      return TSoSVec;
    }

    /**
     * Get polyline hits if available. Polyline tracks that have less than 2 points are not useful - skip
     */
    void getPolylineFromHits( const std::vector<const Trk::TrackStateOnSurface*>& TSoSVec,
			      DataVect& polylineX, DataVect& polylineY, DataVect& polylineZ, DataVect& numPolyline){
      int numPoly = 0 ;
      if (TSoSVec.size() > 1) {
	//Loop over track state on surfaces
	std::vector<const Trk::TrackStateOnSurface*>::const_iterator tsosIter;
	const double onetenth(0.1);
	for (tsosIter=TSoSVec.begin(); tsosIter!=TSoSVec.end(); ++tsosIter) {
	  // get global track position
	  if (!(*tsosIter)->trackParameters()) continue ;
	  const Amg::Vector3D& pos = (*tsosIter)->trackParameters()->position();
	  polylineX.emplace_back(pos.x()*onetenth);
	  polylineY.emplace_back(pos.y()*onetenth);
	  polylineZ.emplace_back(pos.z()*onetenth);
	  ++numPoly;
	}
      }
      //Store counter as well
      numPolyline.emplace_back(numPoly);
    }


    /**
     * Retrieve all the basic hit information from the Trk::TrackStateOnSurface
     * @return the reconstruction input object (RIO) for further use
     */
    const Trk::RIO_OnTrack* getBaseInfoFromHit( const Trk::TrackStateOnSurface* tsos, const AtlasDetectorID* idHelper,
						DataVect& isOutlier, DataVect& hits, DataVect& driftSign, DataVect& tsosDetType){

      //Get corresponding measurement
      const Trk::MeasurementBase *measurement = tsos->measurementOnTrack();

      //If measurement is invalid, return imediately
      if (!measurement) return nullptr;

      // get RIO_OnTrack
      const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(measurement);

      // check for competing RIO_OnTracks
      if (!rot) {
	// try to get identifier by CompetingROT:
	const Trk::CompetingRIOsOnTrack* comprot = dynamic_cast<const Trk::CompetingRIOsOnTrack*>(measurement);
	//Get the input object with highest probability
	if (comprot) rot = &(comprot->rioOnTrack(comprot->indexOfMaxAssignProb()));
      }

      //If there is still no RIO_onTrack, return Null
      if (!rot) return nullptr;

      // Now start writing out values:
      // Check if this is an outlier hit
      isOutlier.emplace_back(tsos->type(Trk::TrackStateOnSurface::Outlier));

      //Now try to get the identifier, create an empty invalid one if no rot
      Identifier hitId (rot->identify());
      //Check if it is valid, othwerise store 0
      hits.push_back( hitId.is_valid()?DataType( hitId.get_compact() ):DataType(0) );

      // get sign of drift radius for TRT measurements
      int theDriftSign = 0;
      if (idHelper->is_trt(hitId)) {
	// get local parameters
	theDriftSign = measurement->localParameters()[Trk::driftRadius] > 0. ? 1 : -1;
      }
      driftSign.emplace_back(theDriftSign);

      //Now get the detector type of the hit
      if ( !hitId.is_valid() ){
	tsosDetType.emplace_back("unident");
      } else if (idHelper->is_pixel(hitId) ) {
	tsosDetType.emplace_back("PIX"); // is PIX in Atlantis
      } else if (idHelper->is_sct(hitId)) {
	tsosDetType.emplace_back("SIL"); // is SIL in Atlantis
      } else if (idHelper->is_trt(hitId)) {
	tsosDetType.emplace_back("TRT");
      } else if (idHelper->is_mdt(hitId)) {
	tsosDetType.emplace_back("MDT");
      } else if (idHelper->is_csc(hitId)) {
	tsosDetType.emplace_back("CSC");
      } else if (idHelper->is_rpc(hitId)) {
	tsosDetType.emplace_back("RPC");
      } else if (idHelper->is_tgc(hitId)) {
	tsosDetType.emplace_back("TGC");
      } else {
	tsosDetType.emplace_back("unident");
      }

      //Return the reco input object
      return rot;
    }


    /**
     * Get the residual pull information from the Trk::TrackStateOnSurface hit
     */
    void getResidualPullFromHit( const Trk::TrackStateOnSurface* tsos, const Trk::RIO_OnTrack* rot,
				 const ToolHandle<Trk::IResidualPullCalculator> & residualPullCalculator,
				 DataVect& tsosResLoc1, DataVect& tsosResLoc2, DataVect& tsosPullLoc1, DataVect& tsosPullLoc2 ){

      //Define default return values for invalid states
      double ResLoc1 = -99.;
      double ResLoc2 = -99.;
      double PullLoc1 = -99.;
      double PullLoc2 = -99.;

      // get TrackParameters on the surface to calculate residual
      const Trk::TrackParameters* tsosParameters = tsos->trackParameters();

      //Check we got the parameters
      if (tsosParameters){

	/**
	 * Using track residual tool: ResidualPullCalculator
	 * -excerpt from Tracking/TrkValidation/TrkValTools/src/BasicValidationNtupleTool.cxx
	 */

	//Get the residualPull object
        std::optional<Trk::ResidualPull> residualPull = residualPullCalculator->residualPull(rot, tsosParameters,Trk::ResidualPull::Biased);

	if (residualPull) {
	  //Get the first residual
	  ResLoc1 = residualPull->residual()[Trk::loc1];
	  //Get the second residual for more than 1 dimension
	  if (residualPull->dimension() >= 2) ResLoc2 = residualPull->residual()[Trk::loc2];

	  if ((residualPull->isPullValid()) ) {
	    //Get the first residual
	    PullLoc1 = residualPull->pull()[Trk::loc1];
	    //Get the second residual for more than 1 dimension
	    if (residualPull->dimension() >= 2) PullLoc2 = residualPull->pull()[Trk::loc2];
	  }
	} // end if residualPull
      } // end if tsosParameters

      //Finally store the values
      tsosResLoc1.emplace_back(ResLoc1 );
      tsosResLoc2.emplace_back(ResLoc2 );
      tsosPullLoc1.emplace_back(PullLoc1 );
      tsosPullLoc2.emplace_back(PullLoc2 );
    }

    /**
     * Get the barcode of the associated truth track
     */
    void getTruthFromTrack( const Trk::Track* track, const TrackCollection* trackCollection,
			    SG::ReadHandle<TrackTruthCollection>& truthCollection, DataVect& barcode){

      if (!truthCollection.isValid()) {
	//Fill with zero if none found
	barcode.emplace_back(0);
	return;
      }

      //Get the element link to this track collection
      ElementLink<TrackCollection> tracklink;
      tracklink.setElement(track);
      tracklink.setStorableObject(*trackCollection);

      //try to find it in the truth collection
      std::map<Trk::TrackTruthKey,TrackTruth>::const_iterator tempTrackTruthItr = truthCollection->find(tracklink);

      //Fill with zero if none found
      if (tempTrackTruthItr == truthCollection->end()){
	//Fill with zero if none found
	barcode.emplace_back(0);
	return;
      }

      //if found, Store the barcode
      barcode.emplace_back((*tempTrackTruthItr).second.particleLink().barcode());
    }

  } //namespace TrackRetrieverHelpers

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  TrackRetriever::TrackRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent){}

  /**
   * Initialize before event loop
   * - retrieve the residual-pull tool
   * - setup the ID helper
   */
  StatusCode TrackRetriever::initialize() {
    //Set up ATLAS ID helper to be able to identify the RIO's det-subsystem.
    ATH_CHECK(detStore()->retrieve(m_idHelper, "AtlasID"));
    
    // try to retrieve residual-pull calculation only if requested
    if (m_doWriteResiduals){
      ATH_CHECK(m_residualPullCalculator.retrieve());
    }

    ATH_CHECK(m_trackSumTool.retrieve());
    
    return StatusCode::SUCCESS;
  }



  /**
   * For each track collection retrieve all data
   * - loop over tracks in all collections
   * - for each track get basic parameters
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode TrackRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    ATH_MSG_DEBUG("In retrieve()");

    std::vector<std::string> keys = getKeys();

    if(keys.empty()){
      ATH_MSG_WARNING("No StoreGate keys found");
      return StatusCode::SUCCESS;
    }

    // Loop through the keys and retrieve the corresponding data
    for (const std::string& key : keys) {
      SG::ReadHandle<TrackCollection> cont(key);
      if (cont.isValid()) {
	DataMap data = getData(&(*cont),key);
	if (FormatTool->AddToEvent(dataTypeName(), key, &data).isFailure()) {
	  ATH_MSG_WARNING("Failed to retrieve Collection " << key);
	} else {
	  ATH_MSG_DEBUG(" (" << key << ") retrieved");
	}
      } else {
	ATH_MSG_WARNING("Collection " << key << " not found in SG");
      }
    }
    return StatusCode::SUCCESS;
  }

  const DataMap TrackRetriever::getData(const TrackCollection* trackCollection, const std::string collectionName) {

    ATH_MSG_DEBUG("in getData()");
    
    //Some sanity checks
    if ( trackCollection->empty()){
      ATH_MSG_DEBUG( "Empty track collection " << collectionName );
    } else {
      ATH_MSG_DEBUG( "Retrieving data for track collection " << collectionName);
    }

    /**
     * Try to find the appropiate truth collection
     * StoreGate key can be either 'TruthCollection' or just 'Truth' suffix. Check both.
     * Using 'SG::contains' check first, avoids spitting out SG warnings
     */

    SG::ReadHandle<TrackTruthCollection> truthCollection;
    
    // Check for given truth collection
    if (collectionName == m_priorityKey) {
      truthCollection = SG::ReadHandle<TrackTruthCollection>(m_TrackTruthCollection);
      if (truthCollection.isValid()) {
        ATH_MSG_DEBUG("Found TrackTruthCollection with key " << m_TrackTruthCollection);
      }
    }else {
      truthCollection = SG::ReadHandle<TrackTruthCollection>(collectionName + "TruthCollection");
      if (truthCollection.isValid()) {
        ATH_MSG_DEBUG("Found TrackTruthCollection with key " << collectionName << "TruthCollection");
      } else {
        truthCollection = SG::ReadHandle<TrackTruthCollection>(collectionName + "Truth");
        if (truthCollection.isValid()) {
	  ATH_MSG_DEBUG("Found TrackTruthCollection with key " << collectionName << "Truth");
        } else {
	  ATH_MSG_DEBUG("Could not find matching TrackTruthCollection for " << collectionName);
        }
      }
    }
      
    // Make a list of track-wise entries and reserve enough space
    DataVect id; id.reserve(trackCollection->size());
    DataVect chi2; chi2.reserve(trackCollection->size());
    DataVect numDoF; numDoF.reserve(trackCollection->size());
    DataVect trackAuthor; trackAuthor.reserve(trackCollection->size());
    DataVect barcode; barcode.reserve(trackCollection->size());
    DataVect numHits; numHits.reserve(trackCollection->size());
    DataVect numPolyline; numPolyline.reserve(trackCollection->size());
    DataVect nBLayerHits; nBLayerHits.reserve(trackCollection->size());
    DataVect nPixHits; nPixHits.reserve(trackCollection->size());
    DataVect nSCTHits; nSCTHits.reserve(trackCollection->size());
    DataVect nTRTHits; nTRTHits.reserve(trackCollection->size());

    // vectors with measurement- or TrackStateOnSurface-wise entries
    // reserve space later on
    DataVect polylineX;
    DataVect polylineY;
    DataVect polylineZ;
    DataVect tsosResLoc1;
    DataVect tsosResLoc2;
    DataVect tsosPullLoc1;
    DataVect tsosPullLoc2;
    DataVect tsosDetType;
    DataVect isOutlier;
    DataVect driftSign;
    DataVect hits;

    //Store wether this collection has perigee parameters
    DataVect pt; pt.reserve(trackCollection->size());
    DataVect d0; d0.reserve(trackCollection->size());
    DataVect z0; z0.reserve(trackCollection->size());
    DataVect phi0; phi0.reserve(trackCollection->size());
    DataVect cotTheta; cotTheta.reserve(trackCollection->size());
    //Covariance matrix has 15 entries per track
    DataVect covMatrix; covMatrix.reserve(trackCollection->size() * 15 );

    // Now loop over all tracks in the collection
    TrackCollection::const_iterator track;
    for (track=trackCollection->begin(); track!=trackCollection->end(); ++track) {
      /**
       * General track fit info
       */
      id.emplace_back(id.size()); //<! simple counter starting from 0
      chi2.emplace_back((*track)->fitQuality()->chiSquared());
      numDoF.emplace_back((*track)->fitQuality()->numberDoF());
      trackAuthor.emplace_back((*track)->info().trackFitter());

      /**
       * Get truth Information
       */
      TrackRetrieverHelpers::getTruthFromTrack(*track,trackCollection,truthCollection,barcode);

      /**
       * Get Perigee parameters (if available)
       */
      const Trk::Perigee* perigee = TrackRetrieverHelpers::getPerigeeParameters(*track, pt, d0, z0, phi0, cotTheta, covMatrix);

      /**
       * Get number of Pix/SCT/TRT hits
       */
      std::unique_ptr<Trk::TrackSummary> summary = nullptr;
      summary = m_trackSumTool->summary(Gaudi::Hive::currentContext(), **track);

      if(not summary){
	ATH_MSG_DEBUG( "Track summary is NULL " );
	nBLayerHits.emplace_back(0);
	nPixHits.emplace_back(0);
	nSCTHits.emplace_back(0);
	nTRTHits.emplace_back(0);
      }else{
	nBLayerHits.emplace_back(summary->get(Trk::numberOfInnermostPixelLayerHits));
	nPixHits.emplace_back(summary->get(Trk::numberOfPixelHits));
	nSCTHits.emplace_back(summary->get(Trk::numberOfSCTHits));
	nTRTHits.emplace_back(summary->get(Trk::numberOfTRTHits));
      }

      /**
       * Get sorted list of track state on surfaces
       */
      // Vector of interesting TrackStateOnSurfaces
      std::vector<const Trk::TrackStateOnSurface*> TSoSVec = TrackRetrieverHelpers::getTrackStateOnSurfaces(*track,perigee,m_doHitsSorting);

      /**
       * Get polyline information
       */
      // Reserving some space for polyline hits
      polylineX.reserve(polylineX.size()+TSoSVec.size());
      polylineY.reserve(polylineY.size()+TSoSVec.size());
      polylineZ.reserve(polylineZ.size()+TSoSVec.size());

      //And fill them
      TrackRetrieverHelpers::getPolylineFromHits(TSoSVec,polylineX,polylineY,polylineZ,numPolyline);

      /**
       * RIO association and outlier id
       */
      //Reserve some space for resPull and other hit info
      isOutlier.reserve(isOutlier.size()+TSoSVec.size());
      hits.reserve(hits.size()+TSoSVec.size());
      driftSign.reserve(driftSign.size()+TSoSVec.size());
      tsosResLoc1.reserve(tsosResLoc1.size()+TSoSVec.size());
      tsosResLoc2.reserve(tsosResLoc2.size()+TSoSVec.size());
      tsosPullLoc1.reserve(tsosPullLoc1.size()+TSoSVec.size());
      tsosPullLoc2.reserve(tsosPullLoc2.size()+TSoSVec.size());
      tsosDetType.reserve(tsosDetType.size()+TSoSVec.size());

      //Now loop over tracks and fill them
      std::vector< const Trk::TrackStateOnSurface* >::const_iterator TSoSItr = TSoSVec.begin();
      //Count number of hits stored in this loop
      long nHits = 0;
      if (m_doHitsDetails){ // disable only for HeavyIons !
	for (; TSoSItr != TSoSVec.end(); ++TSoSItr){
	  // This produces the full long debug dump for TSoS:
	  ATH_MSG_VERBOSE( (**TSoSItr) );
	  //Get the basic hit information
	  const Trk::RIO_OnTrack* rot = TrackRetrieverHelpers::getBaseInfoFromHit(*TSoSItr, m_idHelper, isOutlier, hits, driftSign, tsosDetType );
	  //tell if this didn't work out
	  if (!rot){
	    ATH_MSG_VERBOSE( "Could not obtain RIO for TSoS of type " << (*TSoSItr)->dumpType() );
	    continue ;
	  }
	  //count this as a hit
	  ++nHits;

	  //if we shell retrieve residuals, also get those
	  if (m_doWriteResiduals){
	    TrackRetrieverHelpers::getResidualPullFromHit( *TSoSItr, rot, m_residualPullCalculator, tsosResLoc1, tsosResLoc2, tsosPullLoc1, tsosPullLoc2);
	  }
	}
      } // end hits details

	//Store number of retrieved hits for which we have retrieved information
      numHits.emplace_back(nHits);

    } // end loop over tracks in collection

      //Now fill everything in a datamap
    DataMap DataMap;
    // Start with mandatory entries
    DataMap["id"] = id;
    DataMap["chi2"] = chi2;
    DataMap["numDoF"] = numDoF;
    DataMap["trackAuthor"] = trackAuthor;
    DataMap["barcode"] = barcode;
    DataMap["numHits"] = numHits;
    DataMap["nBLayerHits"] = nBLayerHits;
    DataMap["nPixHits"] = nPixHits;
    DataMap["nSCTHits"] = nSCTHits;
    DataMap["nTRTHits"] = nTRTHits;
    DataMap["numPolyline"] = numPolyline;

    // if perigee parameters are not available, leave the corresponding subtags empty.
    // This way atlantis knows that such tracks can only be displayed as polylines.
    if (!pt.empty()){
      DataMap["pt"] = pt;
      DataMap["d0"] = d0;
      DataMap["z0"] = z0;
      DataMap["phi0"] = phi0;
      DataMap["cotTheta"] = cotTheta;
      DataMap["covMatrix multiple=\"15\""] = covMatrix;
    }

    // vectors with measurement- or TrackStateOnSurface-wise entries
    if ( !polylineX.empty()){
      std::string numPolyPerTrack = DataType(polylineX.size()/((double)id.size())).toString();
      DataMap["polylineX multiple=\"" + numPolyPerTrack + "\""] = polylineX;
      DataMap["polylineY multiple=\"" + numPolyPerTrack + "\""] = polylineY;
      DataMap["polylineZ multiple=\"" + numPolyPerTrack + "\""] = polylineZ;
    }

    if ( !hits.empty()){
      std::string numHitsPerTrack = DataType(hits.size()/((double)id.size())).toString();
      DataMap["hits multiple=\"" + numHitsPerTrack + "\""] = hits;
      DataMap["isOutlier multiple=\""+numHitsPerTrack+"\""] = isOutlier;
      DataMap["driftSign multiple=\""+numHitsPerTrack+"\""] = driftSign;

      if (m_doWriteResiduals){
	// hits counter in principle not needed anymore:
	DataMap["numTsos"] = numHits;
	DataMap["tsosResLoc1 multiple=\""+numHitsPerTrack+"\""] = tsosResLoc1;
	DataMap["tsosResLoc2 multiple=\""+numHitsPerTrack+"\""] = tsosResLoc2;
	DataMap["tsosPullLoc1 multiple=\""+numHitsPerTrack+"\""] = tsosPullLoc1;
	DataMap["tsosPullLoc2 multiple=\""+numHitsPerTrack+"\""] = tsosPullLoc2;
	DataMap["tsosDetType multiple=\""+numHitsPerTrack+"\""] = tsosDetType;
      }
    }
      
    ATH_MSG_DEBUG(dataTypeName() << " collection " << collectionName << " retrieved with " << id.size() << " entries");

    return DataMap;
  }



  const std::vector<std::string> TrackRetriever::getKeys() {

    ATH_MSG_DEBUG("in getKeys()");

    // Initialize keys with the priority key first and then the other requested keys
    std::vector<std::string> keys = {m_priorityKey};

    if (!m_otherKeys.empty() && !m_doWriteAllCollections) {
      keys.insert(keys.end(), m_otherKeys.begin(), m_otherKeys.end());
    }
    else {
      // If all collections are requested, obtain all available keys from StoreGate
      std::vector<std::string> allKeys;
      //if(m_doWriteAllCollections){
	evtStore()->keys<TrackCollection>(allKeys);
	//}

      // Add keys that are not the priority key and do not add containers with "HLT" in their name if requested, ignore TRTSeed and MuonSlimmedTrackCollections
      for (const std::string& key : allKeys) {
	if (key != m_priorityKey && (key.find("HLT") == std::string::npos || m_doWriteHLT) && (key!="TRTSeeds") && (key !="MuonSlimmedTrackCollection")) {
	  keys.emplace_back(key);
	  ATH_MSG_DEBUG("key: " << key);
	}
      }
    }
    return keys;
  }
  
} //namespace JiveXML
