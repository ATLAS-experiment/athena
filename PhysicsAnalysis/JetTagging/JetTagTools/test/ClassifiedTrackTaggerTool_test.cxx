/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file JetTagTools/test/ClassifiedTrackTaggerTool_test.cxx
 * @author Katharina Voss <katharina.voss@cern.ch>
 * @date May, 2022
 * @brief Simple unit test for the ClassifiedTrackTaggerTool. Adopted from InDetTrkInJetType_test.cxx (Scott Synder, synder@bnl.gov)
 */

#include "TrkEventPrimitives/TrkObjectCounter.h"
#undef NDEBUG
#include "JetTagTools/ClassifiedTrackTaggerTool.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticle.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "TestTools/initGaudi.h"
#include "TestTools/FLOATassert.h"
#include "CxxUtils/ubsan_suppress.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "AthenaKernel/DummyRCUSvc.h"
#include "TInterpreter.h"
#include <iostream>
#include <cassert>
#include <cmath>


#include "CLHEP/Vector/LorentzVector.h"

using Gaudi::Units::mm;
using Gaudi::Units::MeV;
using Gaudi::Units::GeV;


AmgSymMatrix(5) cov5()
{
  AmgSymMatrix(5)  m;
  m.setIdentity();
  m(0,0) = 1.e-3; //d0
  m(1,1) = 1.e-2; //Z0
  return m;
}


// Copied from TrackParticleCreatorTool.
void setDefiningParameters( xAOD::TrackParticle& tp,
                            const Trk::Perigee& perigee )
{
  tp.setDefiningParameters(perigee.parameters()[Trk::d0],
    perigee.parameters()[Trk::z0],
    perigee.parameters()[Trk::phi0],
    perigee.parameters()[Trk::theta],
    perigee.parameters()[Trk::qOverP]);
  const AmgSymMatrix(5)* covMatrix = perigee.covariance();
  // see https://its.cern.ch/jira/browse/ATLASRECTS-645 for justification to comment out the following line 
  // assert(covMatrix && covMatrix->rows()==5&& covMatrix->cols()==5); 
  std::vector<float> covMatrixVec;
  if( !covMatrix ) ;//ATH_MSG_WARNING("Setting Defining parameters without error matrix");
  else Amg::compress(*covMatrix,covMatrixVec);
  tp.setDefiningParametersCovMatrixVec(covMatrixVec);
  const Amg::Vector3D& surfaceCenter = perigee.associatedSurface().center(); 
  tp.setParametersOrigin(surfaceCenter.x(), surfaceCenter.y(), surfaceCenter.z() );
}


xAOD::TrackParticle makeTP(int i)
{
  std::cout << "[INFO]: Creating test track particle " << i << std::endl;
  float posx=0., posy=0., posz=0.;
  float momx=0., momy=0., momz=0.;
  uint8_t pixelHits=0, sctHits=0;
  float chi2=0; float numDof=1;

  if(i==1) {posx=-1.35; posy=-1.23; posz=60.36; momx=5333.7; momy=-6171.29; momz=379.76; pixelHits=4; sctHits=8; chi2= 2.23064; numDof =5; }
  else if(i==2) {posx=-0.4; posy=-0.5; posz=59.027; momx=5722.6; momy=-5500.7; momz=3772.28; pixelHits=3; sctHits=7; chi2= 6.233; numDof=10; }
  else if(i==3) {posx=-0.3; posy=-0.3; posz=58.9912; momx=14652.3; momy=-15669.5; momz=663.11; pixelHits=3; sctHits=7; chi2 = 11.5077; numDof = 13; }
  else {std::cout << "[ERROR ]: Specified TrackParticle " << i << " cannot be created, no rules provided" << std::endl; }

  Amg::Vector3D pos0 { 0, 0, 0 };
  Amg::Vector3D pos { posx*mm, posy*mm, posz*mm };
  Amg::Vector3D mom { momx*MeV, momy*MeV, momz*MeV };

  Trk::Perigee p (pos, mom, 1, pos0, cov5());

  xAOD::TrackParticle tp;
  tp.makePrivateStore();
  setDefiningParameters (tp, p);
  tp.setFitQuality (chi2, numDof);
  tp.setSummaryValue (pixelHits, xAOD::numberOfPixelHits);
  tp.setSummaryValue (sctHits, xAOD::numberOfSCTHits);

  return tp;
}


