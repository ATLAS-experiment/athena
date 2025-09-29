/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1JobConfigInfo                  //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: March 2008                               //
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1Utils/VP1JobConfigInfo.h"
#include "VP1Utils/VP1DetInfo.h"
#include "VP1Utils/VP1SGAccessHelper.h"
#include "VP1Utils/VP1SGContentsHelper.h"
#include "VP1Base/IVP1System.h"
#include "VP1Base/VP1Msg.h"
#include "VP1Base/VP1AthenaPtrs.h"

#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "StoreGate/DataHandle.h"

#include <QString>

//____________________________________________________________________
class VP1JobConfigInfo::Imp {
public:
  static void ensureInit();
  static bool actualInit(StoreGateSvc* detStore);
  static bool initialised;

  static GeoPVConstLink geoModelWorld;
  static bool hasGeoModelExperiment;
  static bool hasITkGeometry;
  static bool hasPixelGeometry;
  static bool hasSCTGeometry;
  static bool hasTRTGeometry;
  static bool hasInDetServiceMaterialGeometry;
  static bool hasBeamPipeGeometry;
  static bool hasLArGeometry;
  static bool hasTileGeometry;
  static bool hasMuonGeometry;
  static bool hasMuonNSWGeometry;
  static bool hasLUCIDGeometry;
  static bool hasBCMGeometry;
  static bool hasCavernInfraGeometry;

  static void turnOffAll();
};

GeoPVConstLink VP1JobConfigInfo::Imp::geoModelWorld;
// init default values
bool VP1JobConfigInfo::Imp::initialised = false;
bool VP1JobConfigInfo::Imp::hasGeoModelExperiment = false;
bool VP1JobConfigInfo::Imp::hasITkGeometry = false;
bool VP1JobConfigInfo::Imp::hasPixelGeometry = false;
bool VP1JobConfigInfo::Imp::hasSCTGeometry = false;
bool VP1JobConfigInfo::Imp::hasTRTGeometry = false;
bool VP1JobConfigInfo::Imp::hasInDetServiceMaterialGeometry = false;
bool VP1JobConfigInfo::Imp::hasBeamPipeGeometry = false;
bool VP1JobConfigInfo::Imp::hasLArGeometry = false;
bool VP1JobConfigInfo::Imp::hasTileGeometry = false;
bool VP1JobConfigInfo::Imp::hasMuonGeometry = false;
bool VP1JobConfigInfo::Imp::hasMuonNSWGeometry = false; 
bool VP1JobConfigInfo::Imp::hasLUCIDGeometry = false;
bool VP1JobConfigInfo::Imp::hasBCMGeometry = false;
bool VP1JobConfigInfo::Imp::hasCavernInfraGeometry = false;

//____________________________________________________________________
void VP1JobConfigInfo::Imp::turnOffAll()
{
  hasGeoModelExperiment = false;
  hasITkGeometry = false;
  hasPixelGeometry = false;
  hasSCTGeometry = false;
  hasTRTGeometry = false;
  hasInDetServiceMaterialGeometry = false;
  hasBeamPipeGeometry = false;
  hasLArGeometry = false;
  hasTileGeometry = false;
  hasMuonGeometry = false;
  hasLUCIDGeometry = false;
  hasBCMGeometry = false;
  hasCavernInfraGeometry = false;
}

//____________________________________________________________________
void VP1JobConfigInfo::Imp::ensureInit()
{
  if (initialised)
    return;
  if (VP1Msg::verbose())
    VP1Msg::messageVerbose("VP1JobConfigInfo initialising");
  initialised = true;
  if (!actualInit(VP1AthenaPtrs::detectorStore())) {
    VP1Msg::message("VP1JobConfigInfo ERROR: Problems initialising. "
		    "Will assume all subsystems are off in this job!");
    turnOffAll();
  }
  if (VP1Msg::verbose()) {
    VP1Msg::messageVerbose("VP1JobConfigInfo => Found job configuration:");
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasGeoModelExperiment = "+QString(hasGeoModelExperiment?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasITkGeometry = "+QString(hasITkGeometry?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasPixelGeometry = "+QString(hasPixelGeometry?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasSCTGeometry = "+QString(hasSCTGeometry?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasTRTGeometry = "+QString(hasTRTGeometry?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasInDetServiceMaterialGeometry = "+QString(hasInDetServiceMaterialGeometry?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasBeamPipeGeometry = "+QString(hasBeamPipeGeometry?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasLArGeometry = "+QString(hasLArGeometry?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasTileGeometry = "+QString(hasTileGeometry?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasMuonGeometry = "+QString(hasMuonGeometry?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasMuonNSWGeometry = "+QString(hasMuonNSWGeometry?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasLUCIDGeometry = "+QString(hasLUCIDGeometry?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasBCMGeometry = "+QString(hasBCMGeometry?"On":"Off"));
    VP1Msg::messageVerbose("VP1JobConfigInfo => hasCavernInfraGeometry = "+QString(hasCavernInfraGeometry?"On":"Off"));
  }

}

//____________________________________________________________________
bool VP1JobConfigInfo::hasGeoModelExperiment() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasGeoModelExperiment; }
bool VP1JobConfigInfo::hasITkGeometry() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasITkGeometry; }
bool VP1JobConfigInfo::hasPixelGeometry() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasPixelGeometry; }
bool VP1JobConfigInfo::hasSCTGeometry() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasSCTGeometry; }
bool VP1JobConfigInfo::hasTRTGeometry() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasTRTGeometry; }
bool VP1JobConfigInfo::hasInDetServiceMaterialGeometry() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasInDetServiceMaterialGeometry; }
bool VP1JobConfigInfo::hasBeamPipeGeometry() { if (!Imp::initialised)  Imp::ensureInit(); return Imp::hasBeamPipeGeometry; }
bool VP1JobConfigInfo::hasLArGeometry() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasLArGeometry; }
bool VP1JobConfigInfo::hasTileGeometry() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasTileGeometry; }
bool VP1JobConfigInfo::hasMuonGeometry() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasMuonGeometry; }
bool VP1JobConfigInfo::hasMuonNSWGeometry() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasMuonNSWGeometry; }
bool VP1JobConfigInfo::hasLUCIDGeometry() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasLUCIDGeometry; }
bool VP1JobConfigInfo::hasBCMGeometry() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasBCMGeometry; }
bool VP1JobConfigInfo::hasCavernInfraGeometry() { if (!Imp::initialised) Imp::ensureInit(); return Imp::hasCavernInfraGeometry; }

