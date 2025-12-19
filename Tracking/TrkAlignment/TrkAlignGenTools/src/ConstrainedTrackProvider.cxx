/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkAlignGenTools/ConstrainedTrackProvider.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkMeasurementBase/MeasurementBase.h"


#include "TFile.h"
#include "TH2F.h"


namespace Trk {

  //________________________________________________________________________
  ConstrainedTrackProvider::ConstrainedTrackProvider(const std::string& type,
               const std::string& name,
               const IInterface* parent)
    : AthAlgTool(type,name,parent)
   {
    declareInterface<ITrackCollectionProvider>(this);
  }

  //________________________________________________________________________
  StatusCode ConstrainedTrackProvider::initialize()
  {
    // configure main track fitter
    if(m_trackFitter.retrieve().isFailure()) {
       msg(MSG::FATAL) << "Could not get " << m_trackFitter << endmsg;
       return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Retrieved " << m_trackFitter);


    ATH_MSG_INFO( "init m_useConstraintError " <<m_useConstraintError );

    ATH_MSG_INFO( "Init m_reduceConstraintUncertainty " << m_reduceConstraintUncertainty );

    ATH_MSG_INFO( "Init m_reduceConstraintUncert_z0 " << m_reduceConstraintUncert_z0 );

    if(m_CorrectMomentum){

      m_constraintInputFile_P = new TFile(m_constraintFileName_P.value().c_str(), "read");
      if ( m_constraintInputFile_P->IsZombie() || !(m_constraintInputFile_P->IsOpen()) ) {
        ATH_MSG_FATAL( " Problem reading TFile " << m_constraintFileName_P );
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Opened  file containing the deltaSagitta constraints" << m_constraintFileName_P);
      m_etaphiMap_P = static_cast<TH2F*>(m_constraintInputFile_P->Get(m_constraintHistName_P.value().c_str()));
      if(!m_etaphiMap_P){
        ATH_MSG_FATAL( " Problem getting constraints Hist.  Name " << m_constraintHistName_P );
        m_constraintInputFile_P->Close();
        delete m_constraintInputFile_P;
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Opened constraints histogram " << m_constraintHistName_P);

    }

    if(m_CorrectD0){

      m_constraintInputFile_d0 = new TFile(m_constraintFileName_d0.value().c_str(), "read");
      if ( m_constraintInputFile_d0->IsZombie() || !(m_constraintInputFile_d0->IsOpen()) ) {
        ATH_MSG_FATAL( " Problem reading TFile " << m_constraintFileName_d0 );
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Opened file containing the d0 constraints" << m_constraintFileName_d0);
      m_etaphiMap_d0 = static_cast<TH2F*>(m_constraintInputFile_d0->Get(m_constraintHistName_d0.value().c_str()));
      if(!m_etaphiMap_d0){
        ATH_MSG_FATAL( " Problem getting constraints Hist.  Name " << m_constraintHistName_d0 );
        m_constraintInputFile_d0->Close();
        delete m_constraintInputFile_d0;
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Opened constraints histogram " << m_constraintHistName_d0);

    }

    if(m_CorrectZ0){

      m_constraintInputFile_z0 = new TFile(m_constraintFileName_z0.value().c_str(),"read");
      if ( m_constraintInputFile_z0->IsZombie() || !(m_constraintInputFile_z0->IsOpen()) ) {
        ATH_MSG_FATAL( " Problem reading TFile " << m_constraintFileName_z0 );
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Opened  file containing the z0 constraints" << m_constraintFileName_z0);
      m_etaphiMap_z0 = static_cast<TH2F*>(m_constraintInputFile_z0->Get(m_constraintHistName_z0.value().c_str()));
      if(!m_etaphiMap_z0){
        ATH_MSG_FATAL( " Problem getting constraints Hist.  Name " << m_constraintHistName_z0 );
        m_constraintInputFile_z0->Close();
        delete m_constraintInputFile_z0;
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Opened constraints histogram " << m_constraintHistName_z0);

    }

    ATH_CHECK( m_inputKey.initialize() );
    ATH_CHECK( m_outputKey.initialize() );

    return StatusCode::SUCCESS;
  }





  //________________________________________________________________________
  StatusCode ConstrainedTrackProvider::finalize()
  {
    if(m_constraintInputFile_P){
      m_constraintInputFile_P->Close();
      delete m_constraintInputFile_P;
    }

    if(m_constraintInputFile_d0){
      m_constraintInputFile_d0->Close();
      delete m_constraintInputFile_d0;
    }

    if(m_constraintInputFile_z0){
      m_constraintInputFile_z0->Close();
      delete m_constraintInputFile_z0;
    }

    return StatusCode::SUCCESS;
  }


  StatusCode ConstrainedTrackProvider::trackCollection(const TrackCollection*& finalTracks)
  {
    SG::ReadHandle<TrackCollection> originalTracks(m_inputKey);
    if (!originalTracks.isValid()) {
      ATH_MSG_WARNING(" Can't retrieve " << m_inputKey);
      finalTracks = nullptr; // return empty collection
      return StatusCode::FAILURE;
    }

    SG::WriteHandle<TrackCollection> trackCollection(m_outputKey);
    ATH_CHECK(trackCollection.record(std::make_unique<TrackCollection>()));
    if (!trackCollection.isValid()) {
      ATH_MSG_ERROR("Could not record output TrackCollection " << trackCollection.name() << " to store " << trackCollection.store());
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Input track collection size:  " << originalTracks->size());

    int trackCount = 0;
    for (const Trk::Track* track : *originalTracks) {
      trackCount++;
      ATH_MSG_DEBUG("Dealing with track:  " << trackCount << " / " << originalTracks->size());

      bool acceptedTrack = true;
      if (m_doTrackSelection) acceptedTrack = passTrackSelection( track );

      if( acceptedTrack ){
	ATH_MSG_DEBUG("Track selection OK for track: " << trackCount);
        const Trk::Perigee* measuredPerigee =  track->perigeeParameters();
        if(!measuredPerigee){
          ATH_MSG_DEBUG(" no measuredPerigee");
          continue;
        }

        if(!(measuredPerigee->covariance()) ){
          ATH_MSG_DEBUG(" no measuredPerigee");
          continue;
        }

        const Trk::Surface* surf = dynamic_cast<const Trk::Surface*>( &(measuredPerigee->associatedSurface()) ) ;
        if( !surf ) {
          ATH_MSG_DEBUG("NO surface of the measuredPerigee");
          continue;
        }

        // Get corrections from histograms if they are requested

        double correctedQoverP(0), correctedQoverPError(0);
        if(m_CorrectMomentum){
          getCorrectedValues_P( measuredPerigee, correctedQoverP, correctedQoverPError);
        } else {
          correctedQoverP = measuredPerigee->parameters()[Trk::qOverP];
          correctedQoverPError = 1e6;
        }

        double correctedD0(0), correctedD0Error(0);
        if(m_CorrectD0){
          getCorrectedValues_d0( measuredPerigee, correctedD0, correctedD0Error);
        } else {
          correctedD0 = measuredPerigee->parameters()[Trk::d0];
          correctedD0Error = 1e6;
        }

        double correctedZ0(0), correctedZ0Error(0);
        if(m_CorrectZ0){
          getCorrectedValues_z0( measuredPerigee, correctedZ0, correctedZ0Error);
        } else {
          correctedZ0 = measuredPerigee->parameters()[Trk::z0];
          correctedZ0Error = 1e6;
        }

        // Construct pseudomeasurement
        Trk::DefinedParameter constrainedD0( correctedD0     , Trk::d0) ;
        Trk::DefinedParameter constrainedZ0( correctedZ0     , Trk::z0) ;
        Trk::DefinedParameter constrainedqOverP( correctedQoverP , Trk::qOverP) ;

        std::vector<Trk::DefinedParameter> constrainedPars;
        constrainedPars.push_back( constrainedD0 ) ;
        constrainedPars.push_back( constrainedZ0 ) ;
        constrainedPars.push_back( constrainedqOverP ) ;

        Amg::MatrixX constrainedCov( 3,3 ) ;
	      constrainedCov.setZero();
        constrainedCov( 0, 0 )   =  correctedD0Error;
        constrainedCov( 1, 1 )   =  correctedZ0Error;
        constrainedCov( 2, 2 )   =  correctedQoverPError;
        // constrainedCov( 1, 0 )   =  0;  //initially we will assume the constraints are independant
        // constrainedCov( 2, 0 )   =  0;
        // constrainedCov( 2, 1 )   =  0;

        Trk::PseudoMeasurementOnTrack *pmot = new Trk::PseudoMeasurementOnTrack( Trk::LocalParameters( constrainedPars ),
                                                                                 std::move(constrainedCov), *surf) ;


        // Add pseusdo measurement to a list of hits to be fit;
        std::vector<const Trk::MeasurementBase*> vecOfMB;
        if (pmot) vecOfMB.push_back(pmot);

        // Add the remaining hits;
        DataVector<const Trk::MeasurementBase>::const_iterator it    = track->measurementsOnTrack()->begin();
        DataVector<const Trk::MeasurementBase>::const_iterator itend = track->measurementsOnTrack()->end();
        for (;it!=itend;++it)
          vecOfMB.push_back(*it);

        if(m_runOutlierRemoval){
          it    = track->outliersOnTrack()->begin();
          itend = track->outliersOnTrack()->end();
          // Add the remaining hits;
          for (;it!=itend;++it)
            vecOfMB.push_back(*it);
        }

        Trk::Track* constrainedFittedTrack = m_trackFitter->fit( Gaudi::Hive::currentContext(),
                                                                 vecOfMB, *measuredPerigee,
                                                                 m_runOutlierRemoval, Trk::pion).release();
        delete pmot;

        if (constrainedFittedTrack){
          ATH_MSG_DEBUG("Fit was successful");
          const Trk::Perigee* constrainedPerigee =  constrainedFittedTrack->perigeeParameters();
          if(!constrainedPerigee){
            ATH_MSG_DEBUG(" no constrainedmeasuredPerigee");
          } else {
            ATH_MSG_DEBUG("Initial track parameters --> momentum: " << 1/measuredPerigee->parameters()[Trk::qOverP] * 1e-3
			  << "  pt: " << 1/measuredPerigee->parameters()[Trk::qOverP] * 1e-3 * sin(measuredPerigee->parameters()[Trk::theta])
			  << "  d0: " << measuredPerigee->parameters()[Trk::d0]
			  << "  z0: " << measuredPerigee->parameters()[Trk::z0] );
            ATH_MSG_DEBUG("constraint values  -->       momentum: " << 1/correctedQoverP * 1e-3
			  << "  pt: " << 1/correctedQoverP * 1e-3 * sin(measuredPerigee->parameters()[Trk::theta])
			  << "  d0: " << correctedD0
			  << "  z0: " << correctedZ0 );
            ATH_MSG_DEBUG("Constrained fit result -->   momentum: " << 1/constrainedPerigee->parameters()[Trk::qOverP] * 1e-3
			  << "  pt: " << 1/constrainedPerigee->parameters()[Trk::qOverP] * 1e-3 * sin(constrainedPerigee->parameters()[Trk::theta])
			  << "  d0: " << constrainedPerigee->parameters()[Trk::d0]
			  << "  z0: " << constrainedPerigee->parameters()[Trk::z0] );
          }

	  constrainedFittedTrack->setTrackSummary( std::make_unique<Trk::TrackSummary> (*track->trackSummary()) );

          trackCollection->push_back(constrainedFittedTrack);
          ++m_constrainedTracks;
        } else{
          ATH_MSG_DEBUG("Constrained fit was unsuccessful");
        }
      } //--> if ( acceptedTrack)
      else {

        if(m_useConstrainedTrkOnly) continue;

        //Prob could just copy the track?
        Trk::Track* unconstrainedFittedTrack =
          m_trackFitter
            ->fit(Gaudi::Hive::currentContext(),
                  *track,
                  m_runOutlierRemoval,
                  Trk::pion)
            .release();

        if(unconstrainedFittedTrack) {
          ATH_MSG_DEBUG("Unconstrained fit was successful");
          trackCollection->push_back(unconstrainedFittedTrack);
          ++m_unconstrainedTracks;
        } else {
          ATH_MSG_DEBUG("Unconstrained fit was unsuccessful");
        }
      }
    }

    ATH_MSG_INFO ("Size of the trackCollection in this event : " << trackCollection->size() );
    finalTracks = trackCollection.cptr();
    return StatusCode::SUCCESS;

  }




  bool  ConstrainedTrackProvider::passTrackSelection(const Trk::Track *track)
  {
    ATH_MSG_DEBUG("== TrackSelection == STARTED ==");

    const Trk::Perigee* perigee = track->perigeeParameters();
    if(!perigee){
      ATH_MSG_DEBUG("== TrackSelection == NO perigee on this track");
      return false;
    }

    const Trk::TrackSummary* summary = track->trackSummary();

    if(!summary){
      ATH_MSG_DEBUG("== TrackSelection == No summary on this track");
      return false;
    }

    const int nPixHits        = summary->get(Trk::numberOfPixelHits);
    const int nSCTHits        = summary->get(Trk::numberOfSCTHits);
    const int nTRTHits        = summary->get(Trk::numberOfTRTHits);

    const double qoverP = perigee->parameters()[Trk::qOverP] * 1000.;
    const double z0     = perigee->parameters()[Trk::z0];
    const double d0     = perigee->parameters()[Trk::d0];
    double pt          = 0.;
    if ( qoverP != 0 ) pt = std::abs(1.0/qoverP)*sin(perigee->parameters()[Trk::theta]);

    ATH_MSG_DEBUG( "== TrackSelection == track pt  : "<< pt );
    double charge(0);
    if( qoverP > 0)
      charge = 1.;
    else
      charge = -1.;
    ATH_MSG_DEBUG( " pt  : "<< pt << " ; Charge : "<< charge);

    if(pt < m_minPt ||  pt > m_maxPt   ){
      ATH_MSG_DEBUG("== TrackSelection == Track fails pt cut [ "<<m_minPt<<", "<< m_maxPt << "] " << pt );
      return false;
    }

    if (m_SelectByCharge) {
      if (m_SelectPositive){
	if (charge<0){
	  ATH_MSG_DEBUG("Track fails charge cut: negative charge " << charge );
	  return false;
	}
      }
      if (not m_SelectPositive){
	if (charge>0) {
	  ATH_MSG_DEBUG("Track fails charge cut: positive charge " << charge);
	  return false;
	}
      }
    }

    ATH_MSG_VERBOSE("== TrackSelection == Track N PIX hits: " << nPixHits << "  requested: "<< m_minPIXHits );
    ATH_MSG_VERBOSE("== TrackSelection == Track N SCT hits: " << nSCTHits << "  requested: "<< m_minSCTHits );
    ATH_MSG_VERBOSE("== TrackSelection == Track N TRT hits: " << nTRTHits << "  requested: "<< m_minTRTHits );
    ATH_MSG_VERBOSE("== TrackSelection == Track d0: " << d0 << "  requested: "<< m_maxd0 );
    ATH_MSG_VERBOSE("== TrackSelection == Track z0: " << z0 << "  requested: "<< m_maxz0 );

    if( nPixHits < m_minPIXHits ||
        nSCTHits < m_minSCTHits ||
        nTRTHits < m_minTRTHits ||
        std::abs(d0) > m_maxd0    ||
        std::abs(z0) > m_maxz0    ) {
      ATH_MSG_DEBUG("This track did not pass cuts --- nPixHits: " << nPixHits << " nSCTHits: " << nSCTHits <<
		    " nTRTHits: " << nTRTHits << " idd0atIP: " << d0 << " idz0atIP: " << z0  );
     return false;
    }

    return true;
  }

  void  ConstrainedTrackProvider::getCorrectedValues_P(const Trk::Perigee* measuredPerigee, double& correctedQoverP,double& correctedQoverPError)
  {

    // scale q/p not p

    double charge(0);
    if( measuredPerigee->parameters()[Trk::qOverP] > 0)
      charge = 1.;
    else
      charge = -1.;

    double pt   = std::abs(1./(measuredPerigee->parameters()[Trk::qOverP]))* sin(measuredPerigee->parameters()[Trk::theta]) *1e-3;
    double eta = -log(tan(measuredPerigee->parameters()[Trk::theta]/2.));
    double phi = measuredPerigee->parameters()[Trk::phi];
    double perr =  (*measuredPerigee->covariance())( Trk::qOverP, Trk::qOverP );


    int binNumber = m_etaphiMap_P->FindBin(eta, phi);
    double constraintErr = m_etaphiMap_P->GetBinError(binNumber);

    double delta = m_etaphiMap_P->GetBinContent(binNumber) * m_deltaScaling;

    if (m_scalepmaptogev)
      delta = delta * 0.001;

    correctedQoverP = measuredPerigee->parameters()[Trk::qOverP] * (1.+ charge *pt *delta );
    correctedQoverPError = perr;

    if(m_useConstraintError){
      double qoverpFracError =  perr/pow(measuredPerigee->parameters()[Trk::qOverP], 2);
      qoverpFracError += pow(constraintErr,2);
      correctedQoverPError = qoverpFracError*correctedQoverP*correctedQoverP;
    }
    ATH_MSG_DEBUG("Scaling by 1/m_reduceConstraintUncertainty  " << m_reduceConstraintUncertainty << '\t'<< pow( m_reduceConstraintUncertainty,-2)) ;
    correctedQoverPError = correctedQoverPError * pow( m_reduceConstraintUncertainty,-2);
    ATH_MSG_DEBUG(" == input pt: " << pt << "  deltaSagitta: " << delta << "  final pt: " << sin(measuredPerigee->parameters()[Trk::theta]) * 1e-3/correctedQoverP);
 }


  void  ConstrainedTrackProvider::getCorrectedValues_d0(const Trk::Perigee* measuredPerigee, double& correctedD0, double& correctedD0Error)
  {
    ATH_MSG_DEBUG(" == getCorrectedValues_d0 == STARTED ==");
    // scale d0

    double charge(0);
    if( measuredPerigee->parameters()[Trk::qOverP] > 0)
      charge = 1.;
    else
      charge = -1.;

    double d0  = measuredPerigee->parameters()[Trk::d0];
    double eta = -log(tan(measuredPerigee->parameters()[Trk::theta]/2.));
    double phi = measuredPerigee->parameters()[Trk::phi];
    double d0err =  (*measuredPerigee->covariance())( Trk::d0, Trk::d0 );


    int binNumber = m_etaphiMap_d0->FindBin(eta, phi);
    double constraintErr = m_etaphiMap_d0->GetBinError(binNumber);
    double delta = m_etaphiMap_d0->GetBinContent(binNumber) * m_deltaScaling;

    correctedD0 = d0 - charge * delta * 0.5 ; // delta d0 = d0_pos - d0_neg . To make the difference null, substract half to the pos and add half to the neg track.
    if (m_CorrectMeanD0) {correctedD0 = d0 + delta;}
    correctedD0Error = d0err;
    ATH_MSG_DEBUG("Correcting d0 by delta =  " << delta << "\t, corrected d0 = "<< correctedD0 << "\t, original d0 = "<< d0 ) ;

    if(m_useConstraintError){
      correctedD0Error = d0err + pow( constraintErr, 2 );
    }
    ATH_MSG_DEBUG("Scaling by 1/m_reduceConstraintUncertainty  " << m_reduceConstraintUncertainty << '\t'<< pow( m_reduceConstraintUncertainty,-2)) ;
    correctedD0Error = correctedD0Error * pow( m_reduceConstraintUncertainty,-2);

    ATH_MSG_DEBUG(" == input d0: " << d0 << "  deltad0: " << delta << "  final d0: " << correctedD0 << " +- " << correctedD0Error);
    ATH_MSG_DEBUG(" == getCorrectedValues_d0 == Completed ==");
 }

  void  ConstrainedTrackProvider::getCorrectedValues_z0(const Trk::Perigee* measuredPerigee, double& correctedZ0,double& correctedZ0Error)
  {

    // scale z0

    double charge(0);
    if( measuredPerigee->parameters()[Trk::qOverP] > 0)
      charge = 1.;
    else
      charge = -1.;

    double z0  = measuredPerigee->parameters()[Trk::z0];
    double eta = -log(tan(measuredPerigee->parameters()[Trk::theta]/2.));
    double phi = measuredPerigee->parameters()[Trk::phi];
    double z0err =  (*measuredPerigee->covariance())( Trk::z0, Trk::z0 );

    int binNumber = m_etaphiMap_z0->FindBin(eta, phi);
    double constraintErr = m_etaphiMap_z0->GetBinError(binNumber);
    double delta = m_etaphiMap_z0->GetBinContent(binNumber) * m_deltaScaling;

    correctedZ0 = z0 - charge * delta * 0.5 ; // delta z0 = z0_pos - z0_neg . To make the difference null, substract half to the pos and add half to the neg track.
    correctedZ0Error = z0err;

    if(m_useConstraintError){
      correctedZ0Error = z0err + pow( constraintErr, 2 );
    }
    ATH_MSG_DEBUG("Scaling by 1/m_reduceConstraintUncertainty  " << m_reduceConstraintUncertainty << '\t'<< pow( m_reduceConstraintUncertainty,-2)) ;
    correctedZ0Error = correctedZ0Error * pow( m_reduceConstraintUncertainty,-2);
    ATH_MSG_DEBUG("Scaling by 1/m_reduceConstraintUncert_z0  " << m_reduceConstraintUncert_z0 << '\t'<< pow( m_reduceConstraintUncert_z0,-2)) ;
    correctedZ0Error = correctedZ0Error * pow( m_reduceConstraintUncert_z0,-2);
    ATH_MSG_DEBUG(" == input z0: " << z0 << "  deltaz0: " << delta << "  final z0: " << correctedZ0 << " +- " << correctedZ0Error);
 }





  void ConstrainedTrackProvider::printSummary(){

    if(m_logStream) {

      *m_logStream<<"*************************************************************"<<std::endl;
      *m_logStream<<"******        ConstrainedTrackProvider Summary       ******"<<std::endl;
      *m_logStream<<"*"<<std::endl;
      *m_logStream<<"* number of combined muons failed in refit:                           " << m_constrainedTracks   << std::endl;
      *m_logStream<<"* number of combined muons succeeded in refit:                        " << m_unconstrainedTracks      << std::endl;

      *m_logStream<<"*"<<std::endl;
    }
  }






} // end namespace
