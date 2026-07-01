/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ReFitTrackWithTruth.h, (c) ATLAS Detector Software
// Take a track and replace the cluster position with the 
// associated truth particle truth position
///////////////////////////////////////////////////////////////////

#ifndef TRKREFITALG_REFITTRACKWITHTRUTH_H
#define TRKREFITALG_REFITTRACKWITHTRUTH_H

// Base class
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/MsgStream.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetSimData/InDetSimDataCollection.h"
#include "InDetSimEvent/SiHitCollection.h"
#include "InDetSimEvent/SiHit.h"
#include "InDetReadoutGeometry/SiDetectorDesign.h"
#include "InDetPrepRawData/PixelCluster.h"
#include "StoreGate/WriteHandle.h"
#include "TrkTrack/Track.h"
#include "TrkFitterUtils/FitterTypes.h"
#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "TrkToolInterfaces/IPRDtoTrackMapTool.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkEventUtils/TrkParametersComparisonFunction.h"
#include "TrkTruthData/TrackTruthCollection.h"

class AtlasDetectorID;
class PixelID;
class TRandom3;

namespace Trk{

class ITrackFitter;
class ITrackSummaryTool;
class ITrackSelectorTool;
class IPRD_AssociationTool;

 /** @brief Algorithm using an instance of a ITrackFitter
     to refit the tracks of a given pseudotrack collection
     after changing the cluster position to the truth particle 
     position
     
     @author Gabriel.Facini@cern.ch, Andrea.Sciandra@cern.ch
     */
 
class ReFitTrackWithTruth : public AthAlgorithm  {

public:

  typedef Trk::Track Track;

  //! standard Algorithm constructor
  ReFitTrackWithTruth(const std::string &name,ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx)    override;

private:

  std::vector<SiHit> matchSiHitsToCluster( const int uniqueIdToMatch,
    const InDet::PixelCluster* pixClus,
    SG::ReadHandle<AtlasHitsVector<SiHit>> &siHitCollection) const;

  static bool IsClusterFromTruth( const InDet::PixelCluster* pixClus,
    const int uniqueIdToMatch,
    const InDetSimDataCollection &sdoCollection) ;

  HepGeom::Point3D<double> smearTruthPosition( const HepGeom::Point3D<double>& orig,
      const int bec,
      const int layer_disk,
      const InDetDD::SiDetectorDesign* design) const;

  double getPhiPosResolution(int layer) const;
  double getEtaPosResolution(int layer) const;
  double getPhiPosErrorFactor(int layer) const;
  double getEtaPosErrorFactor(int layer) const;

  // input/output track container
  SG::ReadHandleKey<TrackCollection> m_inputTrackColName{this,"InputTrackColName","","collection name for tracks to be refitted"};   	// Name of the input Trackcollection
  SG::WriteHandleKey<TrackCollection> m_outputTrackCollectionName{this,"OutputTrackColName","ReFitted_TracksWithTruth","collection name for output tracks"};

  // TRandom3 generator for hit-resolution smearing
  boost::thread_specific_ptr<TRandom3> m_random;                    	//!< smear away!

  // --- fitter steering
  Trk::RunOutlierRemoval          m_runOutlier = false;           	// switch whether to run outlier logics or not
  int                             m_matEffects = 3;            		// type of material interaction in extrapolation

  Gaudi::Property<bool> m_saveWrongHits {this, "SaveWrongHits", false};                                		// If running on Reco Tracks, can have wrong hits
  Gaudi::Property<bool> m_fixWrongHits  {this, "FixWrongHits", false};                             		// If running on Reco Tracks, fix wrong hits
  Gaudi::Property<bool> m_rejNoiseHits  {this, "RejectNoiseHits", false};                             		// If running on Reco Tracks, can have noise hits

  Trk::ParticleHypothesis         m_ParticleHypothesis = Trk::pion;	// nomen est omen 

  // --- job options
  SG::ReadHandleKey<SiHitCollection> m_siHitCollectionName{this,"SiHitCollectionName","PixelHits",""};      		// SiHitCollection storegate key  
  SG::ReadHandleKey<InDetSimDataCollection> m_SDOContainerName{this,"MC_SDOs","PixelSDO_Map"};                 		// SDO Container storegate key  
  SG::ReadHandleKey<TrackTruthCollection> m_truthMapName{this,"TruthMap","InDetTracksTruthCollection"};        		// TruthMap name storegate key

  // smearing
  Gaudi::Property<std::vector<float> > m_resolutionRPhi{this, "ResolutionRPhi", {}};           	// per-layer resolution
  Gaudi::Property<std::vector<float> > m_resolutionZ{this, "ResolutionZ", {}};                 	// per-layer resolution
  Gaudi::Property<std::vector<float> > m_errorRPhi{this, "ErrorFactorRPhi", {}};            	// per-layer error - multiplicative factor * res.
  Gaudi::Property<std::vector<float> > m_errorZ{this, "ErrorFactorZ", {}};                    	// per-layer error - multiplicative factor * res.
  
  // -- algorithm members
  ToolHandle<Trk::ITrackFitter>              m_ITrackFitter
     {this, "FitterTool", "Trk::GlobalChi2Fitter/InDetTrackFitter" };        	//!< the refit tool
  ToolHandle<Trk::IExtendedTrackSummaryTool>  m_trkSummaryTool
     {this, "SummaryTool", "Trk::TrackSummaryTool/InDetTrackSummaryTool" };     	//!< the track summary tool
  ToolHandle<Trk::IPRDtoTrackMapTool>        m_assoTool
     {this, "AssociationTool", "Trk::PRDtoTrackMapTool" };              //!< Tool to create and populate PRD to track 
  const AtlasDetectorID*                     m_idHelper = nullptr;            	//!< Detector ID helper
  const PixelID*                             m_pixelID = nullptr;             	//!< Pixel ID
                                                            
};

} 

#endif //TRKREFITALG_REFITTRACKWITHTRUTH_H

