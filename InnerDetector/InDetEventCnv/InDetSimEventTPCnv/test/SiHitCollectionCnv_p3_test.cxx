/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file InDetSimEventTPCnv/test/SiHitCollectionCnv_p3_test.cxx
 * @date Feb, 2018
 * @brief Tests for SiHitCollectionCnv_p3.
 */


#undef NDEBUG
#include "InDetSimEventTPCnv/InDetHits/SiHitCollectionCnv_p3.h"
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
  assert ( p1.cptr() == p2.cptr() );
  assert ( p1 == p2 );
}


void compareWithoutPos (const SiHit& p1,
              const SiHit& p2)
{
  assert (p1.energyLoss() == p2.energyLoss());
  assert (p1.meanTime() == p2.meanTime());
  compare(p1.particleLink(), p2.particleLink());
  assert (p1.particleLink() == p2.particleLink());
  assert (p1.identify() == p2.identify());
}


void compare (const SiHit& p1,
              const SiHit& p2)
{
  assert (p1.localStartPosition() == p2.localStartPosition());
  assert (p1.localEndPosition() == p2.localEndPosition());
  compareWithoutPos(p1,p2);
}


void compareWithoutPos (const SiHitCollection& p1,
              const SiHitCollection& p2)
{
  assert (p1.size() == p2.size());
  for (size_t i = 0; i < p1.size(); i++)
    compareWithoutPos (p1[i], p2[i]);
}


void compare (const SiHitCollection& p1,
              const SiHitCollection& p2)
{
  assert (p1.size() == p2.size());
  for (size_t i = 0; i < p1.size(); i++)
    compare (p1[i], p2[i]);
}


void compare (const SiHitCollection_p3& p1,
              const SiHitCollection_p3& p2)
{
  // The number of groups of hits caused by consecutive steps of "the same particle"
  assert ( p1.m_hit1_meanTime.size() == p2.m_hit1_meanTime.size());
  assert ( p1.m_hit1_meanTime.size() == p2.m_hit1_meanTime.size());
  assert ( p1.m_hit1_x0.size() == p2.m_hit1_x0.size());
  assert ( p1.m_hit1_y0.size() == p2.m_hit1_y0.size());
  assert ( p1.m_hit1_z0.size() == p2.m_hit1_z0.size());
  assert ( p1.m_hit1_theta.size() == p2.m_hit1_theta.size());
  assert ( p1.m_hit1_phi.size() == p2.m_hit1_phi.size());
  assert ( p1.m_nHits.size() == p2.m_nHits.size());
  //  1 element per hit
  assert ( p1.m_hitEne_2b.size() == p2.m_hitEne_2b.size());
  assert ( p1.m_hitLength_2b.size() == p2.m_hitLength_2b.size());
  //  1 element per hit except for first hit in string
  assert ( p1.m_dTheta.size() == p2.m_dTheta.size());
  assert ( p1.m_dPhi.size() == p2.m_dPhi.size());
  //  1 element per hit with  m_hitEne_2b[i] == 2**16
  assert ( p1.m_hitEne_4b.size() == p2.m_hitEne_4b.size());
  //  1 element per hit with  m_hitLength_2b[i] == 2**16
  assert ( p1.m_hitLength_4b.size() == p2.m_hitLength_4b.size());
  // Less than the numberOfStrings as we don't require the start/end
  // positions of consecutive SiHits to match up in this case, so as all
  // delta-ray hits are grouped together they get a single entry
  assert ( p1.m_barcode.size() == p2.m_barcode.size());
  assert ( p1.m_mcEvtIndex.size() == p2.m_mcEvtIndex.size());
  assert ( p1.m_evtColl.size() == p2.m_evtColl.size());
  assert ( p1.m_nBC.size() == p2.m_nBC.size());
  assert ( p1.m_id.size() == p2.m_id.size());
  assert ( p1.m_nId.size() == p2.m_nId.size());
}


