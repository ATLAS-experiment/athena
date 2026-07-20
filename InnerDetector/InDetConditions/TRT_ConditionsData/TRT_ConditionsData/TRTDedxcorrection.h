/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTDEDXCORRECTION_H
#define TRTDEDXCORRECTION_H


struct TRTDedxcorrection {
   
  static constexpr int nParametersTrackBaseddEdx = 100;
  static constexpr int nParametersHitBaseddEdx = 204;

  double hitOccPar[nParametersHitBaseddEdx] = {1};

  // TrackOccupancy calibration constants are separated in three arrays as we use a polynomial
  // function 2nd order of the form f(x)=a+b*x+c*x^2
  
  // TrackOccupancy calibration constants, HT hits are excluded, Pt>0.4 GeV d0<0.4 mm deltaZ0sin(theta)<0.4 mm
  double trackOccPar0NoHt[nParametersTrackBaseddEdx] = {1};
  double trackOccPar1NoHt[nParametersTrackBaseddEdx] = {1};
  double trackOccPar2NoHt[nParametersTrackBaseddEdx] = {1};

  // TrackOccupancy calibration constants, Pt>0.4 GeV d0<0.4 mm deltaZ0sin(theta)<0.4 mm
  double trackOccPar0[nParametersTrackBaseddEdx] = {1};
  double trackOccPar1[nParametersTrackBaseddEdx] = {1};
  double trackOccPar2[nParametersTrackBaseddEdx] = {1};

  static constexpr int nGasTypes = 3;
  
  double paraDivideByLengthDedxP1[nGasTypes] = {0};
  double paraDivideByLengthDedxP2[nGasTypes] = {0};
  double paraDivideByLengthDedxP3[nGasTypes] = {0};
  double paraDivideByLengthDedxP4[nGasTypes] = {0};
  double paraDivideByLengthDedxP5[nGasTypes] = {0};

  double paraDedxP1[nGasTypes] = {0};
  double paraDedxP2[nGasTypes] = {0};
  double paraDedxP3[nGasTypes] = {0};
  double paraDedxP4[nGasTypes] = {0};
  double paraDedxP5[nGasTypes] = {0};

      
  // resolution depends on the number of hits (and is different for e)
  static constexpr int nParametersResolution = 4; //Polynomial function 3rd used
  double resolution[nGasTypes][nParametersResolution] = {{0}};
  double resolutionElectron[nGasTypes][nParametersResolution] = {{0}};

  // corrections for pile-up (as a function of NVtx linear behavior observed)
  // was in principle also done separately for different detector regions
  // should be checked in more details when high pileup data available

  double normOffsetData[nGasTypes] = {0};  // offset in normalization between data and MC
  double normSlopeTot[nGasTypes] = {0};    // nvtx dependence for ToT
  double normSlopeTotDivideByLength[nGasTypes] = {0};   // nvtx dependence for ToT/L
  double normOffsetTot[nGasTypes] = {0};   // nvtx dependence for ToT
  double normOffsetTotDivideByLength[nGasTypes] = {0};  // nvtx dependence for ToT/L
  int normNzero[nGasTypes] = {0};           // for which average NVtx the fit parameters were determined

  static constexpr int nParametersLongStrawsRZ = 3240;
  static constexpr int nParametersShortStrawsRZ = 216;
  static constexpr int nParametersEndcapRZ = 336;
      
  double paraLongCorrRZ[nGasTypes][nParametersLongStrawsRZ] = {{0}};
  double paraShortCorrRZ[nGasTypes][nParametersShortStrawsRZ] = {{0}};
  double paraEndCorrRZ[nGasTypes][nParametersEndcapRZ] = {{0}};
  double paraLongCorrRZMC[nGasTypes][nParametersLongStrawsRZ] = {{0}};
  double paraShortCorrRZMC[nGasTypes][nParametersShortStrawsRZ] = {{0}};
  double paraEndCorrRZMC[nGasTypes][nParametersEndcapRZ] = {{0}};

  static constexpr int nParametersLongStrawsRZDivideByLength = 630;
  static constexpr int nParametersShortStrawsRZDivideByLength = 63;
  static constexpr int nParametersEndcapRZDivideByLength = 252;
  
  double paraLongCorrRZDivideByLengthMC[nGasTypes][nParametersLongStrawsRZDivideByLength] = {{0}};
  double paraShortCorrRZDivideByLengthMC[nGasTypes][nParametersShortStrawsRZDivideByLength] = {{0}};
  double paraEndCorrRZDivideByLengthMC[nGasTypes][nParametersEndcapRZDivideByLength] = {{0}};
  double paraLongCorrRZDivideByLengthDATA[nGasTypes][nParametersLongStrawsRZDivideByLength] = {{0}};
  double paraShortCorrRZDivideByLengthDATA[nGasTypes][nParametersShortStrawsRZDivideByLength] = {{0}};
  double paraEndCorrRZDivideByLengthDATA[nGasTypes][nParametersEndcapRZDivideByLength] = {{0}};

  static constexpr int nParametersLongStrawsMimic = 1800;
  static constexpr int nParametersShortStrawsMimic = 180;
  static constexpr int nParametersEndcapMimic = 560;

  float paraLongMimicToXeMC[nGasTypes][nParametersLongStrawsMimic] = {{0}};
  float paraLongMimicToXeDATA[nGasTypes][nParametersLongStrawsMimic] = {{0}};
  float paraShortMimicToXeMC[nGasTypes][nParametersShortStrawsMimic] = {{0}};
  float paraShortMimicToXeDATA[nGasTypes][nParametersShortStrawsMimic] = {{0}};
  float paraEndMimicToXeMC[nGasTypes][nParametersEndcapMimic] = {{0}};
  float paraEndMimicToXeDATA[nGasTypes][nParametersEndcapMimic] = {{0}};
 
  //==============================================================  
  

};
CLASS_DEF(TRTDedxcorrection,105466510,1)
CONDCONT_DEF(TRTDedxcorrection,114226988);

#endif  /* TRTDEDXCORRECTION_H */
