/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_DerivationFrameworkSUSY


#include <boost/test/unit_test.hpp>
#include "../src/PdgConditional.h"
#include <cmath>
#include <array>



using namespace DerivationFramework;
BOOST_AUTO_TEST_SUITE(PdgConditionalTest)
  BOOST_AUTO_TEST_CASE(constructor){
    BOOST_CHECK_NO_THROW(PdgConditional());
    BOOST_CHECK_NO_THROW(PdgConditional(5));
    BOOST_CHECK_NO_THROW(PdgConditional(unsigned(5)));
    BOOST_CHECK_NO_THROW(PdgConditional([](int i)->bool{return i>100;}));
  }
  
  BOOST_AUTO_TEST_CASE(IntMethods){
    PdgConditional p5(5);
    PdgConditional pm5(-5);
    PdgConditional absP5(unsigned(5));
    BOOST_CHECK((p5==5));
    BOOST_CHECK((pm5==-5));  
    BOOST_CHECK((absP5==-5));
    BOOST_CHECK((absP5==5));
  }
  BOOST_AUTO_TEST_CASE(LambdaMethod){
    PdgConditional pl5([](int i)->bool{return i>5;});
    //seems counterintuitive, but allows simple use of find
    BOOST_CHECK((pl5==6));
    BOOST_CHECK((pl5==7));
    BOOST_CHECK((pl5!=4));
  }
  BOOST_AUTO_TEST_CASE(LegacyCodeComparison){
    int ngluino=0;
    int nsquark=0; // (up and down type without bottom/top)
    int nantisquark=0; // (up and down type without bottom/top)

    int nsbottom=0;
    int nstop=0;
    int nsbottom2=0;
    int nstop2=0;
    int nantisbottom=0;
    int nantistop=0;
    int nantisbottom2=0;
    int nantistop2=0;

    int nchi01=0;
    int nchi02=0;
    int nchi03=0;
    int nchi04=0;
    int nch1plus=0;
    int nch2plus=0;
    int nch1minus=0;
    int nch2minus=0;

    // sleptons
    int nsmuonRplus=0;
    int nsmuonRminus=0;
    int nselecRplus=0;
    int nselecRminus=0;

    int nsmuonLplus=0;
    int nsmuonLminus=0;
    int nselecLplus=0;
    int nselecLminus=0;

    int nstau1plus=0;
    int nstau1minus=0;
    int nstau2plus=0;
    int nstau2minus=0;

    // snutrinos
    int nselnuL=0;
    int nsmunuL=0;
    int nstaunuL=0;
    int unknown=0;
    //Classification of the event follows (gg, sq...):
    
    for (int i{};i<38;++i){
      for (int offset:{1000000, 2000000}){
        for (int multiplier:{-1,1}){
          int pdgId = (i+offset) * multiplier;
          if      (std::abs(pdgId)== 1000022) nchi01++;
          else if (std::abs(pdgId)== 1000023) nchi02++;
          else if (std::abs(pdgId)== 1000025) nchi03++;
          else if (std::abs(pdgId)== 1000035) nchi04++;
          else if (pdgId == 1000024) nch1plus++;
          else if (pdgId ==-1000024) nch1minus++;
          else if (pdgId == 1000037) nch2plus++;
          else if (pdgId ==-1000037) nch2minus++;
          else if (pdgId == 1000021) ngluino++;
          else if ((std::abs(pdgId)>1000000 && std::abs(pdgId)<= 1000004) || (std::abs(pdgId)>2000000 && std::abs(pdgId)<=2000004)) {
            if (pdgId>0) nsquark++;
            else nantisquark++;
          }
          else if (pdgId==1000005) nsbottom++;
          else if (pdgId==1000006) nstop++;
          else if (pdgId==2000005) nsbottom2++;
          else if (pdgId==2000006) nstop2++;
          else if (pdgId==-1000005) nantisbottom++;
          else if (pdgId==-1000006) nantistop++;
          else if (pdgId==-2000005) nantisbottom2++;
          else if (pdgId==-2000006) nantistop2++;
          else if (pdgId==2000011) nselecRminus++;
          else if (pdgId==-2000011) nselecRplus++;
          else if (pdgId==1000011) nselecLminus++;
          else if (pdgId==-1000011) nselecLplus++;
          else if (std::abs(pdgId)==1000012) nselnuL++;
          else if (pdgId==2000013) nsmuonRminus++;
          else if (pdgId==-2000013) nsmuonRplus++;
          else if (pdgId==1000013) nsmuonLminus++;
          else if (pdgId==-1000013) nsmuonLplus++;
          else if (std::abs(pdgId)==1000014) nsmunuL++;
          else if (pdgId==1000015) nstau1minus++;
          else if (pdgId==-1000015) nstau1plus++;
          else if (pdgId==2000015) nstau2minus++;
          else if (pdgId==-2000015) nstau2plus++;
          else if (std::abs(pdgId)==1000016) nstaunuL++;
          else unknown++;
        }
      }
    }
    BOOST_TEST_MESSAGE("nchi01 "<<nchi01);
    BOOST_TEST_MESSAGE("nchi02 "<<nchi02);
    BOOST_TEST_MESSAGE("nchi03 "<<nchi03);
    BOOST_TEST_MESSAGE("nchi04 "<<nchi04);
    BOOST_TEST_MESSAGE("nch1plus "<<nch1plus);
    BOOST_TEST_MESSAGE("nch1minus "<<nch1minus);
    BOOST_TEST_MESSAGE("nsquark "<<nsquark);
    BOOST_TEST_MESSAGE("nantisquark "<<nantisquark);
    BOOST_TEST_MESSAGE("unknown "<<unknown);
    //
    std::array<int, 35> particleCountByType{};
    for (int i{};i<38;++i){
      for (int offset:{1000000, 2000000}){
        for (int multiplier:{-1,1}){
          int idx = 34;
          int pdgId = (i+offset) * multiplier;
          if      (std::abs(pdgId)== 1000022) idx=0;
          else if (std::abs(pdgId)== 1000023) idx=1;
          else if (std::abs(pdgId)== 1000025) idx=2;
          else if (std::abs(pdgId)== 1000035) idx=3;
          else if (pdgId == 1000024) idx=4;
          else if (pdgId ==-1000024) idx=5;
          else if (pdgId == 1000037) idx=6;
          else if (pdgId ==-1000037) idx=7;
          else if (pdgId == 1000021) idx=8;
          else if ((std::abs(pdgId)>1000000 && std::abs(pdgId)<= 1000004) || (std::abs(pdgId)>2000000 && std::abs(pdgId)<=2000004)) {
            if (pdgId>0) idx=9;
            else idx=10;
          }
          else if (pdgId==1000005) idx=11;
          else if (pdgId==1000006) idx=12;
          else if (pdgId==2000005) idx=13;
          else if (pdgId==2000006) idx=14;
          else if (pdgId==-1000005) idx=15;
          else if (pdgId==-1000006) idx=16;
          else if (pdgId==-2000005) idx=17;
          else if (pdgId==-2000006) idx=18;
          else if (pdgId==2000011) idx=19;
          else if (pdgId==-2000011) idx=20;
          else if (pdgId==1000011) idx=21;
          else if (pdgId==-1000011) idx=22;
          else if (std::abs(pdgId)==1000012) idx=23;
          else if (pdgId==2000013) idx=24;
          else if (pdgId==-2000013) idx=25;
          else if (pdgId==1000013) idx=26;
          else if (pdgId==-1000013) idx=27;
          else if (std::abs(pdgId)==1000014) idx=28;
          else if (pdgId==1000015) idx=29;
          else if (pdgId==-1000015) idx=30;
          else if (pdgId==2000015) idx=31;
          else if (pdgId==-2000015) idx=32;
          else if (std::abs(pdgId)==1000016) idx=33;
          ++particleCountByType[idx];
        }
      }
    }
    BOOST_TEST_MESSAGE("nchi01 "<<particleCountByType[0]);
    BOOST_TEST_MESSAGE("nchi02 "<<particleCountByType[1]);
    BOOST_TEST_MESSAGE("nchi03 "<<particleCountByType[3]);
    BOOST_TEST_MESSAGE("nchi04 "<<particleCountByType[4]);
    BOOST_TEST_MESSAGE("nch1plus "<<particleCountByType[5]);
    BOOST_TEST_MESSAGE("nch1minus "<<particleCountByType[6]);
    BOOST_TEST_MESSAGE("nsquark "<<particleCountByType[9]);
    BOOST_TEST_MESSAGE("nantisquark "<<particleCountByType[10]);
    BOOST_TEST_MESSAGE("unknown "<<particleCountByType[34]);
    particleCountByType.fill(0);
    enum CountIndices{
      chi01, chi02, chi03, chi04, ch1plus, ch1minus, ch2plus, ch2minus, gluino, squark,
      antisquark, sbottom, stop, sbottom2, stop2, antisbottom, antistop, antisbottom2, 
      antistop2, selectRminus, selectRplus, selectLminus, selectLplus, selnuL, 
      smuonRminus, smuonRplus, smuonLminus, smuonLplus, smunuL, stau1minus, stau1plus,
      stau2minus, stau2plus, staunuL, unattributed, nParticleIndices
    };
    std::array<PdgConditional,34> conditions{
      unsigned(1000022),
      unsigned(1000023),
      unsigned(1000025),
      unsigned(1000035),
      1000024,
      -1000024,
      1000037,
      -1000037,
      1000021,
      PdgConditional([](int pdgId)->bool{return ((std::abs(pdgId)>1000000 && std::abs(pdgId)<= 1000004) || (std::abs(pdgId)>2000000 && std::abs(pdgId)<=2000004)) && (pdgId>0);}),
      PdgConditional([](int pdgId)->bool{return (std::abs(pdgId)>1000000 && std::abs(pdgId)<= 1000004) || (std::abs(pdgId)>2000000 && std::abs(pdgId)<=2000004);}),
      1000005,
      1000006,
      2000005,
      2000006,
      -1000005,
      -1000006,
      -2000005,
      -2000006,
      2000011,
      -2000011,
      1000011,
      -1000011,
      unsigned(1000012),
      2000013,
      -2000013,
      1000013,
      -1000013,
      unsigned(1000014),
      1000015,
      -1000015,
      2000015,
      -2000015,
      unsigned(1000016)
    };
    for (int i{};i<38;++i){
      for (int offset:{1000000, 2000000}){
        for (int multiplier:{-1,1}){
          int pdgId = (i+offset) * multiplier;
          auto it = std::find(conditions.begin(),conditions.end(), pdgId);
          int idx = std::distance(conditions.begin(), it);
          ++particleCountByType[idx];
        }
      }
    }
    BOOST_TEST_MESSAGE("nchi01 "<<particleCountByType[chi01]);
    BOOST_TEST_MESSAGE("nchi02 "<<particleCountByType[chi02]);
    BOOST_TEST_MESSAGE("nchi03 "<<particleCountByType[chi03]);
    BOOST_TEST_MESSAGE("nchi04 "<<particleCountByType[chi04]);
    BOOST_TEST_MESSAGE("nch1plus "<<particleCountByType[ch1plus]);
    BOOST_TEST_MESSAGE("nch1minus "<<particleCountByType[ch1minus]);
    BOOST_TEST_MESSAGE("nsquark "<<particleCountByType[squark]);
    BOOST_TEST_MESSAGE("nantisquark "<<particleCountByType[antisquark]);
    BOOST_TEST_MESSAGE("unknown "<<particleCountByType[unattributed]);
    BOOST_TEST_MESSAGE("nIndices "<<nParticleIndices);
    BOOST_CHECK(true);
  }
  
 
BOOST_AUTO_TEST_SUITE_END()
