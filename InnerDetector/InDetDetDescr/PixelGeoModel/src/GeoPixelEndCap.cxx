/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoPixelEndCap.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoPixelDisk.h"
#include "GeoPixelECCable.h"
#include "GeoPixelServices.h"
#include "Identifier/Identifier.h"
#include "InDetGeoModelUtils/ExtraMaterial.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include <sstream>
#include <utility>

#include "InDetGeoModelUtils/VolumeBuilder.h"

GeoPixelEndCap::GeoPixelEndCap(InDetDD::PixelDetectorManager* ddmgr,
                               PixelGeometryManager* mgr,
			       GeoModelIO::ReadGeoModel* sqliteReader,
                               std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                               std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX,
                               GeoPixelServices * pixServices)
  : GeoVPixelFactory(ddmgr, mgr, sqliteReader, std::move(mapFPV), std::move(mapAX)),
    m_pixServices(pixServices)
{}

GeoVPhysVol* GeoPixelEndCap::Build( ) {

  GeoFullPhysVol* ecPhys{nullptr};
  if(!m_sqliteReader) {
    //
    // create the Barrel Mother volume
    // 
    double rmin = m_gmt_mgr->PixelEndcapRMin();
    double rmax = m_gmt_mgr->PixelEndcapRMax();
    double halflength = (m_gmt_mgr->PixelEndcapZMax()-m_gmt_mgr->PixelEndcapZMin())/2.;
    const GeoMaterial* air = m_mat_mgr->getMaterial("std::Air");
    const GeoTube* ecTube = new GeoTube(rmin,rmax,halflength);
    const GeoLogVol* ecLog = new GeoLogVol("EndCap",ecTube,air);
    ecPhys = new GeoFullPhysVol(ecLog);
  }

  int ndisks = m_gmt_mgr->PixelEndcapNDisk();
  m_gmt_mgr->msg(MSG::DEBUG) << "Number of disks is " << ndisks << endmsg;
  
  // Set numerology
  m_DDmgr->numerology().setNumDisks(ndisks);

  // position of the endcap
  double endcapZOffset = 0.5* (m_gmt_mgr->PixelEndcapZMax()+m_gmt_mgr->PixelEndcapZMin());
  
  //
  // Place the disks and cables on both sides
  //
  GeoPixelDisk * pd = new GeoPixelDisk (m_DDmgr, m_gmt_mgr, m_sqliteReader, m_mapFPV, m_mapAX);
  GeoPixelECCable * pecc = new GeoPixelECCable (m_DDmgr, m_gmt_mgr, m_sqliteReader, m_mapFPV, m_mapAX);
  for(int idisk = 0; idisk < ndisks; idisk++) {
    m_gmt_mgr->SetCurrentLD(idisk);
    // Some method is accessing the eta before the disk is built so we set it 
    // to a valid value.
    m_gmt_mgr->SetEta(0);
    if(m_gmt_mgr->isLDPresent()) {
      if(m_sqliteReader) {
	pd->Build();
	int brl_ec = 2*m_gmt_mgr->GetSide();
	Identifier id = m_gmt_mgr->getIdHelper()->wafer_id(brl_ec,idisk,0,0);
	std::string key="Disk_"+std::to_string(brl_ec)+"_"+std::to_string(idisk)+"_"+std::to_string(m_gmt_mgr->GetLD())+"_"+std::to_string(m_gmt_mgr->Phi())+"_"+std::to_string(m_gmt_mgr->Eta());
	m_DDmgr->addAlignableTransform(1, id, (*m_mapAX)[key], (*m_mapFPV)[key]);
      }
      else {
      
	//
	// The position is given w.r.t to the center of the detector!
	//
	//position of the disk
	double zdisk = m_gmt_mgr->PixelDiskZPosition()-endcapZOffset;
	// place the disk
	std::ostringstream nameTag; 
	nameTag << "Disk" << idisk;
	GeoNameTag * tag = new GeoNameTag(nameTag.str());
	GeoAlignableTransform* xform = new GeoAlignableTransform(GeoTrf::TranslateZ3D(zdisk));
	
	GeoVPhysVol * diskPhys = pd->Build();
	ecPhys->add(tag);
	ecPhys->add(new GeoIdentifierTag(idisk));
	ecPhys->add(xform);
	ecPhys->add(diskPhys);
	
	// Store the alignable transform
	int brl_ec = 2*m_gmt_mgr->GetSide();
	Identifier id = m_gmt_mgr->getIdHelper()->wafer_id(brl_ec,idisk,0,0);
	m_DDmgr->addAlignableTransform(1, id, xform, diskPhys);
	
	//
	// place the cables twice for the two active parts
	//
	double dz = pd->Thickness()/2. + m_gmt_mgr->PixelECCablesDistance() ;
	GeoTransform * xformCablesPlus = new GeoTransform(GeoTrf::TranslateZ3D(zdisk+dz));
	ecPhys->add(xformCablesPlus);
	ecPhys->add(pecc->Build() );
	GeoTransform * xformCablesMinus = new GeoTransform(GeoTrf::TranslateZ3D(zdisk-dz));
	ecPhys->add(xformCablesMinus);
	ecPhys->add(pecc->Build() );
      }
    } else {
      if(m_gmt_mgr->msgLvl(MSG::DEBUG))	m_gmt_mgr->msg(MSG::DEBUG) << "Disk " << idisk << " not built" << endmsg;
    }
  }
  delete pd;
  delete pecc;

  std::string region = m_gmt_mgr->GetSide()>0 ? "A" : "C";

  if(m_sqliteReader) {
    std::ostringstream ename;
    ename << "EndCap" << region;
    ecPhys = (*m_mapFPV)[ename.str()];
  }
  else if(m_pixServices) {
    //
    // Add the Endcap services inide the EC volume
    //  
    m_pixServices->getBuilder()->buildAndPlace(region, ecPhys, endcapZOffset);
    if (m_pixServices->getServMatBuilder()) {
      m_pixServices->getServMatBuilder()->buildAndPlace(region, ecPhys, endcapZOffset);
    }
  }

  {
    // Extra Material
    InDetDD::ExtraMaterial xMat(m_gmt_mgr->distortedMatManager());
    xMat.add(ecPhys,"PixelEndcap",endcapZOffset);
    if (m_gmt_mgr->GetSide()>0) { // EndcapA
      xMat.add(ecPhys,"PixelEndcapA",endcapZOffset);
    } else {            // EndcapC
      xMat.add(ecPhys,"PixelEndcapC",endcapZOffset);
    }
  }

  return ecPhys;
}
