/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonAlignmentData/MdtAsBuiltPar.h"
#include <iostream>

std::ostream& operator<<(std::ostream& ostr, const MdtAsBuiltPar& par) {
   ostr<<"MdtAsBuilt AMDB id (name,eta,phi,job)=(";
   ostr<<par.AmdbStation()<<",";
   ostr<<par.AmdbEta()<<",";
   ostr<<par.AmdbPhi()<<",";
   ostr<<par.AmdbJob()<<"), \n";
   using multilayer_t = MdtAsBuiltPar::multilayer_t;
   using tubeSide_t = MdtAsBuiltPar::tubeSide_t;  
   for (const multilayer_t ml : {multilayer_t::ML1, multilayer_t::ML2}){
      ostr<<" chamber multi-layer " << static_cast<unsigned int>(ml)<<",";
      for (const tubeSide_t side : {tubeSide_t::POS, tubeSide_t::NEG}){
          ostr << "(y0,z0,alpha,ypitch,zpitch,stagg) at ";
          ostr << (side == tubeSide_t::POS ? "positive" : "negative")<<" side = {";
          ostr << par.y0(ml, side) <<", ";
          ostr << par.z0(ml, side) <<", ";
          ostr << par.alpha(ml, side) <<", ";
          ostr << par.ypitch(ml, side) <<", ";
          ostr << par.zpitch(ml, side) <<", ";
          ostr << par.stagg(ml, side) <<"},  ";
      }
      ostr<<std::endl;
   }
   return ostr;
}

void MdtAsBuiltPar::setAlignmentParameters(multilayer_t iML, tubeSide_t iTubeSide, float y0, float z0, float alpha, float ypitch,
                                           float zpitch, int stagg) {
    AlignmentParameters& params = meas(iML, iTubeSide);
    params.y0 = y0;
    params.z0 = z0;
    params.alpha = alpha;
    params.ypitch = ypitch;
    params.zpitch = zpitch;
    params.stagg = stagg;
}

