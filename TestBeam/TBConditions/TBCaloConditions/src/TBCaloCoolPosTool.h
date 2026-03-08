/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBCALOCONDITIONS_TBCALOCOOLPOSTOOL
# define TBCALOCONDITIONS_TBCALOCOOLPOSTOOL

#include "TBCaloConditions/ITBCaloPosTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"

/** 
 ** Class TBCaloCoolPosTool
 ** 
 ** Implementation of ITBCaloPosTool using COOL

   Feb 6, 2006   Richard Hawkings
	
 **/
 
class TBCaloCoolPosTool : public extends<AthAlgTool, ITBCaloPosTool>
{
public:
    using base_class::base_class;

    virtual StatusCode initialize() override;

    ///  access eta value 
    virtual double eta () const override;

    ///  access eta value 
    virtual double  theta () const override;

    ///  access eta value 
    virtual double z () const override;

    ///  access eta value 
    virtual double delta () const override;

private: 

    // For run<1000454, replace TILE_LV_62 with SYSTEM1

    SG::ReadCondHandleKey<AthenaAttributeList> m_etaTableKey
    { this, "EtaKey", "/TILE/DCS/TILE_LV_62/TABLE/ETA", "" };
    SG::ReadCondHandleKey<AthenaAttributeList> m_thetaTableKey
    { this, "EtaKey", "/TILE/DCS/TILE_LV_62/TABLE/THETA", "" };
    SG::ReadCondHandleKey<AthenaAttributeList> m_zTableKey
    { this, "EtaKey", "/TILE/DCS/TILE_LV_62/TABLE/Z", "" };
    SG::ReadCondHandleKey<AthenaAttributeList> m_deltaTableKey
    { this, "EtaKey", "/TILE/DCS/TILE_LV_62/TABLE/DELTA", "" };
};



#endif
