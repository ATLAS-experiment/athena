/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoPixelBarrel.h"
#include "GeoPixelLayer.h"
#include "GeoPixelServices.h"
#include "GeoPixelOldFrame.h"
#include "GeoPixelStaveRing.h"
#include "GeoPixelIFlexServices.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoPara.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "Identifier/Identifier.h"
#include "InDetGeoModelUtils/ExtraMaterial.h"
#include "InDetIdentifier/PixelID.h"

#include "InDetGeoModelUtils/VolumeBuilder.h"

#include <utility>

using namespace std;
GeoPixelBarrel::GeoPixelBarrel(InDetDD::PixelDetectorManager* ddmgr,
                               PixelGeometryManager* mgr,
			       GeoModelIO::ReadGeoModel* sqliteReader,
                               std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                               std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX,
                               GeoPixelServices * pixServices   )
  : GeoVPixelFactory (ddmgr, mgr, sqliteReader, std::move(mapFPV), std::move(mapAX)),
    m_pixServices(pixServices)
{}

GeoVPhysVol* GeoPixelBarrel::Build( ) {

  GeoFullPhysVol* barrelPhys{nullptr};

  if(m_sqliteReader) {
    barrelPhys = (*m_mapFPV)["Barrel"];
  }
  else {
    //
    // create the Barrel Mother volume
    // 
    double rmin = m_gmt_mgr->PixelBarrelRMin();
    double rmax = m_gmt_mgr->PixelBarrelRMax();
    double halflength = m_gmt_mgr->PixelBarrelHalfLength();
    const GeoMaterial* air = m_mat_mgr->getMaterial("std::Air");
    const GeoTube* barrelTube = new GeoTube(rmin,rmax,halflength);
    const GeoLogVol* barrelLog = new GeoLogVol("Barrel",barrelTube,air);
    barrelPhys = new GeoFullPhysVol(barrelLog);
  }

  // Set numerology
  m_DDmgr->numerology().setNumLayers(m_gmt_mgr->PixelBarrelNLayer());

  // In case of IBL stave detailed description -> add stave ring support and emdblocks
  m_gmt_mgr->SetCurrentLD(0);
  bool bAddIBLStaveRings = m_gmt_mgr->ibl()&&m_gmt_mgr->PixelStaveLayout()>3&&m_gmt_mgr->PixelStaveLayout()<7;

  //
  // Build the layers inside
  //
  GeoPixelLayer layer (m_DDmgr, m_gmt_mgr, m_sqliteReader, m_mapFPV, m_mapAX);
  for(int ii = 0; ii < m_gmt_mgr->PixelBarrelNLayer(); ii++){
    //cout << "Layer" << ii << endl;
    m_gmt_mgr->SetCurrentLD(ii);
    if(m_gmt_mgr->isLDPresent()){
      Identifier id = m_gmt_mgr->getIdHelper()->wafer_id(0,ii,0,0);

      if(m_sqliteReader) {
	// DD2 uses different names for Barrel Layers
	std::string lnameDD2 = "Layer_PixelBarrel"+std::to_string(ii);

	auto itAX = m_mapAX->find(lnameDD2);
	if(itAX == m_mapAX->end())
	  throw std::runtime_error("GeoPixelBarrel cannot find AX for the key " + lnameDD2);
	GeoAlignableTransform* xform = itAX->second;

	auto itFPV = m_mapFPV->find(lnameDD2);
	if(itFPV == m_mapFPV->end())
          throw std::runtime_error("GeoPixelBarrel cannot find FPV for the key " + lnameDD2);
	GeoFullPhysVol* layerFPV = itFPV->second;

	layer.Build();

	// Store the transform (at level 1)
	m_DDmgr->addAlignableTransform(1, id, xform, layerFPV);

	// Extra Material. 
	// From Marilena: I don't think there is much room but we provide the hooks anyway
	// NB this has been taken from the GeoPixelLayer class .. needed to be moved here cause here
	// we have the GeoFullPhysVol available from the SQLite DB
	InDetDD::ExtraMaterial xMat(m_gmt_mgr->distortedMatManager());
	xMat.add(layerFPV,"PixelLayer");
	xMat.add(layerFPV,"PixelLayer"+std::to_string(m_gmt_mgr->GetLD()));
      }
      else {
	// IBL layer shift ( 2mm shift issue )
	double layerZshift = m_gmt_mgr->PixelLayerGlobalShift();
	GeoAlignableTransform* xform = new GeoAlignableTransform(GeoTrf::Translate3D(0.,0.,layerZshift));
	GeoVPhysVol* layerphys = layer.Build();
	std::string lname = "Layer" + std::to_string(ii);
	GeoNameTag *tag = new GeoNameTag(lname);
	barrelPhys->add(tag);
	barrelPhys->add(new GeoIdentifierTag(ii));
	barrelPhys->add(xform);
	barrelPhys->add(layerphys);

	// Store the transform (at level 1)
	m_DDmgr->addAlignableTransform(1, id, xform, layerphys);

	// IBL stave ring service area  ( ring + endblocks + flexes + pipe + ...)
	if(m_pixServices&&bAddIBLStaveRings&&ii==0)
	  {
	    // ----------- end of stave services (side A)
	    GeoNameTag * tagSupportA = new GeoNameTag("StaveRingAndEndblocks_A");
	    GeoTransform *xformSupportA = layer.getSupportTrfA();
	    GeoPhysVol *supportPhys_A = layer.getSupportA();
	    barrelPhys->add(tagSupportA);
	    barrelPhys->add(xformSupportA);
	    barrelPhys->add(supportPhys_A);
	    
	    // ----------- end of stave services (side C)
	    GeoNameTag * tagSupportC = new GeoNameTag("StaveRingAndEndblocks_C");
	    GeoTransform *xformSupportC = layer.getSupportTrfC();
	    GeoPhysVol *supportPhys_C =  layer.getSupportC();
	    barrelPhys->add(tagSupportC);
	    barrelPhys->add(xformSupportC);
	    barrelPhys->add(supportPhys_C);
	    
	    // ----------- middle of stave services 
	    if(m_gmt_mgr->PixelStaveAxe()==0) {
	      GeoNameTag *tagM = new GeoNameTag("Brl0M_StaveRing");         
	      GeoTransform *xformSupportMidRing = layer.getSupportTrfMidRing();
	      GeoVPhysVol *supportPhysMidRing = layer.getSupportMidRing();
	      barrelPhys->add(tagM);
	      barrelPhys->add(xformSupportMidRing);
	      barrelPhys->add(supportPhysMidRing);
	    }
	    
	    // ----------- end of stave PP0 services (insde barrel)
	    
	    if(m_gmt_mgr->IBLFlexAndWingDefined()){
	      GeoPixelIFlexServices iFlexSrv(m_DDmgr, m_gmt_mgr, m_sqliteReader, m_mapFPV, m_mapAX, 0);
	      iFlexSrv.Build();
	      
	      GeoNameTag * tagFlexA = new GeoNameTag("PP0Flex_A");
	      GeoTransform *xformFlexA = iFlexSrv.getSupportTrfA();
	      GeoPhysVol *flexPhys_A = iFlexSrv.getSupportA();
	      barrelPhys->add(tagFlexA);
	      barrelPhys->add(xformFlexA);
	      barrelPhys->add(flexPhys_A);
	      
	      GeoNameTag * tagFlexC = new GeoNameTag("PP0Flex_C");
	      GeoTransform *xformFlexC = iFlexSrv.getSupportTrfC();
	      GeoPhysVol *flexPhys_C = iFlexSrv.getSupportC();
	      barrelPhys->add(tagFlexC);
	      barrelPhys->add(xformFlexC);
	      barrelPhys->add(flexPhys_C);
	    }
	  }
      }
    }
    else 
      {
	if(m_gmt_mgr->msgLvl(MSG::DEBUG)) m_gmt_mgr->msg(MSG::DEBUG) << "Layer " << ii << " not built" << endmsg;
      }
  }
  
  if(!m_sqliteReader && m_pixServices) {
    //
    // Add the services inside the barrel volume
    //
    m_pixServices->getBuilder()->buildAndPlace("B", barrelPhys);
    if (m_pixServices->getServMatBuilder()) {
      m_pixServices->getServMatBuilder()->buildAndPlace("B", barrelPhys);
    }
    
    //
    // Add the pixel frame inside the barrel volume
    // In recent versions this taken care of in the general services.
    if (m_gmt_mgr->oldFrame()) {
      GeoPixelOldFrame frame (m_DDmgr, m_gmt_mgr, m_sqliteReader, m_mapFPV, m_mapAX);
      frame.BuildInBarrel(barrelPhys);      
    }
  }
  
  // Extra Material
  InDetDD::ExtraMaterial xMat(m_gmt_mgr->distortedMatManager());
  xMat.add(barrelPhys,"PixelBarrel");
  
  return barrelPhys;
}

