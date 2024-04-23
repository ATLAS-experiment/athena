/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetGeoModelUtils/GeoModelXmlTool.h"

#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelUtilities/DecodeVersionKey.h>
#include <GeoModelXml/Gmx2Geo.h>
#include <GeoModelXml/GmxInterface.h>
#include <GeoModelRead/ReadGeoModel.h>
#include <PathResolver/PathResolver.h>
#include <RDBAccessSvc/IRDBRecord.h>
#include <RDBAccessSvc/IRDBRecordset.h>

#include <fstream>
#include <utility>

GeoModelXmlTool::GeoModelXmlTool(const std::string &type,
                                 const std::string &name,
                                 const IInterface *parent)
 : GeoModelTool(type, name, parent)
{
}

StatusCode GeoModelXmlTool::createBaseTool()
{
  ATH_CHECK(m_geoDbTagSvc.retrieve());
  ATH_CHECK(m_rdbAccessSvc.retrieve());
  ATH_CHECK(m_sqliteReadSvc.retrieve());

  return StatusCode::SUCCESS;
}

const GeoVPhysVol* GeoModelXmlTool::createTopVolume(GeoPhysVol* world, GmxInterface& gmxInterface, const std::string& vNode, const std::string& tableName, const std::string& containingDetector, const std::string& envelopeName, const GeoModelIO::ReadGeoModel* sqlreader) const
{
  //If we have a valid sqlreader pointer, it means the volume should already exist,
  // so we don't need to create it  
  if(!sqlreader) createVolume(world, gmxInterface, vNode, tableName);
  else ATH_MSG_INFO("Using geometry from .sqlite file for"<<m_detectorName);

  unsigned int nChildren = world->getNChildVols();

  const GeoVPhysVol * envVol = nullptr;
  const GeoVPhysVol * topVol = nullptr;

  bool foundEnvelope = false;
  bool foundContainingDetector = false;
  // find the appropriate volume in the hierarchy, to allow it to be set as the topVolume in
  // our detectorManager
  std::string detectorName = m_detectorName; 
  //if a containingDetector is set, which the detector we are dealing with will sit inside, look for that first. 
  // Otherwise its just the name of the detector we are dealing with we look for
  if(containingDetector!="") detectorName = containingDetector;

  for (int iChild = nChildren - 1; iChild>=0; --iChild) {
    if (world->getNameOfChildVol(iChild) == detectorName) {
      // The * converts from a ConstPVLink to a reference to a GeoVPhysVol;
      // the & takes its address.
      envVol = &*world->getChildVol(iChild);
      foundContainingDetector = true;
      if(envelopeName=="") {topVol = envVol;break;}
      unsigned int nGrandchildren = envVol->getNChildVols();    
    for (int iGchild = nGrandchildren - 1; iGchild>=0; --iGchild) {
     if (envVol->getNameOfChildVol(iGchild) == envelopeName) {  
      topVol = &*(envVol->getChildVol(iGchild));
      foundEnvelope = true;
      break;
     }
    }
   }
  }
  if(containingDetector!="" && !foundContainingDetector) ATH_MSG_ERROR("Couldn't find the containing detector "<<containingDetector<<" in the world hierarchy!");
  else if(envelopeName!="" && !foundEnvelope) ATH_MSG_ERROR("Couldn't find the envelope volume "<<envelopeName<<" in the world hierarchy!");
  return topVol;
}

bool GeoModelXmlTool::isAvailable(const std::string& vNode, const std::string& tableName) const
{
  if (m_gmxFilename.empty()) {
    DecodeVersionKey versionKey(&*m_geoDbTagSvc, vNode);
    const std::string& versionTag  = versionKey.tag();
    const std::string& versionNode = versionKey.node();
    const std::string version = m_rdbAccessSvc->getChildTag(tableName, versionTag, versionNode);
    if (version.empty()) {
      return false;
    }
    ATH_MSG_INFO("Using " << version << " from " << versionNode << " tag " << versionTag);
  }

  return true;
}