void checkPersistentVersion(const SiHitCollection_p3& pers, const SiHitCollection& trans)
{
  constexpr int numberOfStrings{20}; // The number of groups of hits caused by consecutive steps of "the same particle"
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
  assert ( 110 == pers.m_hitEne_4b.size());
  //  1 element per hit with  m_hitLength_2b[i] == 2**16
  assert ( 0 == pers.m_hitLength_4b.size());
  constexpr int numberOfUniqueParticles{11};
  // Less than the numberOfStrings as we don't require the start/end
  // positions of consecutive SiHits to match up in this case, so as all
  // delta-ray hits are grouped together they get a single entry
  assert (numberOfUniqueParticles == pers.m_barcode.size());
  assert (numberOfUniqueParticles == pers.m_mcEvtIndex.size());
  assert (numberOfUniqueParticles == pers.m_evtColl.size());
  assert (numberOfUniqueParticles == pers.m_nBC.size());
  constexpr int numberOfIdentifierGroups{20}; // store id once for set of consecutive hits with same identifier
  assert(numberOfIdentifierGroups == pers.m_id.size());
  assert(numberOfIdentifierGroups == pers.m_nId.size());
}


void testit (const SiHitCollection& trans1)
{
  MsgStream log (nullptr, "test");
  SiHitCollectionCnv_p3 cnv;
  SiHitCollection_p3 pers;
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

  testit (trans1);
}

