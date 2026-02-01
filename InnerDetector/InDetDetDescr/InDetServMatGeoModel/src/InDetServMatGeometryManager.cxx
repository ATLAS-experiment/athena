/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetServMatGeometryManager.h"
#include "InDetGeoModelUtils/InDetMaterialManager.h"
#include "GeoModelUtilities/DecodeVersionKey.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

#include "GaudiKernel/SystemOfUnits.h"

InDetServMatGeometryManager::InDetServMatGeometryManager(InDetDD::AthenaComps * athenaComps)   
  : AthMessaging("InDetServMatGeometryManager")
  , m_athenaComps(athenaComps)
{
  ATH_MSG_DEBUG("Initializing InDetServMatGeometryManager");
   
  const IGeoDbTagSvc *geoDbTag = m_athenaComps->geoDbTagSvc();
  IRDBAccessSvc *rdbSvc = m_athenaComps->rdbAccessSvc();

  // Get version tag and node for Pixel.
  DecodeVersionKey pixelVersionKey(geoDbTag,"Pixel");
  const std::string& pixelDetectorKey  = pixelVersionKey.tag();
  const std::string& pixelDetectorNode = pixelVersionKey.node();

  // Get version tag and node for SCT.
  DecodeVersionKey sctVersionKey(geoDbTag,"SCT");
  const std::string& sctDetectorKey  = sctVersionKey.tag();
  const std::string& sctDetectorNode = sctVersionKey.node();

  //// Get version tag and node for InnerDetector.
  DecodeVersionKey indetVersionKey(geoDbTag,"InnerDetector");
  const std::string& indetDetectorKey  = indetVersionKey.tag();
  const std::string& indetDetectorNode = indetVersionKey.node();
  
/////////////////////////////////////////////////////////
//
// Gets the record sets
//
/////////////////////////////////////////////////////////

  ATH_MSG_DEBUG("Retrieving Pixel Record Sets from database ...");
  ATH_MSG_DEBUG("Pixel: Key = " << pixelDetectorKey << " Node = " << pixelDetectorNode);
  ATH_MSG_DEBUG("SCT:   Key = " << sctDetectorKey   << " Node = " << sctDetectorNode);
  ATH_MSG_DEBUG("InDet: Key = " << indetDetectorKey << " Node = " << indetDetectorNode);
 
  m_InDetWeights       = rdbSvc->getRecordsetPtr("InDetWeights",       indetDetectorKey, indetDetectorNode);

  m_PixelBarrelGeneral = rdbSvc->getRecordsetPtr("PixelBarrelGeneral", pixelDetectorKey, pixelDetectorNode);
  m_PixelEndcapGeneral = rdbSvc->getRecordsetPtr("PixelEndcapGeneral", pixelDetectorKey, pixelDetectorNode);
  m_PixelLayer         = rdbSvc->getRecordsetPtr("PixelLayer",         pixelDetectorKey, pixelDetectorNode);
  m_PixelDisk          = rdbSvc->getRecordsetPtr("PixelDisk",          pixelDetectorKey, pixelDetectorNode);
  m_PixelDiskRing      = rdbSvc->getRecordsetPtr("PixelDiskRing",      pixelDetectorKey, pixelDetectorNode);
  m_PixelStave         = rdbSvc->getRecordsetPtr("PixelStave",         pixelDetectorKey, pixelDetectorNode);
  m_PixelRing          = rdbSvc->getRecordsetPtr("PixelRing",          pixelDetectorKey, pixelDetectorNode);
  m_PixelModule        = rdbSvc->getRecordsetPtr("PixelModule",        pixelDetectorKey, pixelDetectorNode);
  m_PixelReadout       = rdbSvc->getRecordsetPtr("PixelReadout",       pixelDetectorKey, pixelDetectorNode);
  m_PixelWeights       = rdbSvc->getRecordsetPtr("PixelWeights",       pixelDetectorKey, pixelDetectorNode);
  m_PixelEnvelope      = rdbSvc->getRecordsetPtr("PixelEnvelope",      pixelDetectorKey, pixelDetectorNode);
  m_PixelSvcRoute      = rdbSvc->getRecordsetPtr("PixelServiceRoute",  pixelDetectorKey, pixelDetectorNode);

  m_SctBrlGeneral      = rdbSvc->getRecordsetPtr("SSctBrlGeneral",      sctDetectorKey, sctDetectorNode);
  m_SctBrlLayer        = rdbSvc->getRecordsetPtr("SSctBrlLayer",        sctDetectorKey, sctDetectorNode);
  m_SctBrlLadder       = rdbSvc->getRecordsetPtr("SSctBrlLadder",       sctDetectorKey, sctDetectorNode);
  m_SctFwdGeneral      = rdbSvc->getRecordsetPtr("SSctFwdGeneral",      sctDetectorKey, sctDetectorNode);
  m_SctFwdWheel        = rdbSvc->getRecordsetPtr("SSctFwdWheel",        sctDetectorKey, sctDetectorNode);
  m_SctFwdDiscSupport  = rdbSvc->getRecordsetPtr("SSctFwdDiscSupport",  sctDetectorKey, sctDetectorNode);
  m_SctBrlServPerLayer = rdbSvc->getRecordsetPtr("SSctBrlServPerLayer", sctDetectorKey, sctDetectorNode);
  m_SctWeights         = rdbSvc->getRecordsetPtr("SctWeights",         sctDetectorKey, sctDetectorNode);

  m_InDetSimpleServices = rdbSvc->getRecordsetPtr("InDetSimpleServices", indetDetectorKey, indetDetectorNode);

  m_scalingTable = rdbSvc->getRecordsetPtr("InDetServMatScaling", indetDetectorKey, indetDetectorNode);

  m_switches = rdbSvc->getRecordsetPtr("InDetServSwitches", indetDetectorKey, indetDetectorNode);

  m_matMgr = new InDetMaterialManager("InDetServMatMaterialManager", m_athenaComps);
  m_matMgr->addWeightTable(m_InDetWeights, "indet");
  m_matMgr->addWeightTable(m_PixelWeights, "pix");
  m_matMgr->addWeightTable(m_SctWeights,   "sct");
  m_matMgr->addScalingTable(m_scalingTable);

}

