/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/



#include "GeoPixelPigtail.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoTransform.h"
#include <utility>
GeoPixelPigtail::GeoPixelPigtail(InDetDD::PixelDetectorManager* ddmgr,
                                 PixelGeometryManager* mgr,
				 GeoModelIO::ReadGeoModel* sqliteReader,
                                 std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                                 std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX)
  : GeoVPixelFactory (ddmgr, mgr, sqliteReader, std::move(mapFPV), std::move(mapAX))
{
}

GeoVPhysVol* GeoPixelPigtail::Build() {
  //this block was originally in the c'tor
  const GeoMaterial* air  = m_mat_mgr->getMaterial("std::Air");
  double length = m_gmt_mgr->PixelPigtailEnvelopeLength()+m_epsilon;
  double rmin = m_gmt_mgr->PixelPigtailBendRMin();
  double rmax = m_gmt_mgr->PixelPigtailBendRMax();
  double phi  = m_gmt_mgr->PixelPigtailBendPhiMin();
  double dphi = m_gmt_mgr->PixelPigtailBendPhiMax() -  phi;
  double width = m_gmt_mgr->PixelPigtailWidthZ();
  double posZ = m_gmt_mgr->PixelPigtailPosZ();

  double rminEps = rmin-m_epsilon;
  double rmaxEps = rmax+m_epsilon;
  const GeoTubs* tubs = new GeoTubs(rminEps,rmaxEps,0.5*length,phi,dphi);
  GeoLogVol * thePigtail = new GeoLogVol("PigtailCylEnv",tubs,air);
  //
  GeoPhysVol* pigtailPhys = new GeoPhysVol(thePigtail);
  const std::string pigtailStr{"PigtailCyl"};
  for (int iModule = 0; iModule<m_gmt_mgr->PixelNModule(); iModule++) {
    int moduleEta =  m_gmt_mgr->PixelModuleEtaFromIndex(iModule);
    double zShift = m_gmt_mgr->PixelModuleZPosition(moduleEta);
    const GeoTubs* tubs  = new GeoTubs(rmin,rmax,0.5*width,phi,dphi);
    std::string matName = m_gmt_mgr->getMaterialName(pigtailStr, m_gmt_mgr->GetLD());
    const GeoMaterial* cyliMat = m_mat_mgr->getMaterialForVolume(matName,tubs->volume());
    GeoLogVol* theTubs   = new GeoLogVol(pigtailStr,tubs,cyliMat);
    GeoPhysVol* pigtailPart = new GeoPhysVol(theTubs);

    double zpos = posZ + zShift;
    GeoTransform* xform = new GeoTransform(GeoTrf::TranslateZ3D(zpos));
    pigtailPhys->add(xform);
    pigtailPhys->add(pigtailPart);
  }
  return pigtailPhys;
}
