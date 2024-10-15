/**
 **   @file   TGC_RegSelCondAlg.cxx         
 **            
 **           conditions algorithm to create the Si detector 
 **           lookup tables    
 **            
 **   @author sutt
 **   @date   Sun 22 Sep 2019 10:21:50 BST
 **
 **
 **   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 **/


#include "GaudiKernel/EventIDRange.h"
#include "StoreGate/WriteCondHandle.h"

#include "CLHEP/Units/SystemOfUnits.h"
#include "Identifier/IdentifierHash.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>


#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"

#include "MuonTGC_Cabling/MuonTGC_CablingSvc.h"

#include "TGC_RegSelCondAlg.h"



TGC_RegSelCondAlg::TGC_RegSelCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  MuonRegSelCondAlg( name, pSvcLocator )
{ 
  ATH_MSG_DEBUG( "TGC_RegSelCondAlg::TGC_RegSelCondAlg() " << name );
}




std::unique_ptr<RegSelSiLUT> TGC_RegSelCondAlg::createTable( const EventContext& ctx, EventIDRange& id_range ) const { 

  SG::ReadCondHandle<MuonGM::MuonDetectorManager> manager( m_detMgrKey, ctx );

  if( !manager.range( id_range ) ) {
    ATH_MSG_ERROR("Failed to retrieve validity range for " << manager.key());
    return nullptr;
  }


  /// now get the TGC cabling service ...

  ServiceHandle<MuonTGC_CablingSvc> cabling("MuonTGC_CablingSvc", name());
  ATH_CHECK( cabling.retrieve(), {} );

  const TgcIdHelper*  helper = manager->tgcIdHelper();
 
  const IdContext ModuleContext = helper->module_context();

  ATH_MSG_DEBUG("createTable()");
  
  std::unique_ptr<RegSelSiLUT> lut = std::make_unique<RegSelSiLUT>();


  for ( auto i = helper->module_begin(); i != helper->module_end(); ++i) {
   
    Identifier     Id = *i;
    IdentifierHash hashId{0};

    helper->get_module_hash(Id, hashId);
   
    ExpandedIdentifier exp_id;
    if (helper->get_expanded_id( Id, exp_id, &ModuleContext)) {
      ATH_MSG_DEBUG("Failed retrieving ExpandedIdentifier for PRD Identifier = " << Id.getString() << ". Skipping to the next PRD.");
      continue;
    }
      
    int detid   = ( exp_id[2]<0 ? -1 : 1 );
    int layerid = exp_id[1]+1;

    const MuonGM::TgcReadoutElement* tgc = manager->getTgcReadoutElement(Id);
    if (tgc == nullptr)  {
      continue;
    }
            
    constexpr int gapMin = 1;
    const int gapMax = tgc->nGasGaps();

    Identifier chId = helper -> channelID(Id,gapMin,0,1);
    const int chmax = tgc->nWireGangs(gapMin);
    Amg::Vector3D posmax = tgc->channelPos(gapMin,0,chmax); // gapMax gives posmax!
    chId = helper -> channelID(Id,gapMax,0,1);
    constexpr int chmin = 1;
    Amg::Vector3D posmin = tgc->channelPos(gapMax, 0, chmin); // gapMin gives posmin!

    // caliculation based on active sensitive area
      
    Amg::Vector3D posctr = tgc->globalPosition();
    double activeheight  = tgc->length();

    const double zmin = posmin.z();
    const double zmax = posmax.z();

    const double rmin = posctr.perp()-0.5*activeheight;
    const double rmax = posctr.perp()+0.5*activeheight;

    const double minTheta = std::atan2(std::abs(zmin), rmin);
    const double maxTheta = std::atan2(std::abs(zmax), rmax);
    double etamin = -std::log(0.5*std::tan(minTheta));
    double etamax = -std::log(0.5*std::tan(maxTheta));

    if (helper->stationEta(Id) < 0) {
      etamin = -etamin;
      etamax = -etamax;
    }


    // caliculation based on active sensitive area
    double activelongside = tgc->getLongSsize()-tgc->frameXwidth()*2.;
    /// Should use std::atan2 etc, but atan2f is not definied in cmath, so for backwards 
    /// compatability we are forced to use the old versions 
    double phimin = std::atan2(posctr.y(),posctr.x()) - std::atan2(activelongside/2.,posctr.perp()+activeheight/2.);
    double phimax = std::atan2(posctr.y(),posctr.x()) + std::atan2(activelongside/2.,posctr.perp()+activeheight/2.);

    if (phimin < 0) phimin += 2.*M_PI;
    if (phimax < 0) phimax += 2.*M_PI;

    // get ROB id
    int subDetectorId = 0; // 0x67 (A side) or 0x68 (C side)
    int rodId = 0; // 1-12
    cabling->getReadoutIDfromElementID(Id, subDetectorId, rodId);
    uint32_t robId =  ( ((0x0ff) & subDetectorId)<<16 ) | (rodId);
    // end part to get ROB id

    RegSelModule m( zmin, zmax, rmin, rmax, phimin, phimax, layerid, detid, robId, hashId );
    lut->addModule( m );

  }

  lut->initialise();

  return lut;

}







