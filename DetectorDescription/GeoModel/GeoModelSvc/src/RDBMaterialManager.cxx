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


//---------------------------Help find elements in the list-----------------------//
class NameEquals {                                                                //
public:                                                                           //
  NameEquals(const std::string & name):m_name(name){}                              //
  bool operator() (const GeoElement *e) const {return m_name==e->getName();}       //
private:                                                                          //
  std::string m_name;                                                              //
};                                                                                //
//--------------------------------------------------------------------------------//

//---------------------------Help find elements in the list-----------------------//
class NumberEquals {                                                              //
public:                                                                           //
  NumberEquals(unsigned int number):m_number(number){}                             //
  bool operator() (const GeoElement *e) const {return m_number==e->getZ();}        //
private:                                                                          //
  unsigned int m_number;                                                           //
};                                                                                //
//--------------------------------------------------------------------------------//

int CheckElement(std::string &name)
{
  if(name.find("::",0) == std::string::npos) {
    return 1;
  }
  else {
    return 0;	
  }
}

int printElement ( GeoElement* &p_element)
{
  std::string name = p_element->getName();
  std::string symbol = p_element->getSymbol();
  double a = p_element->getA();
  double z = p_element->getZ();
	
  std::cout << " ***** CheckElement(): Print the Element:  " << name << "\n"; 
  std::cout << " ***** The Element: name,		symbol, 	A, 	Z \n" ; 
  std::cout << " *****             "<<name <<"		"<<symbol <<"		"<< a * (Gaudi::Units::mole / GeoModelKernelUnits::gram) <<"	"<< z <<"	"  << std::endl;
	
  return 1;
}

int printElement ( const GeoElement* &p_element)
{
  std::string name = p_element->getName();
  std::string symbol = p_element->getSymbol();
  double a = p_element->getA();
  double z = p_element->getZ();
	
  std::cout << " ***** PrintElement(): Print the Element:  " << name << "\n"; 
  std::cout << " ***** The Element: name,		symbol, 	A, 	Z \n"; 
  std::cout << " *****             "<<name <<"		"<<symbol <<"		"<< a * (Gaudi::Units::mole / GeoModelKernelUnits::gram) <<"	"<< z <<"	"  << std::endl;
	
  return 1;
}

int printMaterial ( GeoMaterial* &p_material)
{
  std::string name = p_material->getName();
  double density = p_material->getDensity() * (Gaudi::Units::cm3 / GeoModelKernelUnits::gram);

  std::cout << " ***** PrintMaterial(): Print the Material:  " << name << "\n"; 
  std::cout << " ***** The Material: name,	density	\n" ; 
  std::cout << " *****              "<< name <<"		"<<density <<"		" << std::endl; 	
	
  return 1;
}

