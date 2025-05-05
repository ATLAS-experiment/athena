/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_MDTREGION_H
#define  TRIGL2MUONSA_MDTREGION_H

#include "src/MuonRoad.h"
namespace TrigL2MuonSA {

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

  class MdtRegion
  {
  public:
    MdtRegion() { Clear(); };    
    
    void Clear()
	{
	  for(int i=0; i<N_STATION; i++) {
	    for(int j=0; j<N_SECTOR; j++) {
	      zMin[i][j] = 0;
	      zMax[i][j] = 0;
	      rMin[i][j] = 0;
	      rMax[i][j] = 0;
	      etaMin[i][j] = 0;
	      etaMax[i][j] = 0;
	      phiMin[i][j] = 0;
	      phiMax[i][j] = 0;
	      for(int k=0; k<2; k++) chamberType[i][j][k] = 0;	      
	    }
	  }
	};
      
  public:
      double zMin[N_STATION][N_SECTOR];
      double zMax[N_STATION][N_SECTOR];
      double rMin[N_STATION][N_SECTOR];
      double rMax[N_STATION][N_SECTOR];
      double etaMin[N_STATION][N_SECTOR];
      double etaMax[N_STATION][N_SECTOR];
      double phiMin[N_STATION][N_SECTOR];
      double phiMax[N_STATION][N_SECTOR];
      int chamberType[N_STATION][N_SECTOR][2];
  };

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
}

#endif
