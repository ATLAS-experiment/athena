/**
 **   @file   RegSelCondAlg_Tile.cxx         
 **            
 **           conditions algorithm to create the Si detector 
 **           lookup tables    
 **            
 **   @author sutt
 **   @date   Sun 22 Sep 2019 10:21:50 BST
 **
 **
 **   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 **/


#include "RegSelCondAlg_Tile.h"

#include "GaudiKernel/EventIDRange.h"
#include "StoreGate/WriteCondHandle.h"

#include "RegionSelector/RegionSelectorLUT.h"

#include "TileRawUtils/TileRoI_Map.h"

#include "eformat/SourceIdentifier.h"

#include "TileByteStream/TileHid2RESrcID.h"
#include "TileIdentifier/TileHWID.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "CaloIdentifier/TileID.h"
#include "TileDetDescr/TileDetDescrManager.h"
#include "CaloDetDescr/CaloDetDescrElement.h"

#include <cmath>
#include <iostream>
#include <string>
#include <memory>


RegSelCondAlg_Tile::RegSelCondAlg_Tile(const std::string& name, ISvcLocator* pSvcLocator):
  AthCondAlgorithm( name, pSvcLocator ),
  m_managerName(""),
  m_printTable(false)
{ 
  declareProperty( "ManagerName", m_managerName );  
  declareProperty( "PrintTable",  m_printTable=false );  
}




StatusCode RegSelCondAlg_Tile::initialize() {
  ATH_MSG_DEBUG("RegSelCondAlg_Tile::initialize() ");
  ATH_CHECK(m_hid2RESrcIDKey.initialize());
  ATH_CHECK(m_tableKey.initialize());
  ATH_MSG_INFO("RegSelCondAlg_Tile::initialize() " << m_tableKey );
  return StatusCode::SUCCESS;
}



StatusCode RegSelCondAlg_Tile::execute(const EventContext& ctx)  const {

  ATH_MSG_DEBUG("RegSelCondAlg_Tile::execute() -- enter -- ");

   /// do stuff here ...  
  ATH_MSG_DEBUG( "Creating region selector table " << m_tableKey );
   

  SG::WriteCondHandle<IRegSelLUTCondData> lutCondData( m_tableKey, ctx );
  if (lutCondData.isValid()) {
    /// inpractice, this should never be called, although in serial athena,                                                                          
    /// because the implementation of the conditions behaviour is flawed in                                                                          
    /// the framework, this routine will be called every event (!) regardless                                                                        
    /// of whether it should be called or not so we need this check to                                                                               
    /// prevent unecessary code execution on out our side                                                                                            
    ATH_MSG_DEBUG("CondHandle " << lutCondData.fullKey() << " is already valid." );
    return StatusCode::SUCCESS;
  }

  SG::ReadCondHandle<TileHid2RESrcID> hid2RESrcID(m_hid2RESrcIDKey, ctx);
  if(!hid2RESrcID.isValid()) {
     ATH_MSG_ERROR( "Do not have TileHid2RESrcID from key " << m_hid2RESrcIDKey.key() );
     return StatusCode::FAILURE;
  }

  EventIDRange id_range;
  
  if( !hid2RESrcID.range( id_range ) ) {
    ATH_MSG_ERROR("Failed to retrieve validity range for " << hid2RESrcID.key());
    return StatusCode::FAILURE;
  }   

  
  ATH_MSG_INFO( "creating new Tile table" );

  /// create the new lookup table

  std::unique_ptr<IRegSelLUT> rd = createTable(*hid2RESrcID);

  if ( !rd ) return StatusCode::FAILURE;

  ATH_MSG_INFO( "Storing the new look up table " << m_tableKey );

  /// create the conditions data for storage 

  IRegSelLUTCondData* rcd = new IRegSelLUTCondData( std::move(rd) );  

  try {
    /// leave this commented here since this is where it should really be,
    /// but we had to move it up in the code to handle the flawed conditions 
    /// handling in the serial athena use case 
    ///    SG::WriteCondHandle<IRegSelLUTCondData> lutCondData( m_tableKey, ctx );
    if( lutCondData.record( id_range, rcd ).isFailure() ) {
      ATH_MSG_ERROR( "Could not record " << m_tableKey 
		     << " " << lutCondData.key()
		     << " with range " << id_range );  
      return StatusCode::FAILURE;   
    } 
    ATH_MSG_INFO( "RegSelCondAlg LUT recorded: " << m_tableKey);
  }
  catch (...) { 
    ATH_MSG_ERROR("RegSelCondAlg_Tile::execute() failed to record table: " << m_tableKey);
    return StatusCode::FAILURE;   
  }   
  
  
  ATH_MSG_DEBUG("RegSelCondAlg_Tile::execute() -- exit -- ");

  return StatusCode::SUCCESS;
}