int printFullMaterial ( GeoMaterial* &p_material)
{
  std::string name = p_material->getName();
  double density = p_material->getDensity() * (Gaudi::Units::cm3 / GeoModelKernelUnits::gram);
	
  std::cout << " ***** PrintFullMaterial(): Print the Material:  " << name << "\n"; 
  std::cout << " ***** The Material: name, 	density\n" ; 
  std::cout << " *****              "<< name <<" 	 "<<density <<"  " << std::endl; 
	
  p_material->lock();
  int element_number = p_material->getNumElements();	
		
 			
  if ( element_number  == 0){
    std::cout << " ***** No Elements now in this printMaterial( ) " << std::endl;	
    return 1;
  }
  else {
    element_number = p_material->getNumElements();	
	
    for(int i =0; i< element_number;i ++)
      {
	const GeoElement* tmp_element = p_material->getElement(i);
	double element_fraction = p_material->getFraction(i);
		
	std::cout<<" ***** ***** Number:  " << i << " Fraction:  " << element_fraction<< std::endl;
	printElement( tmp_element); 
      }	
  }
  return 1;
}
	
	

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

  // --- Standard materials, elements
  DecodeVersionKey keyAtlas(iGeoModel, "ATLAS");
  m_elements = iAccessSvc->getRecordsetPtr("Elements",keyAtlas.tag(),keyAtlas.node());
  if(defaulted(m_elements)) {
    ATH_MSG_WARNING("Getting Elements with default tag");
    m_elements = iAccessSvc->getRecordsetPtr("Elements","Materials-00","Materials");
  }
  m_stdmatcomponents = iAccessSvc->getRecordsetPtr("StdMatComponents",keyAtlas.tag(),keyAtlas.node());
  if(defaulted(m_stdmatcomponents))	{
    ATH_MSG_WARNING("Getting StdMatComponents with default tag");
    m_stdmatcomponents = iAccessSvc->getRecordsetPtr("StdMatComponents","Materials-00","Materials");
  }
  m_stdmaterials = iAccessSvc->getRecordsetPtr("StdMaterials",keyAtlas.tag(),keyAtlas.node());
  if(defaulted(m_stdmaterials)) {
    ATH_MSG_WARNING("Getting StdMaterials with default tag");
    m_stdmaterials = iAccessSvc->getRecordsetPtr("StdMaterials","Materials-00","Materials");
  }
  
  // --- Pixel materials
  DecodeVersionKey keyPixel(iGeoModel, "Pixel");
  m_pixmatcomponents = iAccessSvc->getRecordsetPtr("PixMatComponents",keyPixel.tag(),keyPixel.node());
  if(defaulted(m_pixmatcomponents)) {
    ATH_MSG_WARNING("Getting PixMatComponents with default tag");
    m_pixmatcomponents = iAccessSvc->getRecordsetPtr("PixMatComponents","PixMatComponents-00");
  }
  m_pixmaterials = iAccessSvc->getRecordsetPtr("PixMaterials",keyPixel.tag(),keyPixel.node());
  if(defaulted(m_pixmaterials)) {
    ATH_MSG_WARNING("Getting PixMaterials with default tag");
    m_pixmaterials = iAccessSvc->getRecordsetPtr("PixMaterials","PixMaterials-00");
  }
  
  // --- Pixel materials for TB
  //for test beam materials we just issue debug level messages. Perhaps this load can be fully omitted?
  m_pixtbmatcomponents = iAccessSvc->getRecordsetPtr("PixelTBMatComponents",keyPixel.tag(),keyPixel.node());
  if(defaulted( m_pixtbmatcomponents)) {
    ATH_MSG_DEBUG("Getting PixTBMatComponents with default tag" );
    m_pixtbmatcomponents = iAccessSvc->getRecordsetPtr("PixMatComponents","PixMatComponents-00");
  }
  m_pixtbmaterials = iAccessSvc->getRecordsetPtr("PixelTBMaterials",keyPixel.tag(),keyPixel.node());
  if(defaulted(m_pixtbmaterials)) {
    ATH_MSG_DEBUG("Getting PixTBMaterials with default tag");
    m_pixtbmaterials = iAccessSvc->getRecordsetPtr("PixMaterials","PixMaterials-00");
  }
  
  // --- SCT materials
  DecodeVersionKey keySCT(iGeoModel, "SCT");
  m_sctmatcomponents = iAccessSvc->getRecordsetPtr("SCTMatComponents",keySCT.tag(),keySCT.node());
  if(defaulted(m_sctmatcomponents))	{
    ATH_MSG_WARNING("Getting SCTMatComponents with default tag");
    m_sctmatcomponents = iAccessSvc->getRecordsetPtr("SCTMatComponents","SCTMatComponents-00");
  }
  
  m_sctmaterials = iAccessSvc->getRecordsetPtr("SCTMaterials",keySCT.tag(),keySCT.node());
  if(defaulted(m_sctmaterials)) {
    ATH_MSG_WARNING("Getting SCTMaterials with default tag");
    m_sctmaterials = iAccessSvc->getRecordsetPtr("SCTMaterials","SCTMaterials-00");
  }
  
  // --- TRT materials
  DecodeVersionKey keyTRT(iGeoModel, "TRT");
  m_trtmatcomponents = iAccessSvc->getRecordsetPtr("TrtMatComponents",keyTRT.tag(),keyTRT.node());
  if(defaulted(m_trtmatcomponents))	{
    ATH_MSG_WARNING("Getting TrtMatComponents with default tag");
    m_trtmatcomponents = iAccessSvc->getRecordsetPtr("TrtMatComponents","TrtMatComponents-00");
  }
  m_trtmaterials = iAccessSvc->getRecordsetPtr("TrtMaterials",keyTRT.tag(),keyTRT.node());
  if(defaulted(m_trtmaterials)) {
    ATH_MSG_WARNING("Getting TrtMaterials with default tag");
    m_trtmaterials = iAccessSvc->getRecordsetPtr("TrtMaterials","TrtMaterials-00");
  }
  
  // --- InDet common materials
  DecodeVersionKey keyInDet(iGeoModel, "InnerDetector");
  m_indetmatcomponents = iAccessSvc->getRecordsetPtr("InDetMatComponents",keyInDet.tag(),keyInDet.node());
  if(defaulted(m_indetmatcomponents)) {
    ATH_MSG_DEBUG("Getting InDetMatComponents with default tag");
    m_indetmatcomponents = iAccessSvc->getRecordsetPtr("InDetMatComponents","InDetMatComponents-00");
  }
  
  m_indetmaterials = iAccessSvc->getRecordsetPtr("InDetMaterials",keyInDet.tag(),keyInDet.node());
  if(defaulted(m_indetmaterials)) {
    ATH_MSG_DEBUG("Getting InDetMaterials with default tag");
    m_indetmaterials = iAccessSvc->getRecordsetPtr("InDetMaterials","InDetMaterials-00");
  }
  
  // --- LAr materials
  DecodeVersionKey keyLAr(iGeoModel, "LAr");    
  m_larmatcomponents = iAccessSvc->getRecordsetPtr("LArMatComponents",keyLAr.tag(),keyLAr.node());
  if(defaulted(m_larmatcomponents)) {
    ATH_MSG_WARNING("Getting LArMatComponents with default tag");
    m_larmatcomponents = iAccessSvc->getRecordsetPtr("LArMatComponents","LArMatComponents-00");
  }
  m_larmaterials = iAccessSvc->getRecordsetPtr("LArMaterials",keyLAr.tag(),keyLAr.node());
  if(defaulted(m_larmaterials)) {
    ATH_MSG_WARNING("Getting LArMaterials with default tag");
    m_larmaterials = iAccessSvc->getRecordsetPtr("LArMaterials","LArMaterials-00");
  }
  
  // --- Tile materials
  DecodeVersionKey keyTile(iGeoModel, "TileCal");    
  m_tilematcomponents = iAccessSvc->getRecordsetPtr("TileMatComponents",keyTile.tag(),keyTile.node());
  if (defaulted(m_tilematcomponents)) {
    ATH_MSG_WARNING("Getting TileMatComponents with default tag" );
    m_tilematcomponents = iAccessSvc->getRecordsetPtr("TileMatComponents","TileMatComponents-00");
  }
  m_tilematerials = iAccessSvc->getRecordsetPtr("TileMaterials",keyTile.tag(),keyTile.node());
  if(defaulted(m_tilematerials)) {
    ATH_MSG_WARNING("Getting TileMaterials with default tag");
    m_tilematerials = iAccessSvc->getRecordsetPtr("TileMaterials","TileMaterials-00");
  }
  
  // --- Muon
  DecodeVersionKey keyMuon(iGeoModel, "MuonSpectrometer");
  m_muomatcomponents = iAccessSvc->getRecordsetPtr("MUOMatComponents",keyMuon.tag(),keyMuon.node());
  if(defaulted(m_muomatcomponents))	{
    ATH_MSG_WARNING("Getting MUOMatComponents with default tag");
    m_muomatcomponents = iAccessSvc->getRecordsetPtr("MUOMatComponents","MUOMatComponents-00");
  }
  m_muomaterials = iAccessSvc->getRecordsetPtr("MUOMaterials",keyMuon.tag(),keyMuon.node());
  if(defaulted(m_muomaterials)) {
    ATH_MSG_WARNING("Getting MUOMaterials with default tag" );
    m_muomaterials = iAccessSvc->getRecordsetPtr("MUOMaterials","MUOMaterials-00");  
  }
  m_shieldmatcomponents = iAccessSvc->getRecordsetPtr("ShieldMatComponents",keyMuon.tag(),keyMuon.node());
  if(defaulted(m_shieldmatcomponents)) {
    ATH_MSG_WARNING("Getting ShieldMatComponents with default tag");
    m_shieldmatcomponents = iAccessSvc->getRecordsetPtr("ShieldMatComponents","ShieldMatComponents-00");
  }
  m_shieldmaterials = iAccessSvc->getRecordsetPtr("ShieldMaterials",keyMuon.tag(),keyMuon.node());
  if(defaulted(m_shieldmaterials)) {
    ATH_MSG_WARNING("Getting ShieldMaterials with default tag");
    m_shieldmaterials = iAccessSvc->getRecordsetPtr("ShieldMaterials","ShieldMaterials-00");
  }
  m_toromatcomponents = iAccessSvc->getRecordsetPtr("ToroMatComponents",keyMuon.tag(),keyMuon.node());
  if(defaulted(m_toromatcomponents)) {
    ATH_MSG_WARNING("Getting ToroMatComponents with default tag");
    m_toromatcomponents =	iAccessSvc->getRecordsetPtr("ToroMatComponents","ToroMatComponents-00");
  }
  m_toromaterials = iAccessSvc->getRecordsetPtr("ToroMaterials",keyMuon.tag(),keyMuon.node());
  if(defaulted(m_toromaterials)) {
    ATH_MSG_WARNING("Getting ToroMaterials with default tag");
    m_toromaterials = iAccessSvc->getRecordsetPtr("ToroMaterials","ToroMaterials-00");
  }
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
  NameEquals matchByName(name);
  GeoEleVec::const_iterator e=std::find_if(m_elementVector.begin(), m_elementVector.end(),std::move(matchByName));

  if (e!=m_elementVector.end()) {
    ATH_MSG_VERBOSE(" ***** in searchElementVector() search success for "  << name);
    return *e;
  }

  ATH_MSG_VERBOSE(" ***** in searchElementVector() search failed for "  << name);
  return nullptr;
}


