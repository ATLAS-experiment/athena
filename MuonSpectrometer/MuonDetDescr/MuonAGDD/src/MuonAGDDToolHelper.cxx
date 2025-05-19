/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonAGDDToolHelper.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AGDDControl/AGDDController.h"
#include "AGDDModel/AGDDParameterStore.h"
#include "AGDDControl/AGDD2GeoModelBuilder.h"
#include "AGDDControl/IAGDD2GeoSvc.h"
#include "AGDDKernel/AliasStore.h"
#include "AGDDKernel/AGDDDetector.h"
#include "AGDDKernel/AGDDDetectorStore.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "StoreGate/DataHandle.h"
#include "StoreGate/StoreGateSvc.h"

#include "MuonAGDDBase/micromegasHandler.h"
#include "MuonAGDDBase/mm_TechHandler.h"
#include "MuonAGDDBase/sTGCHandler.h"
#include "MuonAGDDBase/sTGC_readoutHandler.h"
#include "MuonAGDDBase/sTGC_TechHandler.h"
#include "MuonAGDDBase/mmSpacerHandler.h"
#include "MuonAGDDBase/mmSpacer_TechHandler.h"
#include "MuonAGDDBase/mm_readoutHandler.h"

#include "MuonDetDescrUtils/BuildNSWReadoutGeometry.h"

#include <fstream>

using namespace MuonGM;

MuonAGDDToolHelper::MuonAGDDToolHelper()
{
  SmartIF<IGeoModelSvc> geomodel{Gaudi::svcLocator()->service("GeoModelSvc")};
  if(!geomodel.isValid()) throw std::runtime_error("MuonAGDDToolHelper failed to access GeoModelSvc");
  SmartIF<IRDBAccessSvc> rdbaccess{Gaudi::svcLocator()->service("RDBAccessSvc")};
  if(!rdbaccess.isValid()) throw std::runtime_error("MuonAGDDToolHelper failed to access RDBAccessSvc");

  m_geoModelSvc = geomodel.get();
  m_rdbAccessSvc = rdbaccess.get();
}

std::vector<std::string> MuonAGDDToolHelper::ReadAGDDFlags()
{
   std::vector<std::string> structuresFromFlags;
   std::string agdd2geoVersion = m_rdbAccessSvc->getChildTag("AGDD2GeoSwitches",m_geoModelSvc->muonVersion(),"MuonSpectrometer");

   if(!agdd2geoVersion.empty())
   {
     std::string TheKEYNAME;
     int TheKEYVALUE;
     IRDBRecordset_ptr pIRDBRecordset = m_rdbAccessSvc->getRecordsetPtr("AGDD2GeoSwitches",m_geoModelSvc->muonVersion(),"MuonSpectrometer");
     for(unsigned int i=0; i<pIRDBRecordset->size(); i++)
     {
       const IRDBRecord* record = (*pIRDBRecordset)[i];
       TheKEYNAME = record->getString("KEYNAME");
       TheKEYVALUE = record->getInt("KEYVALUE");
       if ( TheKEYVALUE == 1 )
       {
         structuresFromFlags.push_back(TheKEYNAME);
       }
     }
   }
   else
   {
      std::cout<<"MuonAGDDToolHelper\tagdd2geoVersion is empty " <<std::endl;
   }
   return structuresFromFlags;
}


std::string MuonAGDDToolHelper::GetAGDD(const bool dumpIt, const std::string& tableName, const std::string& outFileName)
{
   std::string AtlasVersion = m_geoModelSvc->atlasVersion();
   std::string MuonVersion  = m_geoModelSvc->muonVersionOverride();

   std::string detectorKey  = MuonVersion.empty() ? AtlasVersion : MuonVersion;
   std::string detectorNode = MuonVersion.empty() ? "ATLAS" : "MuonSpectrometer";
   if ( MuonVersion == "CUSTOM"){
     detectorKey  = AtlasVersion ;
     detectorNode = "ATLAS"  ;
   }

   IRDBRecordset_ptr recordsetAGDD = m_rdbAccessSvc->getRecordsetPtr(tableName,detectorKey,detectorNode);
   if(!recordsetAGDD) return "";

   const IRDBRecord *recordAGDD =  (*recordsetAGDD)[0];
   if(!recordAGDD) return "";
   std::string AgddString = recordAGDD->getString("DATA");

   size_t pos=AgddString.find("AGDD.dtd");
   if (pos!=std::string::npos) AgddString.replace(pos-21,32,"-- Reference to AGDD.dtd automatically removed -->");
   std::ofstream  GeneratedFile;
   if (dumpIt)
   {
     std::ofstream GeneratedFile;
     GeneratedFile.open(outFileName);
     GeneratedFile<<AgddString;
     GeneratedFile.close();
   }
   return AgddString;
}

bool MuonAGDDToolHelper::BuildMScomponents()
{
  SmartIF<StoreGateSvc> pDetStore{Gaudi::svcLocator()->service("DetectorStore")};
  if (!pDetStore.isValid()) return false;
  MuonGM::MuonDetectorManager* muonMgr=nullptr;
  if (pDetStore->retrieve(muonMgr).isFailure()) return false;
  return BuildNSWReadoutGeometry::BuildReadoutGeometry(muonMgr);
}

void MuonAGDDToolHelper::SetNSWComponents()
{
  SmartIF<IAGDDtoGeoSvc> agddsvc{Gaudi::svcLocator()->service(m_svcName)};
  if(!agddsvc.isValid()) {
    throw std::runtime_error("MuonAGDDToolHelper::SetNSWComponents() - Could not retrieve "
                             + m_svcName + " from ServiceLocator");
  }

  IAGDDtoGeoSvc::LockedController c = agddsvc->getController();

  agddsvc->addHandler(new micromegasHandler("micromegas", *c));
  agddsvc->addHandler(new mm_TechHandler("mm_Tech", *c));
  agddsvc->addHandler(new sTGCHandler("sTGC", *c));
  agddsvc->addHandler(new sTGC_readoutHandler("sTGC_readout", *c));
  agddsvc->addHandler(new sTGC_TechHandler("sTGC_Tech", *c));
  agddsvc->addHandler(new mmSpacerHandler("mmSpacer", *c));
  agddsvc->addHandler(new mmSpacer_TechHandler("mmSpacer_Tech", *c));
  agddsvc->addHandler(new mm_readoutHandler("mm_readout", *c));
}

void MuonAGDDToolHelper::setAGDDtoGeoSvcName(const std::string& name) {
  m_svcName = name;
}
