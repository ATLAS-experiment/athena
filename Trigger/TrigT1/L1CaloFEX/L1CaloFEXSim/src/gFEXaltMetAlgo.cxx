/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXaltMetAlgo - Noise cut and Rho+RMS algorithm for gFEX MET
//                              -------------------
//     begin                : 31 03 2022
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#include "gFEXaltMetAlgo.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "L1CaloFEXSim/gTower.h"

#include <cmath>
#include <vector>

namespace LVL1 {

gFEXaltMetAlgo::gFEXaltMetAlgo(const std::string& type, const std::string& name, const IInterface* parent):
base_class(type, name, parent) 
{}

StatusCode gFEXaltMetAlgo::initialize(){

  ATH_CHECK(m_l1MenuKey.initialize());

  return StatusCode::SUCCESS;

}

void gFEXaltMetAlgo::altMetAlgo(const EventContext& ctx, const gTowersCentral &Atwr, const gTowersCentral &Btwr, const gTowersCentral &Ctwr,
                                std::array<uint32_t, 4> & outTOB) const {

  // Retrieve the L1 menu configuration
  SG::ReadHandle<TrigConf::L1Menu> l1Menu (m_l1MenuKey, ctx);

  //Parameters related to altMet (noise cut and rho+RMS algorithms)
  const auto & thr_gXE_altMet = l1Menu->thrExtraInfo().gXE();
  int noiseCutThrA = thr_gXE_altMet.noiseCutThr('A');
  int noiseCutThrB = thr_gXE_altMet.noiseCutThr('B');
  int noiseCutThrC = thr_gXE_altMet.noiseCutThr('C');

  std::vector<int> thr_A(12, noiseCutThrA);
  std::vector<int> thr_B(12, noiseCutThrB);
  std::vector<int> thr_C(16, noiseCutThrC);

  std::array<std::vector<int>, 3> etaThr;   // A, B, C
  etaThr[0] = std::move(thr_A);
  etaThr[1] = std::move(thr_B);
  etaThr[2] = std::move(thr_C);

  //FPGA A observables
  int A_MET_x_nc = 0x0;
  int A_MET_y_nc = 0x0;
  int A_MET_x_rms = 0x0;
  int A_MET_y_rms = 0x0;

  int A_sumEt_nc = 0x0;
  int A_sumEt_rms = 0x0;

  //FPGA B observables
  int B_MET_x_nc = 0x0;
  int B_MET_y_nc = 0x0;
  int B_MET_x_rms = 0x0;
  int B_MET_y_rms = 0x0;

  int B_sumEt_nc = 0x0;
  int B_sumEt_rms = 0x0;

  // FPGA C observables
  int C_MET_x_nc = 0x0;
  int C_MET_y_nc = 0x0;
  int C_MET_x_rms = 0x0;
  int C_MET_y_rms = 0x0;
  
  int C_sumEt_nc = 0x0;
  int C_sumEt_rms = 0x0;  

  //Global observables
  int MET_x_nc = 0x0;
  int MET_y_nc = 0x0;
  int MET_nc = 0x0;
  int MET_x_rms = 0x0;
  int MET_y_rms = 0x0;
  int MET_rms = 0x0;


  int total_sumEt_nc = 0x0;
  int total_sumEt_rms = 0x0;

  metFPGA(Atwr, A_MET_x_nc, A_MET_y_nc, 0, etaThr);
  metFPGA(Btwr, B_MET_x_nc, B_MET_y_nc, 1, etaThr);
  metFPGA(Ctwr, C_MET_x_nc, C_MET_y_nc, 2, etaThr);

  metTotal(A_MET_x_nc, A_MET_y_nc, B_MET_x_nc, B_MET_y_nc, C_MET_x_nc, C_MET_y_nc, MET_x_nc, MET_y_nc, MET_nc);

  int A_rho{get_rho(Atwr)};
  int B_rho{get_rho(Btwr)};
  int C_rho{get_rho(Ctwr)};
  int A_sigma{3*get_sigma(Atwr)};
  int B_sigma{3*get_sigma(Btwr)};
  int C_sigma{3*get_sigma(Ctwr)};

  rho_MET(Atwr, A_MET_x_rms, A_MET_y_rms, A_rho, A_sigma);
  rho_MET(Btwr, B_MET_x_rms, B_MET_y_rms, B_rho, B_sigma);
  rho_MET(Ctwr, C_MET_x_rms, C_MET_y_rms, C_rho, C_sigma);

  metTotal(A_MET_x_rms, A_MET_y_rms, B_MET_x_rms, B_MET_y_rms, C_MET_x_rms, C_MET_y_rms, MET_x_rms, MET_y_rms, MET_rms);  

  A_sumEt_nc = sumEtFPGAnc(Atwr, 0, etaThr);
  B_sumEt_nc = sumEtFPGAnc(Btwr, 1, etaThr);
  C_sumEt_nc = sumEtFPGAnc(Ctwr, 2, etaThr);
  total_sumEt_nc = sumEt(A_sumEt_nc, B_sumEt_nc, C_sumEt_nc);
  total_sumEt_nc = total_sumEt_nc/4;

  A_sumEt_rms = sumEtFPGArms(Atwr, A_sigma);
  B_sumEt_rms = sumEtFPGArms(Btwr, B_sigma);
  C_sumEt_rms = sumEtFPGArms(Ctwr, C_sigma);
  total_sumEt_rms = sumEt(A_sumEt_rms, B_sumEt_rms, C_sumEt_rms);
  total_sumEt_rms = total_sumEt_rms/4;
  //Define a vector to be filled with all the TOBs of one event

    //TOB order
    //  1) MET_x | MET_y    <- ncMET
    //  2) MET_x | MET_y    <- rms
    //  3) MET | sumET      <- ncMET
    //  4) MET | sumET      <- rms

  // fill in TOBs
  // The order of the TOBs is given according to the TOB ID (TODO: check how it's done in fw)

  // First TOB is (MET, SumEt)
  outTOB[0] = (MET_y_nc&  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
  outTOB[0] = outTOB[0] | (MET_x_nc  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
  if (MET_y_nc != 0) outTOB[0] = outTOB[0] | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
  if (MET_x_nc != 0) outTOB[0] = outTOB[0] | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
  outTOB[0] = outTOB[0] | (2  &  0x0000001F) << 26;//TOB ID temporary set to 2 according to JwoJ convention (need updates in EDM)

// Second TOB is (MET_x, MET_y)
  outTOB[1] = (MET_y_rms&  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
  outTOB[1] = outTOB[1] | (MET_x_rms  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
  if (MET_y_rms != 0) outTOB[1] = outTOB[1] | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
  if (MET_x_rms != 0) outTOB[1] = outTOB[1] | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
  outTOB[1] = outTOB[1] | (2  &  0x0000001F) << 26;//TOB ID temporary set to 2 according to JwoJ convention (need updates in EDM)

// Third TOB is hard components (MHT_x, MHT_y)
  outTOB[2] = (total_sumEt_nc&  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
  outTOB[2] = outTOB[2] | (MET_nc  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
  if (total_sumEt_nc != 0) outTOB[2] = outTOB[2] | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
  if (MET_nc != 0) outTOB[2] = outTOB[2] | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
  outTOB[2] = outTOB[2] | (1  &  0x0000001F) << 26;//TOB ID temporary set to 1 according to JwoJ convention (need updates in EDM)

  // Fourth TOB is hard components (MST_x, MST_y)
  outTOB[3] = (total_sumEt_rms&  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
  outTOB[3] = outTOB[3] | (MET_rms  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
  if (total_sumEt_rms != 0) outTOB[3] = outTOB[3] | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
  if (MET_rms != 0) outTOB[3] = outTOB[3] | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
  outTOB[3] = outTOB[3] | (1  &  0x0000001F) << 26;//TOB ID temporary set to 1 according to JwoJ convention (need updates in EDM)


}

 void gFEXaltMetAlgo::metFPGA(const gTowersCentral &twrs, int & MET_x, int & MET_y, const unsigned short FPGA_NO, const std::array<std::vector<int>, 3>& etaThr) const {
    static const int s_cosLUT[32] = {
         31, 30, 29, 26, 22, 17, 12,  6,
          0, -6,-12,-17,-22,-26,-29,-30,
        -31,-30,-29,-26,-22,-17,-12, -6,
          0,  6, 12, 17, 22, 26, 29, 30
    };
    static const int s_sinLUT[32] = {
          0,  6, 12, 17, 22, 26, 29, 30,
         31, 30, 29, 26, 22, 17, 12,  6,
          0, -6,-12,-17,-22,-26,-29,-30,
        -31,-30,-29,-26,-22,-17,-12, -6
    };

    int rows = twrs.size();
    int cols = twrs[0].size();
    
    for (int irow = 0; irow < rows; irow++) {
        int etasum = 0;
        for (int jcolumn = 0; jcolumn < cols; jcolumn++) {
	    int tower_et = twrs[irow][jcolumn] & ~3;  // Clear 2 LSBs
	    int scaled_thr = etaThr[FPGA_NO][jcolumn] * 4; // factor of 4 converts threshold from 800 MeV/count (fw) to 200 MeV/count (sim)
            if (tower_et > scaled_thr) {
                etasum += tower_et;
            }
        }

        MET_x += etasum * s_cosLUT[irow];
        MET_y += etasum * s_sinLUT[irow];
    }

    MET_x >>= 5;
    MET_y >>= 5;

}


inline void gFEXaltMetAlgo::metTotal(const int A_MET_x, const int A_MET_y,
                                     const int B_MET_x, const int B_MET_y,
                                     const int C_MET_x, const int C_MET_y,
                                     int & MET_x, int & MET_y, int & MET) const {

  MET_x = A_MET_x + B_MET_x + C_MET_x;
  MET_y = A_MET_y + B_MET_y + C_MET_y;

  if (MET_x < -0x0007FF) MET_x = -0x0007FF;
  if (MET_y < -0x0007FF) MET_y = -0x0007FF;

  if (MET_x > 0x0007FF) MET_x  = 0x0007FF;
  if (MET_y > 0x0007FF) MET_y  = 0x0007FF;

  int MET2 = MET_x * MET_x + MET_y * MET_y;

  if (MET2 > 0x000FFF) MET = 0x000FFF;
  else if (MET2 < 0) MET = 0x000FFF;
  else MET = std::sqrt(MET2);

}

//Function to calculate rho for the given set of gtowers
int gFEXaltMetAlgo::get_rho(const gTowersCentral &twrs) const {
    const int rows = twrs.size();
    const int cols = twrs[0].size();
    const int n{rows*cols};
    float rho = 0;
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            rho += twrs[i][j] < m_rhoPlusThr ? twrs[i][j] : 0;
        }
    }
    return rho/n;
}

//Function calculates standard deviation of the gtowers
int gFEXaltMetAlgo::get_sigma(const gTowersCentral &twrs) const {

  int rows = twrs.size();
  int cols = twrs[0].size();
  const int n{rows*cols};
  int sigma = 0;
  for(int i = 0; i < rows; ++i) {
    for(int j = 0; j < cols; ++j) {
      const int towers{twrs[i][j]};
      sigma += twrs[i][j] < m_rhoPlusThr ? towers*towers: 0;
    }  
  }    

  return static_cast<int>(std::sqrt(sigma * 1. / n));

}


void gFEXaltMetAlgo::rho_MET(const gTowersCentral &twrs, int & MET_x, int & MET_y, const int rho, const int sigma) const {

    int rows = twrs.size();
    int cols = twrs[0].size();
    for( int irow = 0; irow < rows; irow++ ){
        for(int jcolumn = 0; jcolumn < cols; jcolumn++){
            const int ET_gTower_sub{(twrs[irow][jcolumn] - rho) & 0xFFF};
            const bool filter{ET_gTower_sub > sigma && !(ET_gTower_sub & 0x800)};
            MET_x += filter ? (ET_gTower_sub)*cosLUT(irow, 5) : 0;
            MET_y += filter ? (ET_gTower_sub)*sinLUT(irow, 5) : 0;

        }
    }
}

 int gFEXaltMetAlgo::sumEtFPGAnc(const gTowersCentral &twrs, const unsigned short FPGA_NO, const std::array<std::vector<int>, 3>& etaThr) const {

    int partial_sumEt = 0;
    const int rows = twrs.size();
    const int cols = twrs[0].size();
    for( int irow = 0; irow < rows; irow++ ){
        for(int jcolumn = 0; jcolumn<cols; jcolumn++){
            partial_sumEt += twrs[irow][jcolumn] > etaThr[FPGA_NO][jcolumn] * 4 ? twrs[irow][jcolumn] : 0; // factor of 4 converts threshold from 800 MeV/count (fw) to 200 MeV/count (sim)
        }
    }
    return partial_sumEt;
}

int gFEXaltMetAlgo::sumEtFPGArms(const gTowersCentral &twrs, const int sigma) const {

    int partial_sumEt = 0;
    const int rows = twrs.size();
    const int cols = twrs[0].size();
    for(int i{0}; i < rows; ++i){
        for(int j{0}; j < cols; ++j) {
            partial_sumEt += twrs[i][j] > sigma ? twrs[i][j] : 0;
        }
    }
    return partial_sumEt;
}

int gFEXaltMetAlgo::sumEt(const int A_sumEt, const int B_sumEt, const int C_sumEt) const {
    return A_sumEt + B_sumEt + C_sumEt;
}

//----------------------------------------------------------------------------------
// bitwise simulation of sine LUT in firmware
//----------------------------------------------------------------------------------
float gFEXaltMetAlgo::sinLUT(const unsigned int phiIDX, const unsigned int aw) const
{
  float c = ((float)phiIDX)/std::pow(2,aw);
  float rad = (2*M_PI) *c;
  float rsin = std::sin(rad);
  return rsin;
}

//----------------------------------------------------------------------------------
// bitwise simulation cosine LUT in firmware
//----------------------------------------------------------------------------------
float gFEXaltMetAlgo::cosLUT(const unsigned int phiIDX, const unsigned int aw) const
{
  float c = ((float)phiIDX)/std::pow(2,aw);
  float rad = (2*M_PI) *c;
  float rcos = std::cos(rad);
  return rcos;
}


} // namespace LVL1