void persistentTest()
{
  MsgStream log (nullptr, "test");
  SiHitCollectionCnv_p3 cnv;
  SiHitCollection_p3 pers1;
  pers1.m_hit1_meanTime = { 6.34704f, 6.35047f, 5.99725f, 6.00278f, 6.01137f, 6.01423f, 7.53231f, 7.53280f, 7.53601f, 7.53652f, 7.53677f, 7.53710f, 7.53764f, 7.53280f, 7.53360f, 7.53398f, 7.53505f, 7.53523f, 7.53561f, 7.53587f, 7.53677f, 7.53710f, 7.53728f, 7.53750f, 7.53766f, 7.53787f, 7.53805f, 7.53851f, 7.53871f, 6.33193f, 6.33538f, 6.69075f, 6.35443f, 6.35763f, 6.38697f, 6.38966f, 6.49904f, 6.50145f, 6.37501f, 6.37822f, 6.32959f, 6.33299f, 5.92833f, 5.93101f };
  pers1.m_hit1_x0 = { -2.22576f, -2.57258f, 4.67701f, 3.52755f, 2.34020f, 2.16678f, -1.89665f, -1.91604f, -1.10751f, -1.01067f, -0.987451f, -0.961037f, -0.905318f, -1.91604f, -2.00442f, -2.03351f, -1.71213f, -1.73175f, -1.79199f, -1.82721f, -2.00042f, -2.05176f, -2.08916f, -2.14178f, -2.16220f, -2.15978f, -2.17479f, -2.15714f, -2.16289f, 3.01031f, 2.64943f, -2.94242f, 3.92093f, 3.67163f, 4.00926f, 4.07648f, 1.51180f, 1.98511f, 1.34394f, 1.10423f, 4.13133f, 3.79202f, 4.74336f, 4.79294f };
  pers1.m_hit1_y0 = { -3.87132f, -3.95612f, -1.05869f, -2.13017f, -4.38073f, -4.62947f, 3.51055f, 3.56671f, 4.06348f, 4.10756f, 4.11605f, 4.09620f, 4.11041f, 3.56671f, 3.56372f, 3.57023f, 3.46527f, 3.44875f, 3.45378f, 3.47786f, 3.48237f, 3.43904f, 3.41345f, 3.41916f, 3.38980f, 3.34265f, 3.30474f, 3.20937f, 3.16382f, 2.24945f, 2.22388f, -2.43940f, 4.27064f, 4.36607f, 0.935116f, 0.727925f, -3.29336f, -3.13153f, -4.21768f, -4.36860f, 0.0513437f, 0.0536670f, 2.18447f, 2.02587f };
  pers1.m_hit1_z0 = { 0.250000f, 0.250000f, -0.250000f, 0.250000f, 0.0972134f, 0.250000f, 0.250000f, 0.118243f, 0.250000f, 0.156530f, 0.0913337f, 0.00601776f, -0.132894f, 0.118243f, -0.0923615f, -0.194048f, 0.250000f, 0.205992f, 0.122075f, 0.0646511f, -0.108144f, -0.166615f, -0.176535f, -0.193754f, -0.213822f, -0.230477f, -0.210951f, -0.165086f, -0.167853f, 0.250000f, 0.250000f, -0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f };
  pers1.m_hit1_theta = { 2.32229f, 2.31671f, 1.26274f, 1.63077f, 2.43192f, 2.25646f, 2.71796f, 2.37644f, 2.21704f, 2.77915f, 2.77208f, 2.74911f, 2.42038f, 2.67955f, 2.85643f, 2.89548f, 2.61391f, 2.44996f, 2.50264f, 2.39530f, 2.28699f, 1.78631f, 1.88530f, 2.08217f, 1.90992f, 1.12422f, 1.22123f, 1.63098f, 2.32601f, 2.31632f, 2.30928f, 0.472882f, 2.39047f, 2.38395f, 2.64571f, 2.60689f, 2.91657f, 2.83985f, 2.38744f, 2.39000f, 2.32889f, 2.32889f, 2.61230f, 2.71450f };
  pers1.m_hit1_phi = { -3.02529f, -3.03644f, -2.39129f, -2.05627f, -2.73639f, -2.42850f, 1.90322f, 1.22783f, 0.511432f, 0.350519f, -0.644513f, 0.249856f, 0.379676f, -3.00715f, 2.92152f, -2.86509f, -2.44168f, 3.02701f, 2.54189f, 2.93690f, -2.44066f, -2.54145f, 3.03352f, -2.17857f, -1.51935f, -1.94799f, -1.28715f, -1.69644f, -2.24203f, -3.10503f, -3.11512f, 2.37757f, 2.98954f, 3.01115f, -2.58535f, -2.59053f, 1.21904f, 0.988734f, -2.90213f, -2.92996f, 3.13845f, 3.13846f, -2.72741f, -2.69915f };
  pers1.m_nHits = { 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 3, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
  pers1.m_hitEne_2b = { 43443, 36918, 65535, 65535, 50942, 27863, 7429, 8778, 25769, 3562, 3911, 4862, 10029, 7271, 3396, 3433, 234, 4268, 13302, 4817, 2413, 1602, 2896, 2325, 4608, 4145, 7250, 4843, 3136, 4429, 2115, 3066, 3011, 4133, 4900, 4642, 12882, 30095, 9059, 25457, 43260, 39591, 31241, 27726, 28960, 50152, 29929, 65535, 64600, 41308, 29893, 25893, 24465 };
  pers1.m_hitLength_2b = { 65535, 65535, 65535, 65535, 45772, 59739, 14929, 14453, 51054, 7608, 6634, 6973, 9149, 15034, 6934, 7212, 838, 10073, 12822, 10597, 5769, 5094, 5224, 5144, 7154, 9717, 14889, 8906, 4639, 5566, 4101, 5007, 4521, 5656, 5174, 4599, 3756, 65535, 20873, 53316, 25884, 65535, 65535, 56847, 58111, 51293, 52366, 65535, 65535, 65535, 65535, 57926, 54935 };
  pers1.m_dTheta =  { 59549, 48281, 41665, 43515, 43958, 46284, 25283, 13967, 32916 };
  pers1.m_dPhi = { 39630, 13239, 23234, 40681, 12440, 39723, 61408, 10671, 32496 };
  pers1.m_hitEne_4b = { 0.820985f, 1.33424f, 0.717816f };
  pers1.m_hitLength_4b = { 0.732353f, 0.736768f, 1.64904f, 2.54915f, 0.737079f, 0.684066f, 0.688273f, 0.686010f, 0.684366f, 0.727235f, 0.727228f };
  pers1.m_barcode = { 0, 4, 0 };
  pers1.m_mcEvtIndex = { 0, 0, 0 };
  pers1.m_evtColl = { 'a', 'a', 'a' };
  pers1.m_nBC = { 49, 2, 2 };
  pers1.m_id = { 218152, 217128, 218248, 217224, 218312, 217288, 16995528, 16994504, 218184, 217160, 218152, 218216, 217192, 218184, 217160, 218216, 217192, 218184, 217160, 87048, 86024, 218344, 217320 };
  pers1.m_nId = { 1, 1, 3, 2, 2, 8, 4, 16, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

  SiHitCollection trans1;
  cnv.persToTrans (&pers1, &trans1, log);
  SiHitCollection_p3 pers2;
  cnv.transToPers (&trans1, &pers2, log);
  compare (pers1, pers2);
  SiHitCollection trans2;
  cnv.persToTrans (&pers2, &trans2, log);
  compareWithoutPos (trans1, trans2);
}

int main ATLAS_NOT_THREAD_SAFE ()
{
  ISvcLocator* pSvcLoc = nullptr;
  std::vector<HepMC::GenParticlePtr> genPartVector;
  if (!Athena_test::initMcEventCollection(pSvcLoc, genPartVector)) {
    std::cerr << "This test can not be run" << std::endl;
    return 0;
  }

  test1(genPartVector);
  persistentTest();
  return 0;
}
