/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetGeoModelTool.h"

#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "InDetIdentifier/TRT_ID.h"

#include "InDetIdentifier/SCT_ID.h"

#include "InDetIdentifier/PixelID.h"

#include "IdDictDetDescr/IdDictManager.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictMgr.h"

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  InDetGeoModelTool::InDetGeoModelTool(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent) {

      //Only declare the interface
      declareInterface<IInDetGeoModelTool>(this);
  }

  /**
   * In initalizie, retrieve all the requested geometry model managers
   */
  StatusCode InDetGeoModelTool::initialize(){

    /**
     * Check for Pixel ID helper
     */

    // Get identifier helper
    if (detStore()->retrieve(m_PixelIDHelper, "PixelID").isFailure()) {
        ATH_MSG_ERROR( "Could not get Pixel ID helper" );
        return StatusCode::RECOVERABLE;
    }
  
    /**
     * Check for SCT ID helper
     */

    // Get identifier helper
    if (detStore()->retrieve(m_SCTIDHelper, "SCT_ID").isFailure()) {
        ATH_MSG_ERROR( "Could not get SCT ID helper" );
        return StatusCode::RECOVERABLE;
    }
 
    // check if SLHC geo is used (TRT not implemented)
    // if not SLHC, get the TRT Det Descr manager
    bool isSLHC = false;
    const IdDictManager* idDictMgr = nullptr;
    if (detStore()->retrieve(idDictMgr, "IdDict").isFailure()) {
      ATH_MSG_ERROR( "Could not get IdDictManager !" );
      return StatusCode::RECOVERABLE;
    } else {
      const IdDictDictionary* dict = idDictMgr->manager()->find_dictionary("InnerDetector"); 
      if(!dict) {
	ATH_MSG_ERROR( " Cannot access InnerDetector dictionary " );
	return StatusCode::RECOVERABLE;
      }else{
	//	if (dict->file_name().find("SLHC")!=std::string::npos) isSLHC=true;
	if (dict->version().find("SLHC")!=std::string::npos) isSLHC=true;
      }
    }
 
    if(!isSLHC){
      /**
       * Check for TRT geo model manager and ID helper
       */

      // Get geo model manager
      if ( detStore()->retrieve(m_TRTGeoManager, "TRT").isFailure()) {
	      ATH_MSG_ERROR( "Could not get TRT GeoModel Manager!" );
	      return StatusCode::RECOVERABLE;
      }

      // Get identifier helper
      if (detStore()->retrieve(m_TRTIDHelper, "TRT_ID").isFailure()) {
        ATH_MSG_ERROR( "Could not get TRT ID helper" );
        return StatusCode::RECOVERABLE;
      }
    }

    return StatusCode::SUCCESS;
  }
 
}//namespace