std::string GeoModelXmlTool::getBlob(const std::string& vNode, const std::string& tableName) const
{
  DecodeVersionKey versionKey(&*m_geoDbTagSvc, vNode);
  const IRDBRecordset_ptr recordSet = m_rdbAccessSvc->getRecordsetPtr(tableName, versionKey.tag(), versionKey.node());
  if (!recordSet || recordSet->size() == 0) {
    ATH_MSG_FATAL("Unable to obtain " << vNode << " recordSet");
    throw std::runtime_error("Unable to obtain recordSet");
  }
  const IRDBRecord *record = (*recordSet)[0];
  std::string clobString = record->getString("XMLCLOB");
  return clobString;
}

void GeoModelXmlTool::createVolume(GeoPhysVol* world, GmxInterface& gmxInterface, const std::string& vNode, const std::string& tableName) const {
  int flags{};
  std::string gmxInput;

  if (m_gmxFilename.empty()) {
    ATH_MSG_INFO("Getting " << m_detectorName.value() << " GeoModelXml description from the geometry database");
    flags = 0x1; // Lowest bit ==> string; next bit implies gzip'd but we decided not to gzip
    // how to propagate these to here best...?
    gmxInput = getBlob(vNode,tableName);
    if (gmxInput.empty()) { // Invalid blob?
      std::string errMessage("GeoModelXmlTool::createTopVolume: Empty response received from the database.");
      throw std::runtime_error(errMessage);
    }
  } else {
    flags = 0;
    gmxInput = PathResolver::find_file(m_gmxFilename, "DATAPATH");
    ATH_MSG_INFO("Getting " << m_detectorName.value() << " GeoModelXml description from file "<<gmxInput);
    if (gmxInput.empty()) { // File not found
      std::string errMessage("GeoModelXmlTool::createTopVolume: Unable to find file " + m_gmxFilename +
                             " with PathResolver; check filename and DATAPATH environment variable");
      throw std::runtime_error(errMessage);
    }
  }

  // Use the DTD from GeoModel
  if (m_gmxFilename.empty()) {
    std::string replacementName = "GeoModelXml/";
    //now, work out the specific dtd version in the input .gmx
    std::string startdelim = "SYSTEM \"";
    std::string enddelim = "\" [";
    unsigned startpos = gmxInput.find(startdelim) + startdelim.length();
    unsigned endpos = gmxInput.find(enddelim);
    std::string searchName = gmxInput.substr(startpos,(endpos - startpos));
    if(searchName=="geomodel.dtd") replacementName+="geomodel_v0.dtd"; //used in xml for initial geometry tags - special case
    else replacementName+=searchName;
    ATH_MSG_DEBUG("Searching for "<<searchName<<" and replacing it with "<<replacementName);
    size_t chars = searchName.length();
    size_t index = gmxInput.find(searchName);
    if(m_dtdName!="")  replacementName=m_dtdName; //allow overriding of dtd version
    if (index != std::string::npos) {
        std::string dtdFile = PathResolver::find_file(replacementName, "DATAPATH");
        ATH_MSG_DEBUG("dtdFile = " << dtdFile);  
        gmxInput.replace(index,chars, dtdFile);
    } else {  
      throw std::runtime_error("GeoModelXmlTool::createTopVolume: Did not find valid .dtd in the gmx input string.");
    }
  }

  // optionally dump to local file for examination
  if (m_clobOutputFileName != "") {
    std::ofstream out(m_clobOutputFileName);
    if (m_gmxFilename.empty()) {
      out << gmxInput;
    } else {
      std::ifstream in(gmxInput);
      out << in.rdbuf();
    }
    out.close();
  }
  gmxInterface.enableLogVolDeDuplication(m_deduplicateLogVol);
  gmxInterface.enablePhysVolDeDuplication(m_deduplicatePhysVol);
  gmxInterface.enableShapeDeDuplication(m_deduplicateShape);
  gmxInterface.enableTransformDeDuplication(m_deduplicateTransf);

  Gmx2Geo gmx2Geo(gmxInput, world, gmxInterface, flags);  
}

GeoModelIO::ReadGeoModel* GeoModelXmlTool::getSqliteReader() const{
  return m_geoDbTagSvc->getSqliteReader();
}
