/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RDBMaterialManager.h"
#include "GeoModelUtilities/DecodeVersionKey.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"

#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/Units.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "AthenaBaseComps/AthCheckMacros.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

RDBMaterialManager::RDBMaterialManager(ISvcLocator* pSvcLocator)
  : AthMessaging("GeoModelSvc::RDBMaterialManager")
{
  if(!readMaterialsFromDB(pSvcLocator).isSuccess()) {
    throw std::runtime_error("RDBMaterialManager failed to read Geometry DB");
  }
}

StatusCode RDBMaterialManager::readMaterialsFromDB(ISvcLocator* pSvcLocator)
{
  SmartIF<IGeoModelSvc> iGeoModel{pSvcLocator->service("GeoModelSvc")};
  ATH_CHECK( iGeoModel.isValid() );

  SmartIF<IRDBAccessSvc> iAccessSvc{pSvcLocator->service("RDBAccessSvc")};
  ATH_CHECK( iAccessSvc.isValid() );
  
  const bool loadDefaults = iGeoModel->geoConfig() != GeoModel::GEO_RUN4;
  auto defaulted = [=](const IRDBRecordset_ptr pRecordset) -> bool{
    return (loadDefaults and pRecordset->size() == 0);
  };
  // Do not load defaults for RUN4
  if (loadDefaults) ATH_MSG_DEBUG("Will load material defaults if not present");

  // --- Elements
  DecodeVersionKey keyAtlas(iGeoModel, "ATLAS");
  m_elements = iAccessSvc->getRecordsetPtr("Elements",keyAtlas.tag(),keyAtlas.node());
  if(defaulted(m_elements)) {
    ATH_MSG_WARNING("Getting Elements with default tag");
    m_elements = iAccessSvc->getRecordsetPtr("Elements","Materials-00","Materials");
  }

  IRDBRecordset_ptr recMaterials{};
  IRDBRecordset_ptr recMatcomponents{};

  // --- Standard materials
  recMatcomponents = iAccessSvc->getRecordsetPtr("StdMatComponents",keyAtlas.tag(),keyAtlas.node());
  if(defaulted(recMatcomponents))	{
    ATH_MSG_WARNING("Getting StdMatComponents with default tag");
    recMatcomponents = iAccessSvc->getRecordsetPtr("StdMatComponents","Materials-00","Materials");
  }
  recMaterials = iAccessSvc->getRecordsetPtr("StdMaterials",keyAtlas.tag(),keyAtlas.node());
  if(defaulted(recMaterials)) {
    ATH_MSG_WARNING("Getting StdMaterials with default tag");
    recMaterials = iAccessSvc->getRecordsetPtr("StdMaterials","Materials-00","Materials");
  }
  m_detData.emplace("std",DetectorAuxData{"STDMATERIALS_DATA_ID", recMaterials, recMatcomponents});
  
  // --- Pixel materials
  DecodeVersionKey keyPixel(iGeoModel, "Pixel");
  recMatcomponents = iAccessSvc->getRecordsetPtr("PixMatComponents",keyPixel.tag(),keyPixel.node());
  if(defaulted(recMatcomponents)) {
    ATH_MSG_WARNING("Getting PixMatComponents with default tag");
    recMatcomponents = iAccessSvc->getRecordsetPtr("PixMatComponents","PixMatComponents-00");
  }
  recMaterials = iAccessSvc->getRecordsetPtr("PixMaterials",keyPixel.tag(),keyPixel.node());
  if(defaulted(recMaterials)) {
    ATH_MSG_WARNING("Getting PixMaterials with default tag");
    recMaterials = iAccessSvc->getRecordsetPtr("PixMaterials","PixMaterials-00");
  }
  m_detData.emplace("pix",DetectorAuxData{"PIXMATERIALS_DATA_ID", recMaterials, recMatcomponents});
  
  // --- SCT materials
  DecodeVersionKey keySCT(iGeoModel, "SCT");
  recMatcomponents = iAccessSvc->getRecordsetPtr("SCTMatComponents",keySCT.tag(),keySCT.node());
  if(defaulted(recMatcomponents))	{
    ATH_MSG_WARNING("Getting SCTMatComponents with default tag");
    recMatcomponents = iAccessSvc->getRecordsetPtr("SCTMatComponents","SCTMatComponents-00");
  }
  recMaterials = iAccessSvc->getRecordsetPtr("SCTMaterials",keySCT.tag(),keySCT.node());
  if(defaulted(recMaterials)) {
    ATH_MSG_WARNING("Getting SCTMaterials with default tag");
    recMaterials = iAccessSvc->getRecordsetPtr("SCTMaterials","SCTMaterials-00");
  }
  m_detData.emplace("sct",DetectorAuxData{"SCTMATERIALS_DATA_ID", recMaterials, recMatcomponents});
  
  // --- TRT materials
  DecodeVersionKey keyTRT(iGeoModel, "TRT");
  recMatcomponents = iAccessSvc->getRecordsetPtr("TrtMatComponents",keyTRT.tag(),keyTRT.node());
  if(defaulted(recMatcomponents))	{
    ATH_MSG_WARNING("Getting TrtMatComponents with default tag");
    recMatcomponents = iAccessSvc->getRecordsetPtr("TrtMatComponents","TrtMatComponents-00");
  }
  recMaterials = iAccessSvc->getRecordsetPtr("TrtMaterials",keyTRT.tag(),keyTRT.node());
  if(defaulted(recMaterials)) {
    ATH_MSG_WARNING("Getting TrtMaterials with default tag");
    recMaterials = iAccessSvc->getRecordsetPtr("TrtMaterials","TrtMaterials-00");
  }
  m_detData.emplace("trt",DetectorAuxData{"TRTMATERIALS_DATA_ID", recMaterials, recMatcomponents});
  
  // --- InDet common materials
  DecodeVersionKey keyInDet(iGeoModel, "InnerDetector");
  recMatcomponents = iAccessSvc->getRecordsetPtr("InDetMatComponents",keyInDet.tag(),keyInDet.node());
  if(defaulted(recMatcomponents)) {
    ATH_MSG_DEBUG("Getting InDetMatComponents with default tag");
    recMatcomponents = iAccessSvc->getRecordsetPtr("InDetMatComponents","InDetMatComponents-00");
  }
  recMaterials = iAccessSvc->getRecordsetPtr("InDetMaterials",keyInDet.tag(),keyInDet.node());
  if(defaulted(recMaterials)) {
    ATH_MSG_DEBUG("Getting InDetMaterials with default tag");
    recMaterials = iAccessSvc->getRecordsetPtr("InDetMaterials","InDetMaterials-00");
  }
  m_detData.emplace("indet",DetectorAuxData{"INDETMATERIALS_DATA_ID", recMaterials, recMatcomponents});
  
  // --- LAr materials
  DecodeVersionKey keyLAr(iGeoModel, "LAr");    
  recMatcomponents = iAccessSvc->getRecordsetPtr("LArMatComponents",keyLAr.tag(),keyLAr.node());
  if(defaulted(recMatcomponents)) {
    ATH_MSG_WARNING("Getting LArMatComponents with default tag");
    recMatcomponents = iAccessSvc->getRecordsetPtr("LArMatComponents","LArMatComponents-00");
  }
  recMaterials = iAccessSvc->getRecordsetPtr("LArMaterials",keyLAr.tag(),keyLAr.node());
  if(defaulted(recMaterials)) {
    ATH_MSG_WARNING("Getting LArMaterials with default tag");
    recMaterials = iAccessSvc->getRecordsetPtr("LArMaterials","LArMaterials-00");
  }
  m_detData.emplace("LAr",DetectorAuxData{"LARMATERIALS_DATA_ID", recMaterials, recMatcomponents});
  
  // --- Tile materials
  DecodeVersionKey keyTile(iGeoModel, "TileCal");    
  recMatcomponents = iAccessSvc->getRecordsetPtr("TileMatComponents",keyTile.tag(),keyTile.node());
  if (defaulted(recMatcomponents)) {
    ATH_MSG_WARNING("Getting TileMatComponents with default tag" );
    recMatcomponents = iAccessSvc->getRecordsetPtr("TileMatComponents","TileMatComponents-00");
  }
  recMaterials = iAccessSvc->getRecordsetPtr("TileMaterials",keyTile.tag(),keyTile.node());
  if(defaulted(recMaterials)) {
    ATH_MSG_WARNING("Getting TileMaterials with default tag");
    recMaterials = iAccessSvc->getRecordsetPtr("TileMaterials","TileMaterials-00");
  }
  m_detData.emplace("tile",DetectorAuxData{"TILEMATERIALS_DATA_ID", recMaterials, recMatcomponents});
  
  // --- Muon
  DecodeVersionKey keyMuon(iGeoModel, "MuonSpectrometer");
  recMatcomponents = iAccessSvc->getRecordsetPtr("MUOMatComponents",keyMuon.tag(),keyMuon.node());
  if(defaulted(recMatcomponents))	{
    ATH_MSG_WARNING("Getting MUOMatComponents with default tag");
    recMatcomponents = iAccessSvc->getRecordsetPtr("MUOMatComponents","MUOMatComponents-00");
  }
  recMaterials = iAccessSvc->getRecordsetPtr("MUOMaterials",keyMuon.tag(),keyMuon.node());
  if(defaulted(recMaterials)) {
    ATH_MSG_WARNING("Getting MUOMaterials with default tag" );
    recMaterials = iAccessSvc->getRecordsetPtr("MUOMaterials","MUOMaterials-00");
  }
  m_detData.emplace("muo",DetectorAuxData{"MUOMATERIALS_DATA_ID", recMaterials, recMatcomponents});

  // --- Shield
  recMatcomponents = iAccessSvc->getRecordsetPtr("ShieldMatComponents",keyMuon.tag(),keyMuon.node());
  if(defaulted(recMatcomponents)) {
    ATH_MSG_WARNING("Getting ShieldMatComponents with default tag");
    recMatcomponents = iAccessSvc->getRecordsetPtr("ShieldMatComponents","ShieldMatComponents-00");
  }
  recMaterials = iAccessSvc->getRecordsetPtr("ShieldMaterials",keyMuon.tag(),keyMuon.node());
  if(defaulted(recMaterials)) {
    ATH_MSG_WARNING("Getting ShieldMaterials with default tag");
    recMaterials = iAccessSvc->getRecordsetPtr("ShieldMaterials","ShieldMaterials-00");
  }
  m_detData.emplace("shield",DetectorAuxData{"SHIELDMATERIALS_DATA_ID", recMaterials, recMatcomponents});

  // --- Toro
  recMatcomponents = iAccessSvc->getRecordsetPtr("ToroMatComponents",keyMuon.tag(),keyMuon.node());
  if(defaulted(recMatcomponents)) {
    ATH_MSG_WARNING("Getting ToroMatComponents with default tag");
    recMatcomponents = iAccessSvc->getRecordsetPtr("ToroMatComponents","ToroMatComponents-00");
  }
  recMaterials = iAccessSvc->getRecordsetPtr("ToroMaterials",keyMuon.tag(),keyMuon.node());
  if(defaulted(recMaterials)) {
    ATH_MSG_WARNING("Getting ToroMaterials with default tag");
    recMaterials = iAccessSvc->getRecordsetPtr("ToroMaterials","ToroMaterials-00");
  }
  m_detData.emplace("toro",DetectorAuxData{"TOROMATERIALS_DATA_ID", recMaterials, recMatcomponents});

  return StatusCode::SUCCESS;
}