//____________________________________________________________________
bool VP1JobConfigInfo::Imp::actualInit( StoreGateSvc* detStore )
{
  if (!detStore)
    return false;

  VP1SGContentsHelper sg_contents(detStore);
  const QString geomodelkey = "ATLAS";
  if (!sg_contents.contains<GeoModelExperiment>(geomodelkey)) {
    if (VP1Msg::verbose())
      VP1Msg::messageVerbose("VP1JobConfigInfo: No GeoModelExperiment in detectorStore. Concluding all subsystems are off.");
    turnOffAll();
    return true;
  }

  VP1SGAccessHelper sg_access(detStore);

  const GeoModelExperiment * theExpt;
  if (!sg_access.retrieve(theExpt,geomodelkey)) {
    if (VP1Msg::verbose())
      VP1Msg::messageVerbose("VP1JobConfigInfo: No GeoModelExperiment in detectorStore. Concluding all subsystems are off.");
    turnOffAll();
    return true;
  }
  if (VP1Msg::verbose())
    VP1Msg::messageVerbose("VP1JobConfigInfo: Retrieved GeoModelExperiment/"+geomodelkey);

  const GeoPhysVol * worldPhysVol = theExpt->getPhysVol();
  if (!worldPhysVol) {
    VP1Msg::message("VP1JobConfigInfo: ERROR: GeoModelExperiment has null physical volume.");
    turnOffAll();
    return false;
  }
  if (VP1Msg::verbose())
      VP1Msg::messageVerbose("VP1JobConfigInfo: Retrieved GeoModelExperiment physical volumes");

  hasGeoModelExperiment = true;
  PVConstLink world(worldPhysVol);

  GeoVolumeCursor av(world);


  while (!av.atEnd()) {
    std::string name = av.getName();

    if ( !hasITkGeometry && name=="ITkPixel") { hasITkGeometry = true; }
    if ( !hasITkGeometry && name=="ITkStrip") { hasITkGeometry = true; }
    if ( !hasPixelGeometry && name=="Pixel") {
      hasPixelGeometry = true;
      if (not hasITkGeometry and name == "ITkPixel") //TODO: I guess this was to handle temporary configurations, I think it must be removed at some point
        hasITkGeometry = true;
      if ( !hasBCMGeometry ) {
	    //Loop under the top Pixel volume to check if there are BCM volumes
	    //present in the current config:
	    GeoVolumeCursor pv(av.getVolume());
	    while (!pv.atEnd()) {
	        if (pv.getVolume()->getLogVol()->getName()=="bcmModLog") {
	        hasBCMGeometry = true;
	        break;
	        }
	        pv.next();
	    }
      }
    }
    if ( !hasSCTGeometry && name=="SCT" {
      hasSCTGeometry = true;
      if (not hasITkGeometry and name == "ITkStrip")//TODO: I guess this was to handle temporary configurations, I think it must be removed at some point
        hasITkGeometry = true;
    }
    if ( !hasTRTGeometry && name=="TRT") hasTRTGeometry = true;
    if ( !hasInDetServiceMaterialGeometry && name=="InDetServMat") hasInDetServiceMaterialGeometry = true;
    if ( !hasBeamPipeGeometry && name=="BeamPipe") hasBeamPipeGeometry = true;
    if ( !hasLArGeometry && name=="LArBarrel") hasLArGeometry = true;
    if ( !hasLArGeometry && name=="LArEndcapPos") hasLArGeometry = true;
    if ( !hasLArGeometry && name=="LArEndcapNeg") hasLArGeometry = true;
    if ( !hasTileGeometry && name=="Tile") hasTileGeometry = true;
    if ( !hasMuonGeometry && name=="Muon") {
        hasMuonGeometry = true;
        if ( !hasMuonNSWGeometry ) {
            //Loop under the top Muon volume to check if there are NSW volumes
        	//present in the current config:
           	GeoVolumeCursor pv(av.getVolume());
	        while (!pv.atEnd()) {
	            if (pv.getVolume()->getLogVol()->getName()=="NewSmallWheel") {
	            hasMuonNSWGeometry = true;
	            break;
	            }
	        pv.next();
	        }   
        }
    }
    if ( !hasLUCIDGeometry && (name=="LucidSideA"||name=="LucidSideC")) hasLUCIDGeometry = true;
    //FIXME: Look for CavernInfra as well!!!

    av.next(); // increment volume cursor.
  }

  Imp::geoModelWorld = worldPhysVol;

  return true;
}

//____________________________________________________________________
const GeoPVConstLink * VP1JobConfigInfo::geoModelWorld()
{
  Imp::ensureInit();
  return Imp::geoModelWorld == GeoPVConstLink() ? 0 : &Imp::geoModelWorld;
}
