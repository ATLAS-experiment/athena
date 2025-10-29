/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXJwoJAlg - Jets without jets algorithm for gFEX
//                              -------------------
//     begin                : 21 12 2023
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#include <cmath>
#include <vector>

#include "gFEXJwoJAlgo.h"
#include "L1CaloFEXSim/gFEXJwoJTOB.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "L1CaloFEXSim/gTower.h"

namespace LVL1 {

  // default constructor for persistency
gFEXJwoJAlgo::gFEXJwoJAlgo(const std::string& type, const std::string& name, const IInterface* parent):
    AthAlgTool(type, name, parent)
  {
    declareInterface<IgFEXJwoJAlgo>(this);
  }


StatusCode gFEXJwoJAlgo::initialize(){

  return StatusCode::SUCCESS;

}


void gFEXJwoJAlgo::setAlgoConstant(int aFPGA_A, int bFPGA_A,
                                   int aFPGA_B, int bFPGA_B,
                                   int aFPGA_C, int bFPGA_C,
                                   int gXE_seedThrA, int gXE_seedThrB, int gXE_seedThrC) {
  m_aFPGA_A = aFPGA_A;
  m_bFPGA_A = bFPGA_A;
  m_aFPGA_B = aFPGA_B;
  m_bFPGA_B = bFPGA_B;
  m_aFPGA_C = aFPGA_C;
  m_bFPGA_C = bFPGA_C;
  m_gBlockthresholdA = gXE_seedThrA;
  m_gBlockthresholdB = gXE_seedThrB;
  m_gBlockthresholdC = gXE_seedThrC;
}

std::vector<std::unique_ptr<gFEXJwoJTOB>> gFEXJwoJAlgo::jwojAlgo(const gTowersType& Atwr,const gTowersType& Btwr, const gTowersType& Ctwr,
                                                                 std::array<int32_t, 4> & outTOB) const {


  // input towers have 200 MeV LSB
  bool SumETfast = true;

  // find gBlocks
  gTowersType AgBlk;
  gTowersType Ascaled;

  gTowersType BgBlk;
  gTowersType Bscaled;

  gTowersType CgBlk;
  gTowersType Cscaled;

  gTowersType hasSeed;

  gBlockAB(Atwr, AgBlk, hasSeed, m_gBlockthresholdA);
  gBlockAB(Btwr, BgBlk, hasSeed, m_gBlockthresholdB);
  gBlockAB(Ctwr, CgBlk, hasSeed, m_gBlockthresholdC);


  // switch to 10 bit number
  // DMS -- do we eventaully need to check for overflows here? 
  
  for(int irow = 0; irow<FEXAlgoSpaceDefs::ABCrows; irow++){
    for(int jcolumn = 0; jcolumn<FEXAlgoSpaceDefs::ABcolumns; jcolumn++){
      Ascaled[irow][jcolumn] = Atwr[irow][jcolumn] >> 2;
      AgBlk[irow][jcolumn] = AgBlk[irow][jcolumn] >> 2;

      Bscaled[irow][jcolumn] = Btwr[irow][jcolumn] >> 2;
      BgBlk[irow][jcolumn] = BgBlk[irow][jcolumn] >> 2;
      
      Cscaled[irow][jcolumn] = Ctwr[irow][jcolumn] >> 2;
      CgBlk[irow][jcolumn] = CgBlk[irow][jcolumn] >> 2;

    }
  }


 //FPGA A observables
  int A_MHT_x = 0x0;
  int A_MHT_y = 0x0;
  int A_MST_x = 0x0;
  int A_MST_y = 0x0;
  int A_MET_x = 0x0;
  int A_MET_y = 0x0;

  int A_eth = 0x0;
  int A_ets = 0x0;
  int A_etw = 0x0;

  //FPGA B observables
  int B_MHT_x = 0x0;
  int B_MHT_y = 0x0;
  int B_MST_x = 0x0;
  int B_MST_y = 0x0;
  int B_MET_x = 0x0;
  int B_MET_y = 0x0;

  int B_eth = 0x0;
  int B_ets = 0x0;
  int B_etw = 0x0;

  //FPGA C observables
  int C_MHT_x = 0x0;
  int C_MHT_y = 0x0;
  int C_MST_x = 0x0;
  int C_MST_y = 0x0;
  int C_MET_x = 0x0;
  int C_MET_y = 0x0;

  int C_eth = 0x0;
  int C_ets = 0x0;
  int C_etw = 0x0;

  //Global observables
  int MHT_x = 0x0;
  int MHT_y = 0x0;
  int MST_x = 0x0;
  int MST_y = 0x0;
  int MET_x = 0x0;
  int MET_y = 0x0;
 
  int ETH =  0x0; 
  int ETS =  0x0;
  int ETW =  0x0;

  int total_sumEt = 0x0;
  int MET = 0x0; 


  // will need to hard code etFPGA ,a's and b's 
  int etBprime =0;

  metFPGA(0, Ascaled, AgBlk, m_gBlockthresholdA, m_aFPGA_A, m_bFPGA_A, A_MHT_x, A_MHT_y, A_MST_x, A_MST_y, A_MET_x, A_MET_y);
  if (SumETfast) etFastFPGA(0, Ascaled, AgBlk, m_gBlockthresholdA, m_aFPGA_A, etBprime, A_eth, A_ets, A_etw);
  else etFPGA(0, Ascaled, AgBlk, m_gBlockthresholdA, m_aFPGA_A, etBprime, A_eth, A_ets, A_etw);

  metFPGA(1, Bscaled, BgBlk, m_gBlockthresholdB, m_aFPGA_B, m_bFPGA_B, B_MHT_x, B_MHT_y, B_MST_x, B_MST_y, B_MET_x, B_MET_y);
  if (SumETfast) etFastFPGA(1, Bscaled, BgBlk, m_gBlockthresholdB, m_aFPGA_B, etBprime, B_eth, B_ets, B_etw);
  else etFPGA(1, Bscaled, BgBlk, m_gBlockthresholdB, m_aFPGA_B, etBprime, B_eth, B_ets, B_etw);

  metFPGA(2, Cscaled, CgBlk, m_gBlockthresholdC, m_aFPGA_C, m_bFPGA_C, C_MHT_x, C_MHT_y, C_MST_x, C_MST_y, C_MET_x, C_MET_y);
  if (SumETfast) etFastFPGA(2, Cscaled, CgBlk, m_gBlockthresholdC, m_aFPGA_C, etBprime, C_eth, C_ets, C_etw);
  else etFPGA(2, Cscaled, CgBlk, m_gBlockthresholdC, m_aFPGA_C, etBprime, C_eth, C_ets, C_etw);

  metTotal(A_MHT_x, A_MHT_y, B_MHT_x, B_MHT_y, C_MHT_x, C_MHT_y, MHT_x, MHT_y);
  metTotal(A_MST_x, A_MST_y, B_MST_x, B_MST_y, C_MST_x, C_MST_y, MST_x, MST_y);
  metTotal(A_MET_x, A_MET_y, B_MET_x, B_MET_y, C_MET_x, C_MET_y, MET_x, MET_y);

  etTotal(A_eth, B_eth, C_eth, ETH);
  etTotal(A_ets, B_ets, C_ets, ETS);
  etTotal(A_etw, B_etw, C_etw, ETW);
  total_sumEt = ETW;

  // components should all be less than 12 bits at this point with 200 MeV LSB
  int MET2 = MET_x * MET_x + MET_y * MET_y;

  if (MET2 > 0x0FFFFFF) {
	  MET = 0x000FFF;
  } else {
	  // repeat the byte stream converter calculation here -- not what the hardware actually does
	  MET = std::sqrt(MET2); 

	  
          // best guess at current hardware.  Note that this is computed in the bytestream converter	  
	  // take most signficant 12 bits 
	  //int MET12 = MET2 >> 12; 
	  // simulate the look up -- only 6 most signficant bits currently set -- to be checked 
	  //MET = ( (int)(std::sqrt(MET12)) << 6) &  0x00000FC0   ;
  }


  //Define a vector to be filled with all the TOBs of one event
  std::vector<std::unique_ptr<gFEXJwoJTOB>> tobs_v;
  tobs_v.resize(4);


  // fill in TOBs
  // The order of the TOBs is given according to the TOB ID (TODO: check how it's done in fw)

  // First TOB is (MET, SumEt)
  outTOB[0] = (total_sumEt &  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
  outTOB[0] = outTOB[0] | (MET  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
  if (total_sumEt != 0) outTOB[0] = outTOB[0] | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
  if (MET != 0) outTOB[0] = outTOB[0] | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
  outTOB[0] = outTOB[0] | (1  &  0x0000001F) << 26;//TOB ID is 1 for scalar values (5 bits starting at 26)

  // std::cout << "DMS MET " << std::hex << MET  << " total_sumEt "  << total_sumEt <<  std::endl << std::dec;

// Second TOB is (MET_x, MET_y)
  outTOB[1] = (MET_y &  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
  outTOB[1] = outTOB[1] | (MET_x  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
  if (MET_y != 0) outTOB[1] = outTOB[1] | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
  if (MET_x != 0) outTOB[1] = outTOB[1] | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
  outTOB[1] = outTOB[1] | (2  &  0x0000001F) << 26;//TOB ID is 2 for MET_x, MET_y (5 bits starting at 26)

// Third TOB is hard components (MHT_x, MHT_y)
  outTOB[2] = (MHT_y &  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
  outTOB[2] = outTOB[2] | (MHT_x  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
  if (MHT_y != 0) outTOB[2] = outTOB[2] | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
  if (MHT_x != 0) outTOB[2] = outTOB[2] | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
  outTOB[2] = outTOB[2] | (3  &  0x0000001F) << 26;//TOB ID is 3 for hard components (5 bits starting at 26)

  // Fourth TOB is hard components (MST_x, MST_y)
  outTOB[3] = (MST_y &  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
  outTOB[3] = outTOB[3] | (MST_x  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
  if (MST_y != 0) outTOB[3] = outTOB[3] | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
  if (MST_x != 0) outTOB[3] = outTOB[3] | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
  outTOB[3] = outTOB[3] | (4  &  0x0000001F) << 26;//TOB ID is 4 for soft components (5 bits starting at 26)


  tobs_v[0] = std::make_unique<gFEXJwoJTOB>();
  tobs_v[0]->setWord(outTOB[0]);
  tobs_v[0]->setQuantity1(MET);
  tobs_v[0]->setQuantity2(total_sumEt);
  tobs_v[0]->setSaturation(0); //Always 0 for now, need a threshold?
  tobs_v[0]->setTobID(1);
  if( MET != 0 ) tobs_v[0]->setStatus1(1);
  else tobs_v[0]->setStatus1(0);
  if(total_sumEt!= 0) tobs_v[0]->setStatus2(1);
  else tobs_v[0]->setStatus2(0);

  tobs_v[1] = std::make_unique<gFEXJwoJTOB>();
  tobs_v[1]->setWord(outTOB[1]);
  tobs_v[1]->setQuantity1(MET_x);
  tobs_v[1]->setQuantity2(MET_y);
  tobs_v[1]->setSaturation(0); //Always 0 for now, need a threshold?
  tobs_v[1]->setTobID(2);
  if( MET_x != 0 ) tobs_v[1]->setStatus1(1);
  else tobs_v[1]->setStatus1(0);
  if(MET_y!= 0) tobs_v[1]->setStatus2(1);
  else tobs_v[1]->setStatus2(0);

  tobs_v[2] = std::make_unique<gFEXJwoJTOB>();
  tobs_v[2]->setWord(outTOB[2]);
  tobs_v[2]->setQuantity1(MHT_x);
  tobs_v[2]->setQuantity2(MHT_y);
  tobs_v[2]->setSaturation(0); //Always 0 for now, need a threshold?
  tobs_v[2]->setTobID(3);
  if( MHT_x != 0 ) tobs_v[2]->setStatus1(1);
  else tobs_v[2]->setStatus1(0);
  if(MHT_y!= 0) tobs_v[2]->setStatus2(1);
  else tobs_v[2]->setStatus2(0);

  tobs_v[3] = std::make_unique<gFEXJwoJTOB>();
  tobs_v[3]->setWord(outTOB[3]);
  tobs_v[3]->setQuantity1(MST_x);
  tobs_v[3]->setQuantity2(MST_y);
  tobs_v[3]->setSaturation(0); //Always 0 for now, need a threshold?
  tobs_v[3]->setTobID(4);
  if( MST_x != 0 ) tobs_v[3]->setStatus1(1);
  else tobs_v[2]->setStatus1(0);
  if(MST_y!= 0) tobs_v[3]->setStatus2(1);
  else tobs_v[3]->setStatus2(0);


  return tobs_v;

}



void gFEXJwoJAlgo::gBlockAB(const gTowersType & twrs, gTowersType & gBlkSum, gTowersType & hasSeed, int seedThreshold) const {

  int rows = twrs.size();
  int cols = twrs[0].size();
  for( int irow = 0; irow < rows; irow++ ){
    for(int jcolumn = 0; jcolumn<cols; jcolumn++){
      // zero jet sum here
      gBlkSum[irow][jcolumn] = 0;
      int krowUp = (irow + 1)%32;
      int krowDn = (irow - 1 +32)%32;
      if( (jcolumn == 0) || (jcolumn == 6) ) {
        //left edge case
        gBlkSum[irow][jcolumn] =
        twrs[irow][jcolumn]   + twrs[krowUp][jcolumn]   + twrs[krowDn][jcolumn] +
        twrs[irow][jcolumn+1] + twrs[krowUp][jcolumn+1] + twrs[krowDn][jcolumn+1];
      } else if( (jcolumn == 5) || (jcolumn == 11)) {
        //  right edge case
        gBlkSum[irow][jcolumn] =
        twrs[irow][jcolumn]   + twrs[krowUp][jcolumn]   + twrs[krowDn][jcolumn] +
        twrs[irow][jcolumn-1] + twrs[krowUp][jcolumn-1] + twrs[krowDn][jcolumn-1];
      } else{
        // normal case
        gBlkSum[irow][jcolumn] =
        twrs[irow][jcolumn]   + twrs[krowUp][jcolumn]   + twrs[krowDn][jcolumn]   +
        twrs[irow][jcolumn-1] + twrs[krowUp][jcolumn-1] + twrs[krowDn][jcolumn-1] +
        twrs[irow][jcolumn+1] + twrs[krowUp][jcolumn+1] + twrs[krowDn][jcolumn+1];
      }

      if( gBlkSum[irow][jcolumn]  > seedThreshold) {
        hasSeed[irow][jcolumn] = 1;
      } else {
        hasSeed[irow][jcolumn] = 0;
      }
    
      if ( gBlkSum[irow][jcolumn] < 0 )       
        gBlkSum[irow][jcolumn] = 0;

      // was bits 11+3 downto 3, now is 11 downto 0 
      if ( gBlkSum[irow][jcolumn] > FEXAlgoSpaceDefs::gBlockMax )   {
        gBlkSum[irow][jcolumn] =  FEXAlgoSpaceDefs::gBlockMax;
      }
    }
  }
}


void gFEXJwoJAlgo::metFPGA(int FPGAnum, const gTowersType& twrs, 
                           const gTowersType & gBlkSum, int gBlockthreshold,
                           int aFPGA, int bFPGA,
                           int & MHT_x, int & MHT_y,
                           int & MST_x, int & MST_y,
                           int & MET_x, int & MET_y) const {
  
  gBlockthreshold = gBlockthreshold * 200 / 800; //gBlockthreshold is provided in counts with a resolution of 200 MeV, but here needs to be applied with a resolution of 800 GeV
  // in the RTL  code these are 19+ 5 = 24 bits 
  int64_t h_tx_hi = 0;
  int64_t h_ty_hi = 0;
  int64_t h_tx_lw = 0;
  int64_t h_ty_lw = 0;
  
  int64_t e_tx_hi = 0;
  int64_t e_ty_hi = 0;
  int64_t e_tx_lw = 0;
  int64_t e_ty_lw = 0;   
  
  for( int irow = 0; irow < FEXAlgoSpaceDefs::ABCrows; irow++ ){
    for(int jcolumn = 6; jcolumn<12; jcolumn++){
      if( FPGAnum == 2){
        int frow = 2*(irow/2)  + 1; 
        
        if(gBlkSum[irow][jcolumn] > gBlockthreshold){
          h_tx_hi += (twrs[irow][jcolumn])*(cosLUT(frow, 5));
          h_ty_hi += (twrs[irow][jcolumn])*(sinLUT(frow, 5));
        } else {
          e_tx_hi += (twrs[irow][jcolumn])*(cosLUT(frow, 5));
          e_ty_hi += (twrs[irow][jcolumn])*(sinLUT(frow, 5));
        }

      } else {
  
        if(gBlkSum[irow][jcolumn] > gBlockthreshold){
          h_tx_hi += (twrs[irow][jcolumn])*(cosLUT(irow, 5));
          h_ty_hi += (twrs[irow][jcolumn])*(sinLUT(irow, 5));
        } else {
          e_tx_hi += (twrs[irow][jcolumn])*(cosLUT(irow, 5));
          e_ty_hi += (twrs[irow][jcolumn])*(sinLUT(irow, 5));
        }
      }
    }
     
    for(int jcolumn = 0; jcolumn<6; jcolumn++){
      if( FPGAnum == 2){
        int frow = 2*(irow/2)  + 1;
        
        if(gBlkSum[irow][jcolumn] > gBlockthreshold){
          h_tx_lw += (twrs[irow][jcolumn])*(cosLUT(frow, 5));
          h_ty_lw += (twrs[irow][jcolumn])*(sinLUT(frow, 5));
        } else{
          e_tx_lw += (twrs[irow][jcolumn])*(cosLUT(frow, 5));
          e_ty_lw += (twrs[irow][jcolumn])*(sinLUT(frow, 5));
        }
      } else {
  
        if(gBlkSum[irow][jcolumn] > gBlockthreshold){
          h_tx_lw += (twrs[irow][jcolumn])*(cosLUT(irow, 5));
          h_ty_lw += (twrs[irow][jcolumn])*(sinLUT(irow, 5));
        } else {
          e_tx_lw += (twrs[irow][jcolumn])*(cosLUT(irow, 5));
          e_ty_lw += (twrs[irow][jcolumn])*(sinLUT(irow, 5));
        }
      }
    }
  }

  // According to https://gitlab.cern.ch/atlas-l1calo/gfex/firmware/-/issues/406#note_5662344
  // there is no division -- so LSB is indeed 25 MeV

  //but later changed to 200 MeV so divide by 8

  long int fMHT_x =  (h_tx_hi + h_tx_lw) ;
  long int fMHT_y =  (h_ty_hi + h_ty_lw) ;
  long int fMST_x =  (e_tx_hi + e_tx_lw) ;
  long int fMST_y =  (e_ty_hi + e_ty_lw) ;
    
  MHT_x =  (h_tx_hi + h_tx_lw) >> 3;
  MHT_y =  (h_ty_hi + h_ty_lw) >> 3;
  MST_x =  (e_tx_hi + e_tx_lw) >> 3;
  MST_y =  (e_ty_hi + e_ty_lw) >> 3;
 
  //  a and b coffecients are 10 bits
  // multiplication has an addtional 2^10
  // constant JWJ_OW        : integer := 35;--Out width
  // values are 35 bits long and top 16 bits are taken -- so divide by 2^19
  //  2^10/2^19 = 1/2^9 = 1/512
   
  long int fMET_x = ( aFPGA * (fMHT_x) + bFPGA * (fMST_x) ) >> 13 ;
  long int fMET_y = ( aFPGA * (fMHT_y) + bFPGA * (fMST_y) ) >> 13 ;

  MET_x  = fMET_x;
  MET_y  = fMET_y;
    
}

void gFEXJwoJAlgo::etFPGA(int FPGAnum, const gTowersType& twrs, gTowersType &gBlkSum,
                          int gBlockthreshold, int A, int B, int &eth, int &ets, int &etw) const {

  gBlockthreshold = gBlockthreshold * 200 / 800; //gBlockthreshold is provided in counts with a resolution of 200 MeV, but here needs to be applied with a resolution of 800 GeV

  int64_t ethard_hi = 0;
  int64_t etsoft_hi = 0;
  int64_t ethard_lo = 0;
  int64_t etsoft_lo = 0;

  int64_t ethard = 0.0;
  int64_t etsoft = 0.0; 
 
  int multiplicitiveFactor = 0;

  if(FPGAnum < 2 ) {
   multiplicitiveFactor = cosLUT(0, 5);
  } else{
    multiplicitiveFactor = cosLUT(1, 5);
  }

// firmware treats upper and lower columns differnetly 

  for( int irow = 0; irow < FEXAlgoSpaceDefs::ABCrows; irow++ ){
    for(int jcolumn = 0; jcolumn<6; jcolumn++){
      	if(gBlkSum[irow][jcolumn] > gBlockthreshold){
	  ethard_lo = ethard_lo + twrs[irow][jcolumn]*multiplicitiveFactor; 
	} else {
	  etsoft_lo = etsoft_lo + twrs[irow][jcolumn]*multiplicitiveFactor; 
	}
    }
  }

  for( int irow = 0; irow < FEXAlgoSpaceDefs::ABCrows; irow++ ){
    for(int jcolumn = 6; jcolumn<12; jcolumn++){
      	if(gBlkSum[irow][jcolumn] > gBlockthreshold){
	  ethard_hi = ethard_hi + twrs[irow][jcolumn]*multiplicitiveFactor; 
	} else {
	  etsoft_hi = etsoft_hi + twrs[irow][jcolumn]*multiplicitiveFactor; 
	}
    }
  }

  ethard = ethard_hi + ethard_lo;
  
  etsoft = etsoft_hi + etsoft_lo;


  int64_t etsum_hi = ethard_hi*A  + etsoft_hi*B  ;
  if ( etsum_hi < 0 ) etsum_hi = 0; 

  int64_t etsum_lo = ethard_lo*A  + etsoft_lo*B  ;
  if ( etsum_lo < 0 ) etsum_lo = 0; 
  
  int64_t etsum = etsum_hi + etsum_lo; 


  // convert 200 MeV LSB here 
  eth  = ethard>>3;
  ets  = etsoft>>3;
  etw  = (etsum  >>13 ) ;

  if( etw < 0 )  etw  = 0;
  // max value is 15 bits with 800 MeV LSB -- so 17 bits here 
  if( etw > 0X001FFFF ) etw  =  0X001FFFF ; 


  if(msgLvl(MSG::DEBUG)) { 
    std::cout << "DMS FPGA gTEJWOJ " << std::hex <<  FPGAnum << "et sum hard " << eth << "etsum soft" << ets << " A " << A << " B " << B << " weighted term " << etw << std::endl << std::dec; 
  }
}

void gFEXJwoJAlgo::etFastFPGA(int FPGAnum, const gTowersType& twrs, gTowersType &gBlkSum,
                          int gBlockthreshold, int A, int B, int &eth, int &ets, int &etw) const {

  gBlockthreshold = gBlockthreshold * 200 / 800; //gBlockthreshold is provided in counts with a resolution of 200 MeV, but here needs to be applied with a resolution of 800 GeV
  int64_t ethard_hi = 0;
  int64_t etsoft_hi = 0;
  int64_t ethard_lo = 0;
  int64_t etsoft_lo = 0;

  int64_t ethard = 0.0;
  int64_t etsoft = 0.0;

  // firmware treats upper and lower columns differently 
  for( int irow = 0; irow < FEXAlgoSpaceDefs::ABCrows; irow++ ){
    for(int jcolumn = 0; jcolumn<6; jcolumn++){
      	if(gBlkSum[irow][jcolumn] > gBlockthreshold){
	          ethard_lo = ethard_lo + twrs[irow][jcolumn]; 
	      }
        else {
            etsoft_lo = etsoft_lo + twrs[irow][jcolumn];
        }
    }
  }

  for( int irow = 0; irow < FEXAlgoSpaceDefs::ABCrows; irow++ ){
    for(int jcolumn = 6; jcolumn<12; jcolumn++){
      	if(gBlkSum[irow][jcolumn] > gBlockthreshold){
	          ethard_hi = ethard_hi + twrs[irow][jcolumn]; 
	      }
        else {
	          etsoft_hi = etsoft_hi + twrs[irow][jcolumn];
        }
    }
  }

  ethard = ethard_hi + ethard_lo;
  etsoft = etsoft_hi + etsoft_lo;

  // convert 200 MeV LSB here 
  eth  = ethard;
  ets  = etsoft; // Keep for reference -- not used in etFast
  etw  = ethard; // For EtFast the weighted term is just the hard term

  // 16 bits signed, set max and min
  if( etw < -32768 ) etw  = -32768;
  if( etw > 32767 ) etw  =  32767;

  if(msgLvl(MSG::DEBUG)) { 
    std::cout << "DMS FPGA gTEJWOJ " << std::hex <<  FPGAnum << "et sum hard " << eth << "etsum soft" << ets << " A " << A << " B " << B << " weighted term " << etw << std::endl << std::dec; 
  }
}

void gFEXJwoJAlgo::metTotal(int A_MET_x, int A_MET_y,
                            int B_MET_x, int B_MET_y,
                            int C_MET_x, int C_MET_y,
                            int & MET_x, int & MET_y) const {

  
  MET_x = A_MET_x + B_MET_x + C_MET_x;
  MET_y = A_MET_y + B_MET_y+ C_MET_y;

  // Truncation of the result, as the individual quantities are 16 bits, while the TOB field is 12 bits
  // MET_x = MET_x >> 4;
  // MET_y = MET_y >> 4;

  if (MET_x < -0x000800) MET_x = -0x000800; //-2048
  if (MET_y < -0x000800) MET_y = -0x000800; //-2048

  if (MET_x > 0x0007FF) MET_x  = 0x0007FF; //2047
  if (MET_y > 0x0007FF) MET_y  = 0x0007FF; //2047

}

void gFEXJwoJAlgo::etTotal(int A_ET, 
                            int B_ET, 
                            int C_ET, 
                            int & ET ) const {

  //  leave at 200 MeV scale 	
  if (A_ET < 0 ) A_ET = 0;
  if (B_ET < 0 ) B_ET = 0;
  if (C_ET < 0 ) C_ET = 0;

  ET = (A_ET + B_ET + C_ET) ; 

  // main value of ET is always positive 
  if( ET > 0x0000FFF) ET =  0x0000FFF;

}

//----------------------------------------------------------------------------------
// bitwise simulation of sine LUT in firmware
//----------------------------------------------------------------------------------
float gFEXJwoJAlgo::sinLUT(unsigned int phiIDX, unsigned int aw) const
{
  float c = static_cast<float>(phiIDX)/std::pow(2,aw);
  float rad = (2*M_PI) *c;
  float rsin = std::sin(rad);
  int isin = std::round(rsin*(std::pow(2,aw) - 1));
  return isin;

}

//----------------------------------------------------------------------------------
// bitwise simulation cosine LUT in firmware
//----------------------------------------------------------------------------------
float gFEXJwoJAlgo::cosLUT(unsigned int phiIDX, unsigned int aw) const
{
  float c = static_cast<float>(phiIDX)/std::pow(2,aw);
  float rad = (2*M_PI) *c;
  float rcos = std::cos(rad);
  int icos = std::round(rcos*(std::pow(2,aw) - 1));
  return icos;
}

} // namespace LVL1
