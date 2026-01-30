/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file InDetSimEventTPCnv/test/SiHitCollectionCnv_p4_test.cxx
 * @brief Tests for SiHitCollectionCnv_p4.
 */


#undef NDEBUG
#include "InDetSimEventTPCnv/InDetHits/SiHitCollectionCnv_p4.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include <cassert>
#include <iostream>

#include "TruthUtils/MagicNumbers.h"
#include "GeneratorObjectsTPCnv/initMcEventCollection.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/Operators.h"


void compare (const HepMcParticleLink& p1,
              const HepMcParticleLink& p2)
{
  assert ( p1.isValid() == p2.isValid() );
  assert ( HepMC::barcode(p1) == HepMC::barcode(p2) );
  assert ( p1.id() == p2.id() );
  assert ( p1.eventIndex() == p2.eventIndex() );
  assert ( p1.getTruthSuppressionTypeAsChar() == p2.getTruthSuppressionTypeAsChar() );
  assert ( p1.cptr() == p2.cptr() );
  assert ( p1 == p2 );
}

void compare (const SiHit& p1,
              const SiHit& p2)
{
  assert (p1.localStartPosition() == p2.localStartPosition());
  assert (p1.localEndPosition() == p2.localEndPosition());
  assert (p1.energyLoss() == p2.energyLoss());
  assert (p1.meanTime() == p2.meanTime());
  compare(p1.particleLink(), p2.particleLink());
  assert (p1.particleLink() == p2.particleLink());
  assert (p1.identify() == p2.identify());
}


void compare (const SiHitCollection& p1,
              const SiHitCollection& p2)
{
  //assert (p1.Name() == p2.Name());
  assert (p1.size() == p2.size());
  for (size_t i = 0; i < p1.size(); i++)
    compare (p1[i], p2[i]);
}

void checkPersistentVersion(const SiHitCollection_p4& pers, const SiHitCollection& trans)
{
  //  1 element per string
  constexpr int numberOfStrings{21}; // The number of groups of hits caused by consecutive steps of "the same particle"
  assert ( numberOfStrings == pers.m_hit1_meanTime.size());
  assert ( numberOfStrings == pers.m_hit1_meanTime.size());
  assert ( numberOfStrings == pers.m_hit1_x0.size());
  assert ( numberOfStrings == pers.m_hit1_y0.size());
  assert ( numberOfStrings == pers.m_hit1_z0.size());
  assert ( numberOfStrings == pers.m_hit1_theta.size());
  assert ( numberOfStrings == pers.m_hit1_phi.size());
  assert ( numberOfStrings == pers.m_nHits.size());
  //  1 element per hit
  assert (trans.size() == pers.m_hitEne_2b.size());
  assert (trans.size() == pers.m_hitLength_2b.size());
  //  1 element per hit except for first hit in string
  assert (trans.size()-numberOfStrings == pers.m_dTheta.size());
  assert (trans.size()-numberOfStrings == pers.m_dPhi.size());
  //  1 element per hit with  m_hitEne_2b[i] == 2**16
  assert ( 111 == pers.m_hitEne_4b.size());
  //  1 element per hit with  m_hitLength_2b[i] == 2**16
  assert ( 0 == pers.m_hitLength_4b.size());
  constexpr int numberOfUniqueParticles{12};
  // Less than the numberOfStrings as we don't require the start/end
  // positions of consecutive SiHits to match up in this case, so as all
  // delta-ray hits are grouped together they get a single entry
  assert (numberOfUniqueParticles == pers.m_truthID.size());
  assert (numberOfUniqueParticles == pers.m_mcEvtIndex.size());
  assert (numberOfUniqueParticles == pers.m_nTruthID.size());
  constexpr int numberOfIdentifierGroups{21}; // store id once for set of consecutive hits with same identifier
  assert(numberOfIdentifierGroups == pers.m_id.size());
  assert(numberOfIdentifierGroups == pers.m_nId.size());
}

