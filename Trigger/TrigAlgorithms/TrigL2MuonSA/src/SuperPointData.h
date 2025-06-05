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
  PBFitResult() :
    CHI2(0),
    PCHI2(0),
    NPOI(0),
    ALIN(0),
    BLIN(0)
      {
      	//////segment candidate
      	for (int i=0; i<NCAND; i++){
          SlopeCand[i]     = 0;
          InterceptCand[i] = 0;
          Chi2Cand[i]      = 0;
        }
     };

 public:
  std::array<float, NMEAMX> XILIN{};
  std::array<float, NMEAMX> YILIN{};
  std::array<float, NMEAMX> RILIN{};
  std::array<float, NMEAMX> WILIN{};
  std::array<float, NMEAMX> RESI{};

  float CHI2;
  float PCHI2;
  int   NPOI;
  float ALIN;
  float BLIN;
  std::array<float, NCAND> SlopeCand{};     //segment candidate
  std::array<float, NCAND> InterceptCand{}; //
  std::array<float, NCAND> Chi2Cand{};      //

}; // See description at the bottom


class SuperPoint
{
 public:
  SuperPoint() :
    Npoint(0),
    Ndigi(0),
    R(0),
    Z(0),
    Phim(0),
    Alin(0),
    Blin(0),
    Xor(0),
    Yor(0),
    Chi2(0),
    PChi2(0)
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
  int   Npoint;
  int   Ndigi;
  float R;
  float Z;
  float Phim;
  float Alin;
  float Blin;
  float Xor;
  float Yor;
  float Chi2;
  float PChi2;
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