InDetServMatGeometryManager::~InDetServMatGeometryManager()
{
  delete m_matMgr;
}

// flag for whether or not to build services
bool InDetServMatGeometryManager::buildServices() const
{
  try {
    if(!(*m_switches)[0]->isFieldNull("BUILDSERVICES")) {
      return (*m_switches)[0]->getInt("BUILDSERVICES");
    }
  }
  catch(std::runtime_error&) {
    ATH_MSG_DEBUG("The switches table has no BUILDSERVICES column");
  }
  return false;
}

int InDetServMatGeometryManager::SupportTubeIndex(const std::string& name) const 
{
  int i=0;
  for(const auto& rec : *m_InDetSimpleServices) {
    if(rec->getString("NAME")==name) return i;
    ++i;
  }
  return -1;
}

double InDetServMatGeometryManager::SupportTubeRMin(const std::string& name) const 
{
  int ind = SupportTubeIndex(name);
  if (ind >= 0) return (*m_InDetSimpleServices)[ind]->getDouble("RMIN");
  return 0;
}

double InDetServMatGeometryManager::SupportTubeRMax(const std::string& name) const 
{
  int ind = SupportTubeIndex(name);
  if (ind >= 0) return (*m_InDetSimpleServices)[ind]->getDouble("RMAX");
  return 0;
}

double InDetServMatGeometryManager::SupportTubeZMin(const std::string& name) const 
{
  int ind = SupportTubeIndex(name);
  if (ind >= 0) (*m_InDetSimpleServices)[ind]->getDouble("ZMIN");
  return 0;
}

double InDetServMatGeometryManager::SupportTubeZMax(const std::string& name) const 
{
  int ind = SupportTubeIndex(name);
  if (ind >= 0) (*m_InDetSimpleServices)[ind]->getDouble("ZMAX");
  return 0;
}

int InDetServMatGeometryManager::SupportTubeExists(const std::string& name) const 
{
  if (SupportTubeIndex(name) != -1) return 1;
  return 0;
}


// number of layers
int InDetServMatGeometryManager::pixelNumLayers() const
{
  return (*m_PixelBarrelGeneral)[0]->getInt("NLAYER");
}

// layer radius 
double InDetServMatGeometryManager::pixelLayerRadius(int layer) const
{
  return (*m_PixelLayer)[layer]->getDouble("RLAYER") * Gaudi::Units::mm;
}

// layer length
double InDetServMatGeometryManager::pixelLayerLength(int layer) const
{
  int staveIndex = (*m_PixelLayer)[layer]->getInt("STAVEINDEX");
  return (*m_PixelStave)[staveIndex]->getDouble("ENVLENGTH") * Gaudi::Units::mm;
}

// Number of staves/sectors per barrel layer 
int InDetServMatGeometryManager::pixelNumSectorsForLayer(int layer) const
{
  return (*m_PixelLayer)[layer]->getInt("NSECTORS");
}

// Number of modules per stave
int InDetServMatGeometryManager::pixelModulesPerStave(int layer) const
{
  int staveIndex = (*m_PixelLayer)[layer]->getInt("STAVEINDEX");
  return (*m_PixelStave)[staveIndex]->getInt("NMODULE");
}

