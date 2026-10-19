///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// File: RecTPCnv/RecTPCnvDict.h
/// Dictionary file for persistent representation(s) of AOD containers
/// Author: Sebastien Binet <binet@cern.ch>
/// Date:   December 2006

#ifndef RECTPCNV_RECTPCNVCNVDICT_H
#define RECTPCNV_RECTPCNVCNVDICT_H

#include "RecTPCnv/CaloEnergy_p2.h"
#include "RecTPCnv/MuonCaloEnergyContainer_p1.h"
#include "RecTPCnv/MuonCaloEnergyContainer_tlp1.h"

#include "RecTPCnv/CaloEnergy_p1.h"

#include "RecTPCnv/DepositInCalo_p2.h"
#include "RecTPCnv/DepositInCalo_p1.h"

extern "C" {
  void caloenergy_cnv_p2_register_streamer();
}

class CaloEnergyCnv_p2_register_streamer
{
public:
  static
  void reg() { caloenergy_cnv_p2_register_streamer(); }
};

#endif //RECTPCNV_RECTPCNVCNVDICT_H
