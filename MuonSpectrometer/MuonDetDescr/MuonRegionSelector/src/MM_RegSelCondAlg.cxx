/**
 **   @file   MM_RegSelCondAlg.cxx         
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

#include "GaudiKernel/EventIDRange.h"
#include "StoreGate/WriteCondHandle.h"

#include "CLHEP/Units/SystemOfUnits.h"
#include "Identifier/IdentifierHash.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>


#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h" 

#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"

#include "MuonNSWCommonDecode/NSWOfflineHelper.h"

#include "RegSelLUT/RegSelSiLUT.h"


#include "MM_RegSelCondAlg.h"


MM_RegSelCondAlg::MM_RegSelCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  MuonRegSelCondAlg( name, pSvcLocator )
{ 
  ATH_MSG_DEBUG( "MM_RegSelCondAlg::MM_RegSelCondAlg() " << name );
}


std::unique_ptr<RegSelSiLUT> MM_RegSelCondAlg::createTable( const EventContext& ctx, EventIDRange& id_range ) const { 

  /// get the MM detector manager
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> manager( m_detMgrKey, ctx );

  if( !manager.range( id_range ) ) {
    ATH_MSG_ERROR("Failed to retrieve validity range for " << manager.key());
    return {nullptr};
  }   
   


  const MmIdHelper*  helper = manager->mmIdHelper();

  
  std::vector<Identifier>::const_iterator  idfirst = helper->module_begin();
  std::vector<Identifier>::const_iterator  idlast =  helper->module_end();
 
  /// Using the module context
  const IdContext ModuleContext = helper->module_context();
  
  ATH_MSG_DEBUG("createTable()");
  
  std::unique_ptr<RegSelSiLUT> lut = std::make_unique<RegSelSiLUT>(RegSelSiLUT::MM);

  for ( std::vector<Identifier>::const_iterator i=idfirst ; i!=idlast ; ++i ) {

      Identifier     Id = *i;
      IdentifierHash hashId;

      helper->get_hash( Id, hashId, &ModuleContext );

      const MuonGM::MMReadoutElement* mm = manager->getMMReadoutElement(Id);
      if (!mm) continue;

      std::string stationName=mm->getStationName();
      int stationEta=mm->getStationEta();
      int stationPhi=mm->getStationPhi();
      int multilayer = helper->multilayer(Id);

      char side     = mm->getStationEta() < 0 ? 'C' : 'A';

      /// now calculate the required limits
      Amg::Vector3D mmPos = mm->center();      

      double swidth = mm->getSsize();
      double lwidth = mm->getLongSsize();

      double length = mm->getRsize();
      double depth  = mm->getZsize();

      double moduleR = std::sqrt( mmPos.mag()*mmPos.mag() -  mmPos.z()*mmPos.z());

      double zmin = mmPos.z()-0.5*depth;
      double zmax = mmPos.z()+0.5*depth;

      //      std::cout << "MM:module: rcen: " << moduleR << "\tlength: " << length << "\tswidth: " << swidth << "\tlwidth: " << lwidth << std::endl; 

      double rmin = moduleR-0.5*length;
      double rmax = std::sqrt( (moduleR+0.5*length)*(moduleR+0.5*length) + 0.25*lwidth*lwidth );

      double dphi1 = std::atan( (0.5*lwidth)/(moduleR+0.5*length) );
      double dphi2 = std::atan( (0.5*swidth)/(moduleR-0.5*length) );

      double dphi = ( dphi1 > dphi2 ? dphi1 : dphi2 );

      double phimin = mmPos.phi()-dphi;
      double phimax = mmPos.phi()+dphi;

      if ( phimin >  M_PI ) phimin -= 2*M_PI;
      if ( phimin < -M_PI ) phimin += 2*M_PI;

      if ( phimax >  M_PI ) phimax -= 2*M_PI;
      if ( phimax < -M_PI ) phimax += 2*M_PI;

      int layerid = multilayer;
      int detid   = ( side == 'C' ? -1 : 1 );

      /// store the robId
      Muon::nsw::helper::NSWOfflineRobId robIdHelper(stationName,static_cast<int8_t>(stationEta),static_cast<uint8_t>(stationPhi));
      for(uint32_t robId : robIdHelper.get_ids()){ // if the NSW is read out in a split ROB configuration there are multiple ROB ids associated to one region of interest
          RegSelModule m( zmin, zmax, rmin, rmax, phimin, phimax, layerid, detid, robId, hashId );
          lut->addModule( m );
      }

  }

  lut->initialise();

  return lut;

}