// Bent stave (conical layout) parameters
double InDetServMatGeometryManager::pixelLadderBentStaveAngle(int layer)  const
{
  int staveIndex = (*m_PixelLayer)[layer]->getInt("STAVEINDEX");
  try {
    if(!(*m_PixelStave)[staveIndex]->isFieldNull("BENTSTAVEANGLE")) {
      return (*m_PixelStave)[staveIndex]->getDouble("BENTSTAVEANGLE");
    }
  }
  catch(std::runtime_error&) {
    ATH_MSG_DEBUG("No value for the BENTSTAVEANGLE column in the PixelStave table record " << staveIndex);
  }
  return 0.;
}

int InDetServMatGeometryManager::pixelBentStaveNModule(int layer) const
{
  int staveIndex = (*m_PixelLayer)[layer]->getInt("STAVEINDEX");
  try {
    if(!(*m_PixelStave)[staveIndex]->isFieldNull("BENTSTAVENMODULE")) {
      return (*m_PixelStave)[staveIndex]->getDouble("BENTSTAVENMODULE");
    }
  }
  catch(std::runtime_error&) {
    ATH_MSG_DEBUG("No value for the BENTSTAVEMODULE column in the PixelStave table record " << staveIndex);
  }
  return 0.;  
}

double InDetServMatGeometryManager::pixelLadderModuleDeltaZ(int layer) const
{
  int staveIndex = (*m_PixelLayer)[layer]->getInt("STAVEINDEX");
  return (*m_PixelStave)[staveIndex]->getDouble("MODULEDZ");
}

// Number of staves/sectors per endcap layer 
int InDetServMatGeometryManager::pixelEndcapNumSectorsForLayer(int layer) const
{
  if(!(*m_PixelDisk)[layer]->isFieldNull("NSECTORS")) {
    return (*m_PixelDisk)[layer]->getInt("NSECTORS");
  }
  return 0;
}

int InDetServMatGeometryManager::pixelModulesPerRing( int ring) const
{
  return (*m_PixelRing)[ring]->getInt("NMODULE");
}

int InDetServMatGeometryManager::pixelModulesPerEndcapSector( int layer) const
{
  int nModulesDisk = 0;
  for(const auto& diskRing : *m_PixelDiskRing) {
    int disk = diskRing->getInt("DISK");
    if (disk == layer) {
      int ring = diskRing->getInt("RING");
      nModulesDisk += pixelModulesPerRing( ring);
    }
  }
  if(pixelEndcapNumSectorsForLayer(layer)==0) return 0;
  return nModulesDisk / pixelEndcapNumSectorsForLayer(layer);
}

int InDetServMatGeometryManager::pixelChipsPerModuleForDisk( int layer) const
{
  int sumChips = 0;
  int sumModules = 0;
  for(const auto& diskRing : *m_PixelDiskRing) {
    int	disk = diskRing->getInt("DISK");
    if (disk == layer) {
      int ring = diskRing->getInt("RING");
      int moduleType = diskRing->getInt("MODULETYPE");
      int nModules = pixelModulesPerRing(ring);
      sumModules += nModules;
      sumChips += nModules * pixelChipsPerModule(moduleType);
    }
  }
  if(sumModules==0) return 0;
  if (sumChips % sumModules == 0) return sumChips/sumModules;
  else return 1 + sumChips/sumModules; // round to larger integer
}

// number of disks
int InDetServMatGeometryManager::pixelNumDisks() const
{
  return (*m_PixelEndcapGeneral)[0]->getInt("NDISK");
}

// disk Z position
double InDetServMatGeometryManager::pixelDiskZ(int disk) const 
{
  return (*m_PixelDisk)[disk]->getDouble("ZDISK") * Gaudi::Units::mm;
}

// disk min radius
double InDetServMatGeometryManager::pixelDiskRMin(int disk) const 
{
  std::string route = pixelDiskServiceRoute(disk);   
  if(route=="StdRoute")
    return (*m_PixelDisk)[disk]->getDouble("RMIN") * Gaudi::Units::mm - 11*Gaudi::Units::mm;

  // support structures - SUP1RMIN is always closest to centre
  return (*m_PixelDisk)[disk]->getDouble("SUP1RMIN") * Gaudi::Units::mm;

}

// disk max radius
double InDetServMatGeometryManager::pixelDiskRMax(int disk) const 
{
  std::string route = pixelDiskServiceRoute(disk);   
  if(route=="StdRoute")
    return (*m_PixelDisk)[disk]->getDouble("RMAX") * Gaudi::Units::mm + 11*Gaudi::Units::mm;

  // support structures - SUP3RMAX is always furthest from centre
  return (*m_PixelDisk)[disk]->getDouble("SUP3RMAX") * Gaudi::Units::mm;

}

