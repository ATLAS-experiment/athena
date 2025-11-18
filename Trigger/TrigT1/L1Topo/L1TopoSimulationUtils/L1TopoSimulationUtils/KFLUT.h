/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// KFLUT.h
// contains LUT values for KF corrections to MET
//  Created by V Sorin on 03/2015
//

#ifndef L1TopoSimulationUtils_KFLUT
#define L1TopoSimulationUtils_KFLUT

#include <vector>

namespace TCS {



 class KFLUT {

   public:
      KFLUT();
      ~KFLUT();


      int getetabin(double eta);
      int getetbin(unsigned int et);
  
      double getcorrKF(int i, int j);
      

   private:

      void fillLUT();
      std::vector<unsigned int> etlimits;
      std::vector<double> etalimits;
     
      std::vector<std::vector<double>> LUTKF;

 };



} // end namespace

#endif