// Destructor:
RDBMaterialManager::~RDBMaterialManager() = default;

GeoMaterial* RDBMaterialManager::searchMaterialMap(const std::string & name) const
{
  MaterialMapIterator m = m_materialMap.find(name);

  if (m!=m_materialMap.end()) {
    ATH_MSG_VERBOSE(" ***** in searchMaterialMap(): search success for " << name);
    return (*m).second;
  }

  ATH_MSG_VERBOSE(" ***** in searchMaterialMap(): search failed for "  << name);
  return nullptr;
}


GeoElement *RDBMaterialManager::searchElementVector(const std::string & name)  const
{
  GeoEleVec::const_iterator it_element = std::find_if(m_elementVector.begin()
						      , m_elementVector.end()
						      , [&name](const GeoElement* element) {
							return name == element->getName();
						      });

  if (it_element != m_elementVector.end()) {
    ATH_MSG_VERBOSE(" ***** in searchElementVector() search success for "  << name);
    return *it_element;
  }

  ATH_MSG_VERBOSE(" ***** in searchElementVector() search failed for "  << name);
  return nullptr;
}


GeoElement *RDBMaterialManager::searchElementVector(const unsigned int atomicNumber) const
{ 
  GeoEleVec::const_iterator it_element = std::find_if(m_elementVector.begin()
						      , m_elementVector.end()
						      , [atomicNumber](const GeoElement* element) {
							return atomicNumber == element->getZ();
						      });

  if (it_element != m_elementVector.end()) {
    ATH_MSG_VERBOSE(" ***** in searchElementVector(atomicNumber) search success for atomic number "  << atomicNumber);
    return *it_element;
  }
 
  ATH_MSG_VERBOSE(" ***** in searchElementVector(atomicNumber) search success for atomic number "  << atomicNumber);
  return nullptr;
}