// EOS ZOffset
double InDetServMatGeometryManager::pixelDiskEOSZOffset(int disk) const 
{
  try {
    if(disk >=0
       && static_cast<unsigned>(disk) < m_PixelSvcRoute->size()
       && !(*m_PixelSvcRoute)[disk]->isFieldNull("EOSZOFFSET")) {
      return (*m_PixelSvcRoute)[disk]->getDouble("EOSZOFFSET");
    }
  }
  catch(std::runtime_error&) {
    ATH_MSG_DEBUG("No EOSZOFFSET value for the PixelSvcRoute table record " << disk);
  }
  return 0.0;
}

// return name of support tube where 
std::string InDetServMatGeometryManager::pixelDiskServiceRoute(int disk) const 
{
  try {
    if(disk >=0
       && static_cast<unsigned>(disk) < m_PixelSvcRoute->size()
       && !(*m_PixelSvcRoute)[disk]->isFieldNull("SERVICEROUTE")) {
      return (*m_PixelSvcRoute)[disk]->getString("SERVICEROUTE");
    }
  }
  catch (std::runtime_error&) {
    ATH_MSG_DEBUG("No SERVICEROUTE value for the PixelSvcRoute table record " << disk);
  }
  return "StdRoute";
}

double InDetServMatGeometryManager::pixelEnvelopeRMax() const
{
  return (*m_PixelEnvelope)[0]->getDouble("RMAX") * Gaudi::Units::mm;
}

int InDetServMatGeometryManager::pixelBarrelModuleType( int layer) const 
{
  return (*m_PixelLayer)[layer]->getInt("MODULETYPE");
}

int InDetServMatGeometryManager::pixelDesignType( int moduleType) const
{
  return (*m_PixelModule)[moduleType]->getInt("DESIGNTYPE");
}

int InDetServMatGeometryManager::pixelChipsPerModule( int moduleType) const 
{
  int nChipsEta = (*m_PixelReadout)[moduleType]->getInt("NCHIPSETA");
  int nChipsPhi = (*m_PixelReadout)[moduleType]->getInt("NCHIPSPHI");
  return nChipsEta*nChipsPhi;
}

// number of layers
int InDetServMatGeometryManager::sctNumLayers() const
{
  return (*m_SctBrlGeneral)[0]->getInt("NUMLAYERS");
}

// layer radius 
double InDetServMatGeometryManager::sctLayerRadius(int layer) const
{
  return (*m_SctBrlLayer)[layer]->getDouble("RADIUS") * Gaudi::Units::mm;
}

// layer length
double InDetServMatGeometryManager::sctLayerLength(int layer) const
{
  return (*m_SctBrlLayer)[layer]->getDouble("CYLLENGTH") * Gaudi::Units::mm;
}

// layer type. Long(0) or Short (1) strips. NEEDS CHECKING
int InDetServMatGeometryManager::sctLayerType(int layer) const
{
  int ladType = (*m_SctBrlLayer)[layer]->getInt("LADDERTYPE");
  return (*m_SctBrlLadder)[ladType]->getInt("MODTYPE");
}

// Number of staves/sectors per barrel layer 
int InDetServMatGeometryManager::sctNumSectorsForLayer(int layer) const
{
  return (*m_SctBrlLayer)[layer]->getInt("SKISPERLAYER");
}

int InDetServMatGeometryManager::sctModulesPerLadder(int layer) const
{
  int ladType = (*m_SctBrlLayer)[layer]->getInt("LADDERTYPE");
  return (*m_SctBrlLadder)[ladType]->getInt("NUMPERLADDER");
}

// Number of staves/sectors per endcap layer 
int InDetServMatGeometryManager::sctEndcapNumSectorsForLayer(int /*layer*/) const
{
  return 32; // FIXME: hardwired number, should go to text file and DB
}

// number of disks
int InDetServMatGeometryManager::sctNumDisks() const
{
  return (*m_SctFwdGeneral)[0]->getInt("NUMWHEELS");
}

// disk Z position
double InDetServMatGeometryManager::sctDiskZ(int disk) const 
{
  return (*m_SctFwdWheel)[disk]->getDouble("ZPOSITION") * Gaudi::Units::mm;
}

// disk Z position
double InDetServMatGeometryManager::sctDiskRMax(int disk) const 
{
  return (*m_SctFwdDiscSupport)[disk]->getDouble("OUTERRADIUS") * Gaudi::Units::mm;
}

double InDetServMatGeometryManager::sctInnerSupport() const 
{
  return (*m_SctBrlServPerLayer)[0]->getDouble("SUPPORTCYLINNERRAD") * Gaudi::Units::mm;
}

