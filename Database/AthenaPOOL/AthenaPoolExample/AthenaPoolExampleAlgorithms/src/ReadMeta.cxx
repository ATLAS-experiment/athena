/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file ReadMeta.cxx
 *  @brief This file contains the implementation for the ReadMeta class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "ReadMeta.h"

// the user data-class definitions
#include "AthenaPoolExampleData/ExampleHitContainer.h"

#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/FileIncident.h"

#include "StoreGate/StoreGateSvc.h"

using namespace AthPoolEx;

//___________________________________________________________________________
ReadMeta::ReadMeta(const std::string& type, const std::string& name, const IInterface* parent) :
   base_class(type, name, parent),
   m_pMetaDataStore ("StoreGateSvc/MetaDataStore",      name),
   m_pInputStore    ("StoreGateSvc/InputMetaDataStore", name) {
}
//___________________________________________________________________________
StatusCode ReadMeta::initialize() {
   ATH_MSG_INFO("in initialize()");

   // locate the DetectorStore and initialize our local ptr
   ATH_CHECK( m_pMetaDataStore.retrieve() );
   ATH_CHECK( m_pInputStore.retrieve() );

   // Set to be listener for end of event
   ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", this->name());
   ATH_CHECK( incSvc.retrieve() );
   incSvc->addListener(this, "BeginInputFile", 60); // pri has to be < 100 to be after MetaDataSvc.
   incSvc->addListener(this, "EndInputFile", 50); // pri has to be > 10 to be before MetaDataSvc.
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
void ReadMeta::handle(const Incident& inc) {
   ATH_MSG_DEBUG("handle() " << inc.type());
   const FileIncident* fileInc  = dynamic_cast<const FileIncident*>(&inc);
   if (fileInc == nullptr) {
      ATH_MSG_ERROR(" Unable to get FileName from BeginInputFile/EndInputFile incident");
      return;
   }
   ATH_MSG_DEBUG("handle() " << inc.type() << " for " << fileInc->fileName());
}
//__________________________________________________________________________
StatusCode ReadMeta::beginInputFile(const SG::SourceID&)
{
   ATH_MSG_DEBUG("saw BeginInputFile incident.");
   if (m_pInputStore->contains<ExampleHitContainer>("PedestalWriteData")) {
      std::list<SG::ObjectWithVersion<ExampleHitContainer> > allVersions;
      ATH_CHECK( m_pInputStore->retrieveAllVersions(allVersions, "PedestalWriteData") );
      //const ExampleHitContainer* ep;
      ExampleHitContainer* ep_out = nullptr;
      for (SG::ObjectWithVersion<ExampleHitContainer>& obj : allVersions) {
         const ExampleHitContainer* ep = obj.dataObject.cptr();
         if (!m_pMetaDataStore->contains<ExampleHitContainer>("PedestalWriteData")) {
            auto ep_out_unique = std::make_unique<ExampleHitContainer>();
            const ExampleHit* entry = *ep->begin();
            auto entry_out = std::make_unique<ExampleHit>();
            entry_out->setX(entry->getX());
            entry_out->setY(entry->getY());
            entry_out->setZ(entry->getZ());
            entry_out->setDetector(entry->getDetector());
            ep_out_unique->push_back(std::move(entry_out));
            ep_out = ep_out_unique.get();
            ATH_CHECK( m_pMetaDataStore->record(std::move(ep_out_unique), "PedestalWriteData") );
         } else {
            ATH_CHECK( m_pMetaDataStore->retrieve(ep_out, "PedestalWriteData") );
            const ExampleHit* entry = *ep->begin();
            ExampleHit* entry_out = *ep_out->begin();
            int weight = entry->getDetector().size() - 2;
            int weight_out = entry_out->getDetector().size() - 2;
            entry_out->setX((entry->getX() * weight + entry_out->getX() * weight_out) / (weight + weight_out));
            entry_out->setY((entry->getY() * weight + entry_out->getY() * weight_out) / (weight + weight_out));
            entry_out->setZ((entry->getZ() * weight + entry_out->getZ() * weight_out) / (weight + weight_out));
            entry_out->setDetector(entry->getDetector().substr(0, entry->getDetector().size() - 1) + entry_out->getDetector().substr(1));
         }
      }
      if (ep_out != nullptr) {
         for (const ExampleHit* obj : *ep_out) {
            ATH_MSG_INFO("Pedestal x = " << obj->getX() << " y = " << obj->getY() << " z = " << obj->getZ() << " string = " << obj->getDetector());
         }
      }
   }

   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
