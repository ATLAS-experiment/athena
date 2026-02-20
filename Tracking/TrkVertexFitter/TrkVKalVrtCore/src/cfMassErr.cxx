/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//Calculate mass and mass error for any subset of tracks
#include "TrkVKalVrtCore/cfMassErr.h"
#include "TrkVKalVrtCore/CommonPars.h"
#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"
#include <cmath>
#include <array>
#include <cfenv>

namespace Trk {


void cfmasserr(VKVertex * vk, const int *list, double *MASS, double *sigM)
{
  int NTRK=vk->TrackList.size();
  //Deliberately not using make_unique
  std::unique_ptr < double[] > deriv(new double[3*NTRK+3]);
  double ptot[4]={0.};
  std::vector< std::array<double,6> > pmom(NTRK);
  double dm2dpx, dm2dpy, dm2dpz, ee, pt, px, py, pz, cth;

  double fieldPos[3];
  fieldPos[0]=vk->refIterV[0]+vk->fitV[0];
  fieldPos[1]=vk->refIterV[1]+vk->fitV[1];
  fieldPos[2]=vk->refIterV[2]+vk->fitV[2];
  double vBx,vBy,vBz;
  Trk::vkalMagFld::getMagFld(fieldPos[0], fieldPos[1], fieldPos[2],vBx,vBy,vBz,(vk->vk_fitterControl).get());

  for(int it=0; it<NTRK; it++){
    if (list[it]) {
       auto trk=vk->TrackList[it].get();
       double constBF = Trk::vkalMagFld::getEffField(vBx, vBy, vBz, trk->fitP[1], trk->fitP[0]) * vkalMagCnvCst ;
       pmom[it][4] = pt = std::abs( constBF/trk->fitP[2] );
       pmom[it][5] = cth = 1. /tan(trk->fitP[0]);
       pmom[it][0] = px = pt * cos(trk->fitP[1]);
       pmom[it][1] = py = pt * sin(trk->fitP[1]);
       pmom[it][2] = pz = pt * cth;
       double mmm=trk->getMass();
       pmom[it][3] = sqrt(px*px + py*py + pz*pz + mmm*mmm);
       ptot[0] += px;
       ptot[1] += py;
       ptot[2] += pz;
       ptot[3] += pmom[it][3];
    }
  }

  for(int it = 0; it < NTRK; ++it) {
    if (list[it]) {
       pt = pmom[it][4];
       cth= pmom[it][5];
       px = pmom[it][0];
       py = pmom[it][1];
       pz = pmom[it][2];
       ee = pmom[it][3];
       dm2dpx = (ptot[3] / ee * px - ptot[0]) * 2.;
       dm2dpy = (ptot[3] / ee * py - ptot[1]) * 2.;
       dm2dpz = (ptot[3] / ee * pz - ptot[2]) * 2.;
       deriv[it*3 + 0] = dm2dpz * ((-pt) * (cth * cth + 1.));                                /* d(M2)/d(Theta) */
       deriv[it*3 + 1] =  -dm2dpx * py + dm2dpy * px;                                        /* d(M2)/d(Phi)   */
       deriv[it*3 + 2] = (-dm2dpx * px - dm2dpy*py - dm2dpz*pz)/vk->TrackList[it]->fitP[2];  /* d(M2)/d(Rho)   */
    } else {
       deriv[it*3 + 0] = deriv[it*3 + 1] = deriv[it*3 + 2] = 0.;
    }
  }
//----
  double covM2=0.;
  for(int i=0; i<NTRK*3; i++){
     double tmp=0.;
     for(int j=0; j<NTRK*3; j++){
       tmp += ARR2D_FS(vk->ader, vkalNTrkM*3+3, i+3, j+3) *deriv[j];
     }
     tmp *= deriv[i];
     if(std::isnan(tmp)) {tmp=0.;}
     if(std::isinf(tmp)) {tmp=0.;}
     covM2 += tmp;
  }
  std::feclearexcept(FE_ALL_EXCEPT);
  if(covM2<1.e-10)covM2=1.e-10;
//----
  (*MASS) = (ptot[3]-ptot[2])*(ptot[3]+ptot[2])-ptot[1]*ptot[1]-ptot[0]*ptot[0];
  if((*MASS)<1.e-10)  (*MASS) = 1.e-10;
  (*MASS)   = sqrt(*MASS);
  (*sigM) = sqrt(covM2) / 2. / (*MASS);
}



} /* End of namespace */

