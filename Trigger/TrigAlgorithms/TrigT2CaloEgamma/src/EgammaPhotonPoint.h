/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
 
 NAME:     EgammaPhotonPoint.h
 PACKAGE:  Trigger/TrigAlgorithms/TrigT2CaloEgamma
 
 AUTHOR:   D.O. Damazio
 
 PURPOSE:  Calculates energy weighted cluster position around
 *******************************************************************/

#ifndef TRIGT2CALOEGAMMA_CALOPHOTONPOINTGAMMA_H 
#define TRIGT2CALOEGAMMA_CALOPHOTONPOINTGAMMA_H


#include "TrigT2CaloCommon/IReAlgToolCalo.h"
#include "TrigT2CaloCommon/Calo_Def.h"
#include "StoreGate/ReadHandleKey.h"
#include "CaloConditions/CaloNoise.h"

class IRoiDescriptor;

/** Feature extraction Tool for LVL2 Calo. Second EM Calorimeter sample. */
class EgammaPhotonPoint: public IReAlgToolCalo {
  public:
  
    /** Constructor */
    EgammaPhotonPoint(const std::string & type, const std::string & name, 
                 const IInterface* parent);

    virtual StatusCode initialize() override;
   /** @brief execute feature extraction for the EM Calorimeter
    *	second layer 
    *   @param[out] rtrigEmCluster is the output cluster.
    *   @param[in] eta/phi-min/max = RoI definition.
    */
    virtual StatusCode execute(xAOD::TrigEMCluster &rtrigEmCluster,
		       const IRoiDescriptor& roi,
		       const CaloDetDescrElement*& /*caloDDE*/,
                       const EventContext& context ) const override;

   private:
    Gaudi::Property<float> m_maxHotCellDeta{this, "MaxDetaHotCell", 1.0};
    Gaudi::Property<float> m_maxHotCellDphi{this, "MaxDphiHotCell", 1.0};

    SG::ReadCondHandleKey<CaloNoise>  m_noiseCDOKey{this,"CaloNoiseKey","totalNoise","SG Key of CaloNoise data object"};

};


#endif