const GeoMaterial*  RDBMaterialManager::getMaterial(const std::string &name)
{
  [[maybe_unused]] static const bool specialMaterialsDone = [this]() {
    buildSpecialMaterials();
    return true;
  }();

  ATH_MSG_DEBUG(" ***** getMaterial( ): " << name);

  GeoMaterial* pmaterial = searchMaterialMap(name);
  if (pmaterial)
    return pmaterial;

  size_t pos = name.find("::");
  if(pos==std::string::npos) {
    ATH_MSG_ERROR("Wrong format for the material name " << name
		  << ". Must be detector::material");
    return nullptr;
  }

  const std::string detector = name.substr(0,pos);
  auto it = m_detData.find(detector);
  if(it==m_detData.end()) {
    ATH_MSG_ERROR("Wrong detector name " << detector
		  << " passed to getMaterial()");
    return nullptr;
  }

  IRDBRecordset_ptr tmp_materials = it->second.m_materials;
  IRDBRecordset_ptr tmp_matcomponents = it->second.m_matcomponents;
  const std::string& data_id = it->second.m_prim_key;

  auto it_material = std::find_if(tmp_materials->begin()
				  , tmp_materials->end()
				  , [&name, &detector](const IRDBRecord_ptr& rec) {
				    return name == detector+"::"+rec->getString("NAME");
				  });

  if(it_material==tmp_materials->end()) {
    ATH_MSG_VERBOSE(detector << " materials retrieved from the database don't include " << name);
    return nullptr;
  }

  const auto& rec = *it_material;
  std::string material_name = detector+"::"+rec->getString("NAME");
  long material_id = rec->getLong(data_id);
  double material_density = rec->getDouble("DENSITY");
  
  ATH_MSG_DEBUG(" ***** Material: name id density: "  << material_name <<" " << material_id <<" "<< material_density);
  
  pmaterial = new GeoMaterial( material_name,material_density * (GeoModelKernelUnits::gram / Gaudi::Units::cm3));

  bool firstComponent = true;
  bool hasSubMaterial = false;
  bool calculateFraction = false;
  double totalFraction = 0.;
  double component_fraction = 0.;
  std::string component_name{};

  std::vector <const GeoElement*> elementComponents;
  std::vector <double>        elementFractions;

  for(const auto& rec : *tmp_matcomponents) {
    if(rec->getLong("MATERIAL_ID") == material_id) {
      component_name = rec->getString("COMPNAME");
      component_fraction = rec->getDouble("FRACTION");

      if(firstComponent) {
	firstComponent = false;
	if(component_fraction>=1.) {
	  calculateFraction = true;
	}
      }

      if(component_name.find("::",0) == std::string::npos) {
	const GeoElement* p_com_element = getElement(component_name);

	if(calculateFraction) {
	  totalFraction += component_fraction*p_com_element->getA();
	  elementComponents.push_back(p_com_element);
	  elementFractions.push_back(component_fraction);
	}
	else {
	  pmaterial->add( p_com_element, component_fraction);
	}
      }
      else {
	hasSubMaterial = true;
	const GeoMaterial* p_com_material = getMaterial(component_name);
	pmaterial->add(p_com_material, component_fraction);
      }	// Element vs Sub-Material
    } // Deal with the component
  } // Loop over records

  if(calculateFraction && hasSubMaterial && elementComponents.size()>0)
    ATH_MSG_WARNING(material_name << " description should be changed. Please indicate the exact fraction for elements");

  if(calculateFraction && !elementComponents.empty()) {
    double inv_totalFraction = totalFraction == 0 ? 1 : 1. / totalFraction;
    for(unsigned i=0; i<elementComponents.size(); ++i) {
      pmaterial->add(elementComponents[i],elementFractions[i]*elementComponents[i]->getA() * inv_totalFraction);
    }
  }

  // Cache new material
  addMaterial(detector,pmaterial);
	
  return pmaterial;
}