void testit (const SiHitCollection& trans1)
{
  MsgStream log (nullptr, "test");
  SiHitCollectionCnv_p4 cnv;
  SiHitCollection_p4 pers;
  cnv.transToPers (&trans1, &pers, log);
  checkPersistentVersion(pers, trans1);
  SiHitCollection trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE (std::vector<HepMC::GenParticlePtr>& genPartVector)
{
  std::cout << "test1\n";
  auto particle = genPartVector.at(0);
  const int eventNumber = particle->parent_event()->event_number();
  // Create HepMcParticleLink outside of leak check.
  HepMcParticleLink dummyHMPL(HepMC::uniqueID(particle), eventNumber, HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_ID);
  assert(dummyHMPL.cptr()==particle);
  // Create DVL info outside of leak check.
  SiHitCollection dum ("coll");
  Athena_test::Leakcheck check;

  SiHitCollection trans1 ("coll");
  //check behaviour for delta-rays
  {
    for (int i=0; i < 10; i++) {
      const double angle = i*0.2*M_PI;
      HepMcParticleLink deltaRayLink(0, eventNumber, HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE);
      std::vector< HepGeom::Point3D<double> > stepPoints(11);
      for (int j=0; j<11; ++j) {
        const double jd(j);
        const double r(30.+110.*jd);
        stepPoints.emplace_back(r*std::cos(angle),
                                r*std::sin(angle),
                                350.*jd);
      }
      const int o = i*100;
      trans1.Emplace (stepPoints.at(i), stepPoints.at(i+1),
                      16.5+o,
                      17.5+o,
                      deltaRayLink,
                      19+o);
    }
  }
  for (int i=0; i < 10; i++) {
    auto pGenParticle = genPartVector.at(i);
    HepMcParticleLink trkLink(HepMC::uniqueID(pGenParticle),pGenParticle->parent_event()->event_number(), HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_ID);
    const double angle = i*0.2*M_PI;
    // build step points for this particle
    std::vector< HepGeom::Point3D<double> > stepPoints(11);
    for (int j=0; j<11; ++j) {
      const double jd(j);
      const double r(30.+110.*jd);
      stepPoints.emplace_back(r*std::cos(angle),
                              r*std::sin(angle),
                              350.*jd);
    }
    const int o = i*100;
    // Add multiple SiHits per particle
    for (int j=0; j<10; ++j) {
      trans1.Emplace (stepPoints.at(j), //   local start position of the energy deposit
                      stepPoints.at(j+1), //   local end position of the energy deposit
                      16.5+o, //   deposited energy
                      17.5+o, //   time of energy deposition
                      trkLink, //   link to particle which released this energy
                      19+o // SiHitIdentifier (int) - dummy value
                      );
    }

  }

  // HepMcParticleLink pointing at filtered pileup truth
  HepMC::ConstGenParticlePtr pileupParticle = genPartVector.at(12);
  HepMcParticleLink pileupLink(HepMC::uniqueID(pileupParticle),pileupParticle->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM,HepMcParticleLink::IS_ID);
  pileupLink.setTruthSuppressionType(EBC_PU_SUPPRESSED);
  const double angle = 0.2*M_PI;
  std::vector< HepGeom::Point3D<double> > stepPoints(2);
    for (int j=0; j<2; ++j) {
      const double jd(j);
      const double r(30.+110.*jd);
      stepPoints.emplace_back(r*std::cos(angle),
                              r*std::sin(angle),
                              350.*jd);
    }
    trans1.Emplace (stepPoints.at(0), //   local start position of the energy deposit
                    stepPoints.at(1), //   local end position of the energy deposit
                    16.5, //   deposited energy
                    17.5, //   time of energy deposition
                    pileupLink, //   link to particle which released this energy
                    19 // SiHitIdentifier (int) - dummy value
                    );

  testit (trans1);
}

//coverity[root_function]
int main ATLAS_NOT_THREAD_SAFE ()
{
  ISvcLocator* pSvcLoc = nullptr;
  std::vector<HepMC::GenParticlePtr> genPartVector;
  if (!Athena_test::initMcEventCollection(pSvcLoc, genPartVector)) {
    std::cerr << "This test can not be run" << std::endl;
    return 0;
  }

  test1(genPartVector);
  return 0;
}
