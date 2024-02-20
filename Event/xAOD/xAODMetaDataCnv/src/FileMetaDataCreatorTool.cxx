/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "FileMetaDataCreatorTool.h"

// System include(s):
#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <utility>

// Athena metadata EDM:
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODMetaData/FileMetaData.h"
#include "xAODMetaData/FileMetaDataAuxInfo.h"


namespace xAODMaker {

StatusCode
    FileMetaDataCreatorTool::initialize() {
      ATH_CHECK(m_eventStore.retrieve());
      ATH_CHECK(m_metaDataSvc.retrieve());
      ATH_CHECK(m_inputMetaDataStore.retrieve());
      ATH_CHECK(m_tagInfoMgr.retrieve());

      // If DataHeader key not specified, try determining it
      if (m_dataHeaderKey.empty()) {
        const auto *parentAlg = dynamic_cast< const INamedInterface* >(parent());
        if (parentAlg)
          m_dataHeaderKey = parentAlg->name();
      }

      // Listen for the begin of an input file. Act after MetaDataSvc (prio 80) and
      // TagInfoMgr (prio 50). That means the FileMetaDataTool be called first
      ServiceHandle< IIncidentSvc > incidentSvc("IncidentSvc", name());
      ATH_CHECK(incidentSvc.retrieve());
      incidentSvc->addListener(this, "EndInputFile", 40);

      // Create a fresh object to fill
      ATH_MSG_DEBUG("Creating new xAOD::FileMetaData object to fill");
      m_info = std::make_unique< xAOD::FileMetaData >();
      m_aux  = std::make_unique< xAOD::FileMetaDataAuxInfo >();
      m_info->setStore(m_aux.get());

      // FileMetaData has no content
      m_filledNonEvent = false;
      m_filledEvent = false;

      // Return gracefully:
      return StatusCode::SUCCESS;
    }

void
    FileMetaDataCreatorTool::handle(const Incident& inc) {
      // gracefully ignore unexpected incident types
      if (inc.type() == "EndInputFile") {
        // Lock the tool while we work on the FileMetaData
        std::lock_guard lock(m_toolMutex);
        if (!updateFromNonEvent().isSuccess())
          ATH_MSG_DEBUG("Failed to fill FileMetaData with non-event info");
      }
    }

StatusCode
    FileMetaDataCreatorTool::postInitialize() {
      return StatusCode::SUCCESS;
    }

StatusCode
    FileMetaDataCreatorTool::preExecute() {
      return StatusCode::SUCCESS;
    }

StatusCode
    FileMetaDataCreatorTool::preStream() {
      return StatusCode::SUCCESS;
    }

StatusCode
    FileMetaDataCreatorTool::preFinalize() {
      std::lock_guard lock(m_toolMutex);

      if (!m_filledNonEvent) {
        ATH_MSG_DEBUG("Not writing empty or incomplete FileMetaData object");
        return StatusCode::SUCCESS;
      }

      // Set metadata with content created for given stream
      for (const std::string& key : m_metaDataSvc->getPerStreamKeysFor(m_key)) {
        // Remove any existing objects with this key
        if (!m_metaDataSvc->contains<xAOD::FileMetaData>(key)) {
          auto info = std::make_unique<xAOD::FileMetaData>();
          auto aux = std::make_unique<xAOD::FileMetaDataAuxInfo>();
          info->setStore(aux.get());
          ATH_CHECK(m_metaDataSvc->record(std::move(info), key));
          ATH_CHECK(m_metaDataSvc->record(std::move(aux), key + "Aux."));
        }

        auto* output = m_metaDataSvc->tryRetrieve<xAOD::FileMetaData>(key);
        if (output) {
          // save event info that we've already had
          float orig_mcProcID = -1;
          std::vector<uint32_t> orig_runNumbers, orig_lumiBlocks;
          if (!output->value(xAOD::FileMetaData::mcProcID, orig_mcProcID))
            ATH_MSG_DEBUG("Could not get mcProcID");
          if (!output->value("runNumbers", orig_runNumbers))
            ATH_MSG_DEBUG("Could not get runNumbers");
          if (!output->value("lumiBlocks", orig_lumiBlocks))
            ATH_MSG_DEBUG("Could not get lumiBlocks");

          // Replace content in store with content created for this stream
          *output = *m_info;
          ATH_MSG_DEBUG("FileMetaData payload replaced in store with content created for this stream");
          if (!m_filledEvent) {
            // restore original event info if it was not filled for this stream
            ATH_MSG_DEBUG("Event information was not filled, restoring what we had");
            if (!output->setValue(xAOD::FileMetaData::mcProcID, orig_mcProcID))
              ATH_MSG_DEBUG("Could not set " << xAOD::FileMetaData::mcProcID << " to " << orig_mcProcID);
            if (!output->setValue("runNumbers", orig_runNumbers))
              ATH_MSG_DEBUG("Could not restore runNumbers");
            if (!output->setValue("lumiBlocks", orig_lumiBlocks))
              ATH_MSG_DEBUG("Could not restore lumiBlocks");
          }
        } else {
            ATH_MSG_DEBUG("cannot copy FileMetaData payload to output");
        }
      }

      return StatusCode::SUCCESS;
}

StatusCode
    FileMetaDataCreatorTool::finalize() {
      return StatusCode::SUCCESS;
    }

StatusCode
    FileMetaDataCreatorTool::postExecute() {
      // Lock the tool while working with FileMetaData
      std::lock_guard lock(m_toolMutex);

      // Fill information from TagInfo and Simulation Parameters
      if (!updateFromNonEvent().isSuccess())
        ATH_MSG_DEBUG("Failed to fill FileMetaData with non-event info");

      // Sanity check
      if (!(m_info && m_aux)) {
        ATH_MSG_DEBUG("No xAOD::FileMetaData object to fill");
        return StatusCode::SUCCESS;
      }

      {  // MC channel, run and/or lumi block numbers
        const xAOD::EventInfo* eventInfo = nullptr;
        StatusCode sc = StatusCode::FAILURE;

        if (m_eventStore->contains< xAOD::EventInfo >(m_eventInfoKey))
          sc = m_eventStore->retrieve(eventInfo, m_eventInfoKey);
        else if (m_eventStore->contains< xAOD::EventInfo >("Mc" + m_eventInfoKey))
          sc = m_eventStore->retrieve(eventInfo, "Mc" + m_eventInfoKey);

        if (eventInfo && sc.isSuccess()) {
          addUniqueValue("runNumbers", eventInfo->runNumber());
          addUniqueValue("lumiBlocks", eventInfo->lumiBlock());
          // Return if object has already been filled
          if (m_filledEvent) return StatusCode::SUCCESS;

          try {
            ATH_MSG_DEBUG("Retrieved " << m_eventInfoKey);

            xAOD::FileMetaData::MetaDataType type = xAOD::FileMetaData::mcProcID;
            const float id = static_cast< float >(eventInfo->mcChannelNumber());

            if (m_info->setValue(type, id))
              ATH_MSG_DEBUG("setting " << type << " to " << id);
            else
              ATH_MSG_DEBUG("error setting " << type << " to " << id);
          } catch (std::exception&) {
            // Processing data not generated events
            ATH_MSG_DEBUG("Failed to set " << xAOD::FileMetaData::mcProcID);
          }
        } else {
          ATH_MSG_DEBUG(
            "Failed to retrieve " << m_eventInfoKey << " => cannot set "
            << xAOD::FileMetaData::mcProcID
            << ", runNumbers, or lumiBlockNumbers");
        }
      }

      m_filledEvent = true;

      return StatusCode::SUCCESS;
    }

StatusCode
    FileMetaDataCreatorTool::updateFromNonEvent() {

      // Have we already done this?
      if (m_filledNonEvent) return StatusCode::SUCCESS;

      // Sanity check
      if (!(m_info && m_aux)) {
        ATH_MSG_DEBUG("No xAOD::FileMetaData object to fill");
        return StatusCode::SUCCESS;
      }

      set(xAOD::FileMetaData::productionRelease,
          m_tagInfoMgr->findTag("AtlasRelease"));

      set(xAOD::FileMetaData::amiTag, m_tagInfoMgr->findTag("AMITag"));

      set(xAOD::FileMetaData::geometryVersion, m_tagInfoMgr->findTag("GeoAtlas"));

      set(xAOD::FileMetaData::conditionsTag,
          m_tagInfoMgr->findTag("IOVDbGlobalTag"));

      set(xAOD::FileMetaData::beamType, m_tagInfoMgr->findTag("beam_type"));

      set(xAOD::FileMetaData::mcCampaign, m_tagInfoMgr->findTag("mc_campaign"));

      set(xAOD::FileMetaData::generatorsInfo, m_tagInfoMgr->findTag("generators"));

      std::string beamEnergy = m_tagInfoMgr->findTag("beam_energy");
      try {
        set(xAOD::FileMetaData::beamEnergy,
            std::stof(beamEnergy));
      } catch (std::invalid_argument& e) {
        ATH_MSG_DEBUG("beam energy \"" << beamEnergy << "\" tag could not be converted to float");
      } catch (std::out_of_range& e) {
        ATH_MSG_DEBUG("converted beam energy value (\"" << beamEnergy << "\") outside float range");
      }

      std::string dataYear = m_tagInfoMgr->findTag("data_year");
      if (!dataYear.empty()) {
        try {
          set(xAOD::FileMetaData::dataYear,
              static_cast<uint32_t>(std::stoul(dataYear)));
        } catch (std::invalid_argument& e) {
          ATH_MSG_DEBUG("data year \"" << dataYear << "\" tag could not be converted to unsigned long");
        } catch (std::out_of_range& e) {
          ATH_MSG_DEBUG("converted data year value (\"" << dataYear << "\") outside unsigned long range");
        }
      }

      // Read simulation parameters
      const IOVMetaDataContainer * simInfo = nullptr;
      StatusCode sc = StatusCode::FAILURE;
      if (m_inputMetaDataStore->contains< IOVMetaDataContainer >(m_simInfoKey))
        sc = m_inputMetaDataStore->retrieve(simInfo, m_simInfoKey);
      const coral::AttributeList * attrList = nullptr;
      if (simInfo && sc.isSuccess())
        for (const CondAttrListCollection* payload : *simInfo->payloadContainer())
          for (const auto& itr : *payload)
            attrList = &(itr.second);
      if (attrList) {
        { // set simulation flavor
          std::string key = "SimulationFlavour";
          std::string value = "none";
          if (attrList->exists(key))
            value = (*attrList)[key].data< std::string >();

          // remap simulation flavor "default" to "FullSim"
          if (value == "default")
            value = "FullSim";

          set(xAOD::FileMetaData::simFlavour, value);
        }

        { // set whether this is overlay
          std::string key = "IsEventOverlayInputSim";
          std::string attr = "False";
          if (attrList->exists(key))
            attr = (*attrList)[key].data< std::string >();
          set(xAOD::FileMetaData::isDataOverlay, attr == "True");
        }

      } else {
          ATH_MSG_DEBUG(
            "Failed to retrieve " << m_simInfoKey << " => cannot set: "
            << xAOD::FileMetaData::simFlavour << ", and "
            << xAOD::FileMetaData::isDataOverlay
            << ". Trying to get them from input metadata store." );

          for (const std::string& key : m_metaDataSvc->getPerStreamKeysFor(m_key)) {
              const xAOD::FileMetaData* input = nullptr;
              input = m_inputMetaDataStore->tryConstRetrieve< xAOD::FileMetaData >(key);
              if (input) {
                  std::string orig_simFlavour = "none";
                  if (!input->value(xAOD::FileMetaData::simFlavour, orig_simFlavour)) {
                      ATH_MSG_DEBUG(
                          "Could not get xAOD::FileMetaData::simFlavour "
                          "from input metadata "
                          "store");
                  } else {
                      ATH_MSG_DEBUG("Retrieved from input metadata store: "
                                    << xAOD::FileMetaData::simFlavour << " = "
                                    << orig_simFlavour);
                      set(xAOD::FileMetaData::simFlavour, orig_simFlavour);
                  }

                  bool orig_isDataOverlay = false;
                  if (!input->value(xAOD::FileMetaData::isDataOverlay, orig_isDataOverlay)) {
                      ATH_MSG_DEBUG(
                          "Could not get "
                          "xAOD::FileMetaData::isDataOverlay from input "
                          "metadata store");
                  } else {
                      ATH_MSG_DEBUG("Retrieved from input metadata store: "
                                    << xAOD::FileMetaData::isDataOverlay << " = "
                                    << orig_isDataOverlay);
                      set(xAOD::FileMetaData::isDataOverlay, orig_isDataOverlay);
                  }
              }
          }
      }

      if (m_filledNonEvent) return StatusCode::SUCCESS;

      {  // get dataType
        xAOD::FileMetaData::MetaDataType type = xAOD::FileMetaData::dataType;
        try {
          if (m_info->setValue(type, m_dataHeaderKey.value()))
            ATH_MSG_DEBUG("set " << type << " to " << m_dataHeaderKey.value());
          else
            ATH_MSG_DEBUG("error setting " << type << " to " << m_dataHeaderKey.value());
        } catch (std::exception&) {
          // This is unexpected
          ATH_MSG_DEBUG("Failed to set " << xAOD::FileMetaData::dataType);
        }
      }

      // FileMetaData object has been filled with non event info
      m_filledNonEvent = true;

      return StatusCode::SUCCESS;
    }

void
    FileMetaDataCreatorTool::set(
            const xAOD::FileMetaData::MetaDataType key,
            bool value) {
      try {
        if (m_info->setValue(key, value))
          ATH_MSG_DEBUG("setting " << key << " to " << value);
        else
          ATH_MSG_DEBUG("error setting " << key << " to " << std::boolalpha << value
                        << std::noboolalpha);
      } catch (std::exception&) {
        // Processing data not generated events
        ATH_MSG_DEBUG("Failed to set " << key);
      }
    }

void
    FileMetaDataCreatorTool::set(
            const xAOD::FileMetaData::MetaDataType key,
            uint32_t value) {
      try {
        if (m_info->setValue(key, value))
          ATH_MSG_DEBUG("setting " << key << " to " << value);
        else
          ATH_MSG_DEBUG("error setting " << key << " to " << value);
      } catch (std::exception&) {
        // Processing data not generated events
        ATH_MSG_DEBUG("Failed to set " << key);
      }
    }

void
    FileMetaDataCreatorTool::set(
            const xAOD::FileMetaData::MetaDataType key,
            float value) {
      try {
        if (m_info->setValue(key, value))
          ATH_MSG_DEBUG("setting " << key << " to " << value);
        else
          ATH_MSG_DEBUG("error setting " << key << " to " << value);
      } catch (std::exception&) {
        // Processing data not generated events
        ATH_MSG_DEBUG("Failed to set " << key);
      }
    }

void
    FileMetaDataCreatorTool::set(
            const xAOD::FileMetaData::MetaDataType key,
            const std::string& value) {
      if (value.empty()) return;
      try {
        if (m_info->setValue(key, value))
          ATH_MSG_DEBUG("setting " << key << " to " << value);
        else
          ATH_MSG_DEBUG("error setting " << key << " to " << value);
      } catch (std::exception&) {
        // Processing data not generated events
        ATH_MSG_DEBUG("Failed to set " << key);
      }
    }

void FileMetaDataCreatorTool::addUniqueValue(
  const std::string& type, uint32_t value) {
  try {
    std::vector<uint32_t> list;
    if (m_info->value(type, list)) {
      ATH_MSG_DEBUG("retrieved existing list of " << type);
    } else {
      ATH_MSG_DEBUG("adding new list for " << type);
    }
    // we want a sorted list of unique values (without using std::set)
    std::sort(list.begin(), list.end());
    auto it = std::lower_bound(list.begin(), list.end(), value);
    if (it == list.end() || (*it) != value) {
      list.insert(it, value);
      ATH_MSG_DEBUG("added " << value << " to list of " << type);
    }
    if (!m_info->setValue(type, list)) {
      ATH_MSG_WARNING("error updating list for " + type);
    }
  } catch (std::exception& e) {
    // Processing generated events not data
    ATH_MSG_WARNING(e.what());
  }
}

}  // namespace xAODMaker