const GeoElement *RDBMaterialManager::getElement(const std::string & name)
{
  ATH_MSG_VERBOSE(" ***** getElement(): " << name);

  GeoElement* pelement = searchElementVector(name);
  if(!pelement) {
    for(const auto& rec : *m_elements) {
      if(name == rec->getString("NAME")) {
	pelement = new GeoElement(rec->getString("NAME")
				  , rec->getString("SYMBOL")
				  , rec->getDouble("Z")
				  , rec->getDouble("A")*GeoModelKernelUnits::gram/Gaudi::Units::mole);
	
	// a table to keep the memory allocation, and easy for delete
	m_elementVector.push_back(pelement);
	break;
      }
    }
  }
  return pelement;
}

const GeoElement *RDBMaterialManager::getElement(unsigned int atomicNumber)
{
  ATH_MSG_VERBOSE(" ***** const getElement(atomicNumber) const : " << atomicNumber);

  GeoElement* pelement = searchElementVector(atomicNumber);
  if(!pelement) {
    for(const auto& rec : *m_elements) {
      if(atomicNumber == rec->getDouble("A")) {
	pelement = new GeoElement(rec->getString("NAME")
				  , rec->getString("SYMBOL")
				  , rec->getDouble("Z")
				  , rec->getDouble("A")*GeoModelKernelUnits::gram/Gaudi::Units::mole);
	
	// a table to keep the memory allocation, and easy for delete
	m_elementVector.push_back( pelement);
	break;
      }
    }
  }
  return pelement;
}

