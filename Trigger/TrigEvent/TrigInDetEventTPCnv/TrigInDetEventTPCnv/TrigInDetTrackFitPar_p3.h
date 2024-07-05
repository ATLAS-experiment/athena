/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigInDetEventTPCnv
 * @class  : TrigInDetTrackFitPar_p2
 *
 * @brief persistent partner for TrigInDetTrackFitPar
 *
 * @author Andrew Hamilton  <Andrew.Hamilton@cern.ch>  - U. Geneva
 * @author Francesca Bucci  <f.bucci@cern.ch>          - U. Geneva
 *
 **********************************************************************************/
#ifndef TRIGINDETEVENTTPCNV_TRIGINDETTRACKFITPAR_P3_H
#define TRIGINDETEVENTTPCNV_TRIGINDETTRACKFITPAR_P3_H


class TrigInDetTrackFitPar_p3 {
   
  public:
     TrigInDetTrackFitPar_p3() {}

     // double m_a0;
     // double m_phi0;
     // double m_z0;
     // double m_eta;
     // double m_pT;
     // double m_surfaceCoordinate;
     double m_allDoubles[6]{}; // all the variables above in the same order.
     float m_cov[15]{};
     int m_surfaceType{};  //TrigInDetTrackFitPar::TrigSurfaceType

};


#endif 