void testDefaultTCT (const EventContext& ctx, Analysis::IClassifiedTrackTaggerTool& cttTool)
{
  std::cout << "test CTT with default TCT training\n";
  xAOD::TrackParticle TP1 = makeTP(1);  
  xAOD::TrackParticle TP2 = makeTP(2);  
  xAOD::TrackParticle TP3 = makeTP(3);  


  std::vector<const xAOD::TrackParticle*> v_tp;
  v_tp.push_back(& TP1);
  v_tp.push_back(& TP2);
  v_tp.push_back(& TP3);

  std::cout << "[INFO]: Creating test primary vertex" << std::endl;
  xAOD::Vertex pv;
  pv.makePrivateStore();
  pv.setPosition ({-0.49*mm, -0.50*mm, 59.0*mm});

  std::cout << "[INFO]: Creating test jet LorentzVector" << std::endl;
  TLorentzVector jet(88424.03*MeV, -63613.38*MeV, 41088.09*MeV, 118046.10*MeV); // ca. 108 GeV

  //float predCTTScore = -0.22773;
  float predCTTScore = -0.335793;
  std::cout << "[INFO]: Retrieving CTT score "<< std::endl;
  float cttScore = cttTool.bJetWgts(ctx, v_tp, pv, jet);
  assert(Athena_test::isEqual(cttScore,predCTTScore));

  std::cout << "testDefaultTCT is OK" << std::endl;
}

EventIDBase timestamp (int t)
{
  return EventIDBase (EventIDBase::UNDEFNUM,  // run
                      EventIDBase::UNDEFEVT,  // event
                      t);
}


EventIDBase runlbn (int run,
                    int lbn)
{
  return EventIDBase (run,  // run
                      EventIDBase::UNDEFEVT,  // event
                      EventIDBase::UNDEFNUM,  // timestamp
                      0,                      // ns offset
                      lbn);
}


int main()
{
  std::cout << "JetTagTools/ClassifiedTrackTaggerTool_test\n";
  CxxUtils::ubsan_suppress ([]() { TInterpreter::Instance(); });
  ISvcLocator* svcloc = nullptr;
  if (!Athena_test::initGaudi("JetTagTools/ClassifiedTrackTaggerTool_tests.txt", svcloc)) {
    return 1;
  }

  
  std::cout << "JetTagTools/ClassifiedTrackTaggerTool initGaudi \n";

  
  EventContext ctx;
  ctx.setExtension (Atlas::ExtendedEventContext());
  EventIDBase eid (1, 0, 0, 0, 20);
  ctx.setEventID (eid);

  Gaudi::Hive::setCurrentContext(ctx);
  
  Athena_test::DummyRCUSvc rcu;
  DataObjID id ("fieldCondObj");
  auto cc = std::make_unique<CondCont<AtlasFieldCacheCondObj> > (rcu, id);
  const EventIDRange range (runlbn (1, 10), runlbn (1, 100));
  assert( cc->insert (range, std::make_unique<AtlasFieldCacheCondObj>(), ctx).isSuccess() );

  ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "test");
  assert( conditionStore->record (std::move (cc), "fieldCondObj") );

    
  std::cout << "JetTagTools/ClassifiedTrackTaggerTool_test record \n";


  ToolHandle<Analysis::IClassifiedTrackTaggerTool> cttTool ("Analysis::ClassifiedTrackTaggerTool");
  assert( cttTool.retrieve().isSuccess() );
  
  
  std::cout << "JetTagTools/ClassifiedTrackTaggerTool_test retrieve tool \n";


  testDefaultTCT (ctx, *cttTool);

  return 0;
}