GeoElement *RDBMaterialManager::searchElementVector(const unsigned int atomicNumber) const
{ 
  NumberEquals matchByNumber(atomicNumber);
  GeoEleVec::const_iterator e=std::find_if(m_elementVector.begin(), m_elementVector.end(), matchByNumber);
  	
  if (e!=m_elementVector.end()) {
    ATH_MSG_VERBOSE(" ***** in searchElementVector(atomicNumber) search success for atomic number "  << atomicNumber);
    return *e;
  }
 
  ATH_MSG_VERBOSE(" ***** in searchElementVector(atomicNumber) search success for atomic number "  << atomicNumber);
  return nullptr;
}

const GeoMaterial*  RDBMaterialManager:: getMaterial(const std::string &name) {
  unsigned int  ind{0}, com_ind{0};
  std::string material_name;
  std::string tmp_name;
  long 	    material_id{0};
  double    material_density{0.};
  std::string component_name{};
  double      component_fraction{0.};
  int 	      component_id{0};

  std::string detector;
  std::string tmp_det;
  std::string data_id;
	
  std::string matcomponents_table;

  [[maybe_unused]] static const bool specialMaterialsDone = [this]() {
    buildSpecialMaterials();
    return true;
  }();

  GeoMaterial* pmaterial;

  const GeoElement*  p_com_element;

  IRDBRecordset_ptr tmp_materials;
  IRDBRecordset_ptr tmp_matcomponents;

  ATH_MSG_DEBUG(" ***** getMaterial( ): "  << name);

  pmaterial = nullptr;
  pmaterial = searchMaterialMap( name);
  if (pmaterial!= nullptr) 
      return pmaterial;

  if(name.starts_with("std"))
    {
      detector = "std";
      tmp_materials = m_stdmaterials;
      tmp_matcomponents = m_stdmatcomponents;
      data_id = "STDMATERIALS_DATA_ID";
    }
  else if(name.starts_with("trt"))
    {
      detector = "trt";
      tmp_materials = m_trtmaterials;
      tmp_matcomponents = m_trtmatcomponents;
      data_id = "TRTMATERIALS_DATA_ID";
    }
  else if(name.starts_with("LAr"))
    {
      detector = "LAr";
      tmp_materials = m_larmaterials;
      tmp_matcomponents = m_larmatcomponents;
      data_id = "LARMATERIALS_DATA_ID";
    }
  else if(name.starts_with("muo"))
    {
      detector = "muo";
      tmp_materials = m_muomaterials;
      tmp_matcomponents = m_muomatcomponents;
      data_id = "MUOMATERIALS_DATA_ID";
    }
  else if(name.starts_with("pixtb"))
    {
      detector = "pixtb";
      tmp_materials = m_pixtbmaterials;
      tmp_matcomponents = m_pixtbmatcomponents;
      data_id = "PIXELTBMATERIALS_DATA_ID";
    }
  else if(name.starts_with("pix"))
    {
      detector = "pix";
      tmp_materials = m_pixmaterials;
      tmp_matcomponents = m_pixmatcomponents;
      data_id = "PIXMATERIALS_DATA_ID";
    }
  else if(name.starts_with("sct"))
    {
      detector = "sct";
      tmp_materials = m_sctmaterials;
      tmp_matcomponents = m_sctmatcomponents;
      data_id = "SCTMATERIALS_DATA_ID";
    }
  else if(name.starts_with("indet"))
    {
      detector = "indet";
      tmp_materials = m_indetmaterials;
      tmp_matcomponents = m_indetmatcomponents;
      data_id = "INDETMATERIALS_DATA_ID";
    }
  else if(name.starts_with("shield"))
    {
      detector = "shield";
      tmp_materials = m_shieldmaterials;
      tmp_matcomponents = m_shieldmatcomponents;
      data_id = "SHIELDMATERIALS_DATA_ID";
    }
  else if(name.starts_with("tile"))
    {
      detector = "tile";
      tmp_materials = m_tilematerials;
      tmp_matcomponents = m_tilematcomponents;
      data_id = "TILEMATERIALS_DATA_ID";
    }
  else if(name.starts_with("toro"))
    {
      detector = "toro";
      tmp_materials = m_toromaterials;
      tmp_matcomponents = m_toromatcomponents;
      data_id = "TOROMATERIALS_DATA_ID";
    }
  else {return 0;}

  for( ind = 0; ind < tmp_materials->size(); ind++)
    {
      const IRDBRecord* rec = (*tmp_materials)[ind];
      tmp_name = detector+"::"+rec->getString("NAME");

      if( name == tmp_name){
	material_name  =detector+"::"+rec->getString("NAME");
	material_id = rec->getLong(data_id);
	material_density = rec->getDouble("DENSITY");

	ATH_MSG_DEBUG(" ***** Material: name id density: "  << material_name <<" " << material_id <<" "<< material_density);
	break;
      }
    }
		
  if (ind == tmp_materials->size()) 
      return nullptr;

  pmaterial = new GeoMaterial( material_name,material_density * (GeoModelKernelUnits::gram / Gaudi::Units::cm3));

  bool firstComponent = true;
  bool hasSubMaterial = false;
  bool calculateFraction = false;
  double totalFraction = 0.;
  std::vector <const GeoElement*> elementComponents;
  std::vector <double>        elementFractions;

  for(  com_ind = 0; com_ind <tmp_matcomponents->size(); com_ind++)
    {
      const IRDBRecord* com_rec = (*tmp_matcomponents)[com_ind];
		
      component_id = com_rec->getLong("MATERIAL_ID");
      if( component_id == material_id)
	{
	  component_name = com_rec->getString("COMPNAME");
	  component_fraction = com_rec->getDouble("FRACTION");
			
	  if(firstComponent)
	  {
	    firstComponent = false;
	    if(component_fraction>=1.)
	      calculateFraction = true;
	  }

	  if( CheckElement( component_name) == 1)
	    {
	      p_com_element = getElement(component_name);

	      if(calculateFraction)
	      {
		totalFraction += component_fraction*p_com_element->getA();
		elementComponents.push_back(p_com_element);
		elementFractions.push_back(component_fraction);
	      }
	      else
		pmaterial->add( p_com_element, component_fraction);
										
	    }
	  else{
	    hasSubMaterial = true;
	    const GeoMaterial* p_com_material = getMaterial(component_name);
	    pmaterial->add(p_com_material, component_fraction);
			
	  }		
	}
    }    

  if(calculateFraction && hasSubMaterial && elementComponents.size()>0)
    ATH_MSG_WARNING(material_name << " description should be changed. Please indicate the exact fraction for elements");

  if(calculateFraction && !elementComponents.empty()) {
    double inv_totalFraction = totalFraction == 0 ? 1 : 1. / totalFraction;
    for(unsigned i=0; i<elementComponents.size(); i++)
      pmaterial->add(elementComponents[i],elementFractions[i]*elementComponents[i]->getA() * inv_totalFraction);
  }

  // a table to keep the memory allocation, and easy for delete
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
