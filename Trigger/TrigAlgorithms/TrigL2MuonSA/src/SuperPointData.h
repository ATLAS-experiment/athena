/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
 #include <array> 

#ifndef  TRIGL2MUONSA_SUPERPOINTDATA_H
#define  TRIGL2MUONSA_SUPERPOINTDATA_H

#define NMEAMX 50
#define MNLINE 14
#define NLAYER 8
#define NCAND  6  //segment candidate
namespace TrigL2MuonSA {

class PBFitResult
{

 public:
  std::array<float, NMEAMX> XILIN{};
  std::array<float, NMEAMX> YILIN{};
  std::array<float, NMEAMX> RILIN{};
  std::array<float, NMEAMX> WILIN{};
  std::array<float, NMEAMX> RESI{};

  float CHI2 {0};
  float PCHI2 {0};
  int   NPOI {0};
  float ALIN {0};
  float BLIN {0};
  std::array<float, NCAND> SlopeCand{};     //segment candidate
  std::array<float, NCAND> InterceptCand{}; //
  std::array<float, NCAND> Chi2Cand{};      //

}; // See description at the bottom


class SuperPoint
{
 public:
  SuperPoint()
  {
    for (int i=0; i<NLAYER; i++) Residual[i]=0;
    //////
    for (int i=0; i<NCAND; i++){
      SlopeCand[i]     = 0;
      InterceptCand[i] = 0;
      Chi2Cand[i]      = 0;
    }
   //////
  };

 public:
  int   Npoint {0};
  int   Ndigi {0};
  float R {0};
  float Z {0};
  float Phim {0};
  float Alin {0};
  float Blin {0};
  float Xor{0};
  float Yor {0};
  float Chi2 {0};
  float PChi2 {0};
  float Residual[NLAYER];
  float SlopeCand[NCAND];     //
  float InterceptCand[NCAND]; //
  float Chi2Cand[NCAND];      //

};

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

}

#endif  // TRIGL2MUONSA_SUPERPOINTDATA_H


/*
      XILIN(j)   : x of point j (for tubes, x of tube center)
      YILIN(j)   : y of point j (for tubes, y of tube center)
      RILIN(j)   : for tubes, drift distance;
      WILIN(j)   : 1/sigma**2 (i.e. the weight of point j);
      RESI(j)    : signed residual (meas - line);
      CHI2       : chi square of the fit;
      PCHI2      : prob (chi2, degrees of freedom);
      NPOI       : totsl number of meas (including not used);
      ALIN       : line coefficient 1 --> y = ALIN * x + BLIN
      BLIN       : line coefficient 2 --> y = ALIN * x + BLIN
      DABLIN(2,2): error matrix
*/