std::unique_ptr<RegSelectorMap> RegSelCondAlg_Tile::createTable(const TileHid2RESrcID* hid2re) const {

  std::unique_ptr<RegSelectorMap> lut(nullptr);

  /// get all the tools, managers and helpers locally - done't need to create 
  /// class variables for any of these 


  // Get the TileID helper from the detector store
  const TileID* tileID;
  if (detStore()->retrieve(tileID, "TileID").isFailure()) {
    ATH_MSG_ERROR( "Could not get TileID helper !");
    return lut;
  }

  // Get the TileHWID helper from the detector store
  const TileHWID* tileHWID;
  if (detStore()->retrieve(tileHWID, "TileHWID").isFailure()) {
    ATH_MSG_ERROR( "Could not get TileHWID helper !");
    return lut;
  }

  // Get pointer to TileDetDescrManager
  const TileDetDescrManager* tileMgr;
  if (detStore()->retrieve(tileMgr).isFailure()) {
    ATH_MSG_ERROR( "Unable to retrieve TileDetDescrManager from DetectorStore" );
    return lut;
  }

  std::unique_ptr<RegionSelectorLUT> ttLut = std::make_unique<RegionSelectorLUT>(256);

  enum Partition { Ancillary = 0, LBA = 1, LBC = 2, EBA = 3, EBC = 4 };

  // hard-coded etamin/etamax - ignoring slightly different positions of EBA end ENC partitions
  // FIXME: this is very poor - the actual positions should come from the geometry, not be hard 
  ///       coded in here

  std::vector<double> etamin(TileCalibUtils::MAX_ROS);
  etamin[LBA]=-0.1;
  etamin[LBC]=-1.0;
  etamin[EBA]=+0.708779;
  etamin[EBC]=-1.60943;
  std::vector<double> etamax(TileCalibUtils::MAX_ROS);
  etamax[LBA]=+1.0;
  etamax[LBC]=+0.1;
  etamax[EBA]=+1.60943;
  etamax[EBC]=-0.708779;

  // so presumably USE_CELL_PHI is not definied, so we don't want 
  /// this code - this is not clear at all

# if (defined USE_MODULE_PHI)
# else
  double dphi = 2 * M_PI / TileCalibUtils::MAX_DRAWER; // 0.09817477;
# endif

  int sam = 0; // ?
  int layer = 0; //?
  int firstone = tileHWID->drawerIdx( tileHWID->drawer_id(LBA,0) ); // hash index of first real drawer (LBA01)
  for (unsigned int ros = 0; ros < TileCalibUtils::MAX_ROS; ++ros) {
    for (unsigned int drawer = 0; drawer < TileCalibUtils::MAX_DRAWER; ++drawer) {

      int hash = tileHWID->drawerIdx( tileHWID->drawer_id(ros,drawer) ) - firstone;
      if (hash<0) continue;
      
      int coll = tileHWID->frag(ros, drawer);
      int rod = hid2re->getRodID(coll);

      double etami = etamin[ros];
      double etama = etamax[ros];

#     if (defined USE_MODULE_PHI)
      // alternative approach
      int section = (ros==LBA || ros==LBC) ? TileID::BARREL : TileID::EXTBAR;
      int side = (ros==LBA || ros==EBA) ? TileID::POSITIVE : TileID::NEGATIVE;
      Identifier moduleID = tileID->module_id(section,side,drawer);
      CaloDetDescriptor *moduleDDE = tileMgr->get_module_element(moduleID);
      double phimin = moduleDDE->calo_phi_min();
      double phimax = moduleDDE->calo_phi_max();
      if ( phimin < 0.0 ) {
        phimin+=2*M_PI;
        if ( phimax < phimin ) phimax+=2*M_PI;
      }
#     else
      // OLD method - we can predict phi boundaries from module number
      double phimin = drawer * dphi;
      double phimax = (drawer+1) * dphi;
#     endif

      ttLut->additem(etami,etama,phimin,phimax,sam,layer,hash,rod);

    }
  }

  
  /// The RegionSelectorLUT and RegSelectorMap classes are truly 
  /// awful and should be completely replaced, but for the time being 
  /// need to create the RegSelectorMap from the RegionSelectorLUT 
  /// (why not just fill the Map to start with ???)
  /// and then call mountDataStruct() - why this insane approach ? 

  lut = std::make_unique<RegSelectorMap>();
  lut->addLut( ttLut.get() );
  lut->mountDataStruct(); 

  return lut;
}

