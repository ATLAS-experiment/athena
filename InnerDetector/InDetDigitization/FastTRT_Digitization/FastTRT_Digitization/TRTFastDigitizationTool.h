/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FASTTRT_DIGITIZATION_FASTTRT_DIGITIZATIONTOOL_H
#define FASTTRT_DIGITIZATION_FASTTRT_DIGITIZATIONTOOL_H
/** @file TRTFastDigitizationTool.h
 * @brief a sample implementation of IPileUpTool to test the framework
 * @author Paolo Calafiura - ATLAS Collaboration
 */

#include "PileUpTools/PileUpToolBase.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "InDetPrepRawData/TRT_DriftCircleContainer.h"
#include "TRT_DriftFunctionTool/ITRT_DriftFunctionTool.h"
#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h"
#include "TrkToolInterfaces/ITRT_ElectronPidTool.h"

#include "GaudiKernel/RndmGenerators.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include "TrkParameters/TrackParameters.h"

#include "Identifier/Identifier.h"
#include "CLHEP/Random/RandGauss.h"

#include "TrkTruthData/PRD_MultiTruthCollection.h"

#include "xAODEventInfo/EventInfo.h"

#include "HitManagement/TimedHitCollection.h"
#include "InDetSimEvent/TRTUncompressedHitCollection.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "PileUpTools/PileUpMergeSvc.h"

#include <utility>
#include <vector>
#include <map>
#include <cmath>

class TRT_ID;
class TRTUncompressedHit;

namespace InDetDD {
  class TRT_DetectorManager;
}

class TRT_ID;
class StoreGateSvc;
class ITRT_DriftFunctionTool;


class TRTFastDigitizationTool : public PileUpToolBase {
public:
  TRTFastDigitizationTool( const std::string &type, const std::string &name, const IInterface *parent );

  ///called at the end of the subevts loop. Not (necessarily) able to access
  ///SubEvents
  StatusCode mergeEvent(const EventContext& ctx);

  ///called for each active bunch-crossing to process current SubEvents
  /// bunchXing is in ns
  StatusCode processBunchXing( int bunchXing,
                               SubEventIterator bSubEvents,
                               SubEventIterator eSubEvents );

  /// return false if not interested in  certain xing times (in ns)
  /// implemented by default in PileUpToolBase as FirstXing<=bunchXing<=LastXing
  //  virtual bool toProcess(int bunchXing) const;

  StatusCode prepareEvent(const EventContext& ctx, const unsigned int /*nInputEvents*/ );

  ///alternative interface which uses the PileUpMergeSvc to obtain all
  ///the required SubEvents.
  StatusCode processAllSubEvents(const EventContext& ctx);

  /** Initialize */
  virtual StatusCode initialize();

  /** Finalize */
  StatusCode finalize();

private:

  StatusCode initializeNumericalConstants();    // once per run 
  StatusCode setNumericalConstants();    // once per event (pileup-dependent constants) 

  StatusCode produceDriftCircles(const EventContext& ctx,
                                 CLHEP::HepRandomEngine* rndmEngine,
                                 TimedHitCollection< TRTUncompressedHit >& thpctrt);

  Identifier getIdentifier( int hitID, IdentifierHash &hash, Identifier &layer_id, bool &status ) const;

  StatusCode createAndStoreRIOs(const EventContext& ctx, CLHEP::HepRandomEngine* rndmEngine);

  static double getDriftRadiusFromXYZ( const TimedHitPtr< TRTUncompressedHit > &hit );
  HepGeom::Point3D< double > getGlobalPosition( const TimedHitPtr< TRTUncompressedHit > &hit );
  bool isArgonStraw( const Identifier &straw_id ) const;
  int gasType( const Identifier &straw_id ) const;
  double getProbHT( const EventContext& ctx, int particleEncoding, float kineticEnergy, const Identifier &straw_id, double driftRadiusLoc, double hitGlobalPosition ) const;
  static double HTProbabilityElectron_high_pt( double eta );
  static double HTProbabilityElectron_low_pt( double eta );
  static double HTProbabilityMuon_5_20( double eta );
  static double HTProbabilityMuon_60( double eta );
  static double strawEfficiency( double driftRadius, int BEC = 0 );
  static double correctionHT( double momentum, Trk::ParticleHypothesis hypothesis );
  double particleMass( int i ) const;

  // Tools and Services
  PublicToolHandle< ITRT_DriftFunctionTool > m_trtDriftFunctionTool{this, "TRT_DriftFunctionTool", "TRT_DriftFunctionTool/FatrasTrtDriftFunctionTool"};
  bool m_useTrtElectronPidTool{true};                                           // false: use Tina's parametrization
  PublicToolHandle< Trk::ITRT_ElectronPidTool > m_trtElectronPidTool{this, "TRT_ElectronPidTool", "InDet::TRT_ElectronPidToolRun2/InDetTRT_ElectronPidTool"};
  ToolHandle< ITRT_StrawStatusSummaryTool > m_trtStrawStatusSummaryTool{this, "TRT_StrawStatusSummaryTool", "InDetTRTStrawStatusSummaryTool"}; // Argon / Xenon
  ServiceHandle< PileUpMergeSvc > m_mergeSvc{this, "MergeSvc", "PileUpMergeSvc"};                             // PileUp Merge service
  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};  //!< Random number service
  StringProperty m_randomEngineName{this, "RandomStreamName", "FatrasRnd"};                                         // Name of the random number stream

  // INPUT
  StringProperty m_trtHitCollectionKey{this, "trtHitCollectionName", "TRTUncompressedHits"};
  std::vector< TRTUncompressedHitCollection * > m_trtHitCollList;

  // OUTPUT 
  SG::WriteHandleKey< InDet::TRT_DriftCircleContainer > m_trtDriftCircleContainerKey{this, "trtDriftCircleContainer", "TRT_DriftCircles"};
  SG::WriteHandleKey< PRD_MultiTruthCollection > m_trtPrdTruthKey{this, "trtPrdMultiTruthCollection", "PRD_MultiTruthTRT"};

  TimedHitCollection< TRTUncompressedHit > *m_thpctrt{};
  std::multimap< Identifier, InDet::TRT_DriftCircle * > m_driftCircleMap;

  // Helpers
  const InDetDD::TRT_DetectorManager *m_trt_manager{};
  const TRT_ID *m_trt_id{};                                                 // TRT Id Helper

  // Split configuration
  IntegerProperty m_HardScatterSplittingMode{this, "HardScatterSplittingMode", 0, "Control pileup & signal splitting"};                                         // Process all TRT_Hits or just those from signal or background events
  bool m_HardScatterSplittingSkipper{false};

  BooleanProperty m_useEventInfo{this, "useEventInfo", false};  // get mu from EventInfo ?
  SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey
    { this, "EventInfoKey", "EventInfo", "SG key for EventInfo" };
  FloatProperty m_NCollPerEvent{this, "NCollPerEvent", 30};

  // numerical constants. Might wish to move these to a DB in the future
  double m_trtTailFraction{0.0};            // fraction in tails 
  double m_trtSigmaDriftRadiusTail{0.0};    // sigma of one TRT straw in R
  double m_trtHighProbabilityBoostBkg{1.};
  double m_trtHighProbabilityBoostEle{1.};
  double m_cFit[ 8 ][ 5 ]{};             // efficiency and resolution

};

#endif // FASTTRT_DIGITIZATION_FASTTRT_DIGITIZATIONTOOL_H
