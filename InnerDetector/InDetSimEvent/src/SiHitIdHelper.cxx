/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <mutex>

#include "InDetSimEvent/SiHitIdHelper.h"
#include "StoreGate/StoreGateSvc.h"
#include "InDetIdentifier/PixelID.h"
#include "GaudiKernel/ServiceHandle.h"


//
// private constructor
SiHitIdHelper::SiHitIdHelper() :HitIdHelper() {
  Initialize();
}

const SiHitIdHelper* SiHitIdHelper::GetHelper() {
  static const SiHitIdHelper helper;
  return &helper;
}

void SiHitIdHelper::Initialize() {

  const PixelID* pix = nullptr;
  ServiceHandle<StoreGateSvc> detStore ("DetectorStore", "SiHitIdHelper");
  if (detStore.retrieve().isSuccess()) {
    if (detStore->retrieve(pix, "PixelID").isFailure()) { pix = nullptr; }
  }

  bool isDBM  = (pix != nullptr && pix->dictionaryVersion() == "IBL-DBM");
  // check for ITk and HGTD
  bool isITkHGTD = (pix !=nullptr &&  pix->dictionaryVersion() == "ITkHGTD");
  // we might include PLR as well, then we have to increase endcap range to +/- 4
  bool isITkHGTDPLR = (pix !=nullptr &&  pix->dictionaryVersion() == "ITkHGTDPLR");
  // new identification scheme for HGTD (endcap-layer-moduleinlayer)
  bool isITk_HGTD_NewID_PLR = (pix !=nullptr &&  pix->dictionaryVersion() == "P2-RUN4");
  // cache the HL-LHC decision
  m_isITkHGTD = isITkHGTD || isITkHGTDPLR || isITk_HGTD_NewID_PLR;

  if (isITkHGTD) InitializeField("Part",0,2);
  else if (isITkHGTDPLR || isITk_HGTD_NewID_PLR) InitializeField("Part",0,3);
  else InitializeField("Part",0,1);
  if (isDBM || isITkHGTDPLR || isITk_HGTD_NewID_PLR) InitializeField("BarrelEndcap",-4,4);
  else InitializeField("BarrelEndcap",-2,2);
  InitializeField("LayerDisk",0,20);
  if (m_isITkHGTD) InitializeField("EtaModule",-100,100);
  else InitializeField("EtaModule",-20,20);
  if (isITk_HGTD_NewID_PLR) InitializeField("PhiModule",0,1022);
  else InitializeField("PhiModule",0,200);
  InitializeField("Side",0,3);

}

// Info retrieval:
// Pixel, SCT, HGTD, or PLR
bool SiHitIdHelper::isPixel(const int& hid) const
{
  int psh = this->GetFieldValue("Part", hid);
  return psh ==0;
}

bool SiHitIdHelper::isSCT(const int& hid) const
{
  int psh = this->GetFieldValue("Part", hid);
  return psh ==1;
}

bool SiHitIdHelper::isHGTD(const int& hid) const
{
  int psh = this->GetFieldValue("Part", hid);
  return psh ==2;
}

bool SiHitIdHelper::isPLR(const int& hid) const
{
  if (!m_isITkHGTD) return false;

  int psh = this->GetFieldValue("BarrelEndcap", hid);
  return std::abs(psh) == 4;
}


// Barrel or Endcap
int SiHitIdHelper::getBarrelEndcap(const int& hid) const
{
  return this->GetFieldValue("BarrelEndcap", hid);
}

// Layer/Disk
int SiHitIdHelper::getLayerDisk(const int& hid) const
{
  return this->GetFieldValue("LayerDisk", hid);
}

// eta module
int SiHitIdHelper::getEtaModule(const int& hid) const
{
  return this->GetFieldValue("EtaModule", hid);
}

// phi module
int SiHitIdHelper::getPhiModule(const int& hid) const
{
  return this->GetFieldValue("PhiModule", hid);
}

// side
int SiHitIdHelper::getSide(const int& hid) const
{
  return this->GetFieldValue("Side", hid);
}


//
// Info packing:
int SiHitIdHelper::buildHitId(const int Part, const int BrlECap, const int LayerDisk,
                              const int etaM, const int phiM, const int side) const
{
  int theID(0);
  this->SetFieldValue("Part",           Part, theID);
  this->SetFieldValue("BarrelEndcap",   BrlECap, theID);
  this->SetFieldValue("LayerDisk",      LayerDisk, theID);
  this->SetFieldValue("EtaModule",      etaM, theID);
  this->SetFieldValue("PhiModule",      phiM, theID);
  this->SetFieldValue("Side",           side, theID);
  return theID;
}

int SiHitIdHelper::buildHitIdFromStringITk(int part, const std::string& physVolName) const
{
    int brlEcap = 0;
    int layerDisk = 0;
    int etaMod = 0;
    int phiMod = 0;
    int side = 0;
    //Extract the indices from the name, and write them in to the matching int
    std::map<std::string, int&> fields{{"barrel_endcap",brlEcap},{"layer_wheel",layerDisk},{"phi_module",phiMod},{"eta_module",etaMod},{"side",side}};
    for(const auto & field:fields){
        size_t pos1 = (physVolName).find(field.first+"_");
        size_t pos2 = (physVolName).find("_",pos1+field.first.size()+1);//start looking only after end of first delimiter (plus 1 for the "_" appended) ends
        std::string strNew = (physVolName).substr(pos1+field.first.size()+1,pos2-(pos1+field.first.size()+1));
        field.second = std::stoi(strNew);
    }
    return buildHitId(part,brlEcap,layerDisk,etaMod,phiMod,side);
}

int SiHitIdHelper::buildHitIdFromStringHGTD(int part, const std::string& physVolName) const
{
    int endcap = 0;
    int layer = 0;
    int moduleInLayer = 0;
    //Extract the indices from the name, and write them in to the matching int
    std::map<std::string, int&> fields{{"endcap",endcap},{"layer",layer},{"moduleInLayer",moduleInLayer}};
    for(const auto & field:fields){
        size_t pos1 = (physVolName).find(field.first+"_");
        size_t pos2 = (physVolName).find("_",pos1+field.first.size()+1);//start looking only after end of first delimiter (plus 1 for the "_" appended) ends
        std::string strNew = (physVolName).substr(pos1+field.first.size()+1,pos2-(pos1+field.first.size()+1));
        field.second = std::stoi(strNew);
    }
    return buildHitId(part,endcap,layer,0,moduleInLayer,0);
}