void RDBMaterialManager::addMaterial(const std::string & /*space*/, GeoMaterial *material)
{
  ATH_MSG_VERBOSE(" ***** RDBMaterialManager::addMaterial() ");
  const std::string& key = material->getName();
  const auto [it_material, success] = m_materialMap.emplace(key,material);
  if(success) {
    it_material->second->lock();
  }
  else {
    // Warn if we already have material with the same name defined
    ATH_MSG_WARNING(" Attempt to redefine material " << key
		    << "!. The existing instance is kept. Please choose another name for new material");
  }
}

StoredMaterialManager::MaterialMapIterator RDBMaterialManager::begin() const
{
  return m_materialMap.begin();
}

StoredMaterialManager::MaterialMapIterator RDBMaterialManager::end() const
{
  return m_materialMap.end();
}

size_t RDBMaterialManager::size()
{
  return m_materialMap.size();
}

std::ostream &  RDBMaterialManager::printAll(std::ostream & o) const 
{
  o << "============Material Manager Element List========================\n";
  for (GeoElement* elt : m_elementVector){
    o << elt->getSymbol() << '\t' << elt->getZ() <<  '\t' << elt->getA() * (Gaudi::Units::mole / GeoModelKernelUnits::gram) << '\t' << elt->getName() << "\n";
  }

  for (const auto& p : m_materialMap){
    o << "Material: " << p.first <<  " Density " << p.second->getDensity() * (Gaudi::Units::cm3 / GeoModelKernelUnits::gram)  << "\n";
    for (size_t i = 0; i< p.second->getNumElements();i++) {
      o <<" ***** ***** "<< int (p.second->getFraction(i)*100) << "% \t"  << p.second->getElement(i)->getName() << std::endl;
    }
  }
  	  	
  return o;
}

void RDBMaterialManager::buildSpecialMaterials()
{
  // Create special materials
  GeoElement* ethElement = new GeoElement("Ether","ET",500.0,0.0);
  m_elementVector.push_back(ethElement);  
  GeoMaterial* ether = new GeoMaterial("special::Ether",0.0);	
  ether->add(ethElement,1.);
  addMaterial("special",ether);
  // "Alternative" assembly material
  GeoMaterial* hu = new GeoMaterial("special::HyperUranium",0.0);	
  hu->add(ethElement,1.);
  addMaterial("special",hu);
}
