/**
 **   @file   sTGC_RegSelCondAlg.cxx         
 **            
 **           conditions algorithm to create the Si detector 
 **           lookup tables    
 **            
 **   @author sutt
 **   @date   Sun 22 Sep 2019 10:21:50 BST
 **
 **
 **   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/


#include <iostream>
#include <fstream>
#include <string>

#include "CLHEP/Units/SystemOfUnits.h"
#include "Identifier/IdentifierHash.h"

#include "RegSelLUT/RegSelModule.h" 
#include "RegSelLUT/RegSelSiLUT.h" 

#include "MuonReadoutGeometry/MuonReadoutElement.h" 

/// sTGC naming convention headers
#include "MuonReadoutGeometry/sTgcReadoutElement.h"

#include "MuonReadoutGeometry/MuonStation.h"
#include "MuonNSWCommonDecode/NSWOfflineHelper.h"

#include "sTGC_RegSelCondAlg.h"


sTGC_RegSelCondAlg::sTGC_RegSelCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  MuonRegSelCondAlg( name, pSvcLocator )
{ 
  ATH_MSG_DEBUG( "sTGC_RegSelCondAlg::sTGC_RegSelCondAlg() " << name );
}




std::unique_ptr<RegSelSiLUT> sTGC_RegSelCondAlg::createTable( const EventContext& ctx, EventIDRange& id_range ) const { 

  SG::ReadCondHandle<MuonGM::MuonDetectorManager> manager( m_detMgrKey, ctx );

  if( !manager.range( id_range ) ) {
    ATH_MSG_ERROR("Failed to retrieve validity range for " << manager.key());
    return {nullptr};
  } 

  const sTgcIdHelper*  helper = manager->stgcIdHelper();

  std::vector<Identifier>::const_iterator  idfirst = helper->module_begin();
  std::vector<Identifier>::const_iterator  idlast =  helper->module_end();
 
  /// we want the module context ...
  const IdContext ModuleContext = helper->module_context();
  
  ATH_MSG_DEBUG("createTable()");
  
  std::unique_ptr<RegSelSiLUT> lut = std::make_unique<RegSelSiLUT>(RegSelSiLUT::sTGC);


  for ( std::vector<Identifier>::const_iterator i=idfirst ; i!=idlast ; ++i ) {
   
      Identifier     Id = *i;
      IdentifierHash hashId;

      helper->get_hash( Id, hashId, &ModuleContext );

      const MuonGM::sTgcReadoutElement* stgc = manager->getsTgcReadoutElement(Id);
      if (!stgc) continue;

      //      std::cout << "stgc station name: " << mm->getStationName() << "\teta: " << mm->getStationEta() << "\tphi: " << mm->getStationPhi() << std::endl; 
  
      std::string stationName=stgc->getStationName();
      int stationEta=stgc->getStationEta();
      int stationPhi=stgc->getStationPhi();
      int multilayer = helper->multilayer(Id);

      char side     = stationEta < 0 ? 'C' : 'A';


  
      Amg::Vector3D sTgcPos = stgc->center();      
  
      double swidth = stgc->getSsize();
      double lwidth = stgc->getLongSsize();

      double ycutout = stgc->getPadDesign(1)->yCutout;

      double length = stgc->getRsize();
      double depth  = stgc->getZsize();

      double moduleR = std::sqrt( sTgcPos.mag()*sTgcPos.mag() -  sTgcPos.z()*sTgcPos.z());

      double zmin = sTgcPos.z()-0.5*depth;
      double zmax = sTgcPos.z()+0.5*depth;

      double rmin = moduleR-0.5*length;
      double rmax = std::sqrt( (moduleR+0.5*length)*(moduleR+0.5*length) + lwidth*lwidth/4 );

      double dphi1 = std::atan( (0.5*lwidth)/(moduleR+0.5*length) );
      double dphi2 = std::atan( (0.5*swidth)/(moduleR-0.5*length) );

      double dphi = ( dphi1 > dphi2 ? dphi1 : dphi2 );

      if ( ycutout > 0 ) { 
        double rcutout = moduleR+0.5*length - ycutout;
        double dphicutout = std::atan( (0.5*lwidth)/rcutout );
        if ( dphi < dphicutout ) dphi = dphicutout;
      }

      double phimin = sTgcPos.phi()-dphi;
      double phimax = sTgcPos.phi()+dphi;

      if ( phimin >  M_PI ) phimin -= 2*M_PI;
      if ( phimin < -M_PI ) phimin += 2*M_PI;

      int layerid = multilayer;
      int detid   = ( side == 'C' ? -1 : 1 );

      /// store the robId
      Muon::nsw::helper::NSWOfflineRobId robIdHelper(stationName,static_cast<int8_t>(stationEta),static_cast<uint8_t>(stationPhi));
      for(uint32_t robId : robIdHelper.get_ids()){
          RegSelModule m( zmin, zmax, rmin, rmax, phimin, phimax, layerid, detid, robId, hashId );
          lut->addModule( m );
      }

  }

  lut->initialise(); 

  return lut;
}
