/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#ifndef XAOD_STANDALONE
  #include "AthAnalysisBaseComps/AthAnalysisHelper.h"
  #include "EventInfo/EventInfo.h"
  #include "EventInfo/EventType.h"
#endif

// Local include(s):
#include <PATInterfaces/SystematicRegistry.h>
#include <regex>
#include <PMGTools/WeightHelpers.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODMetaData/FileMetaData.h>

#include <PMGTools/PMGTruthWeightTool.h>


namespace PMGTools
{
  PMGTruthWeightTool::PMGTruthWeightTool(const std::string& name)
    : asg::AsgMetadataTool(name)
  {
    declareProperty("MetaObjectName", m_metaName = "TruthMetaData");
  }


  StatusCode PMGTruthWeightTool::initialize()
  {
    ATH_MSG_DEBUG("Initialising...");

    ATH_MSG_INFO("Attempting to retrieve truth meta data from the first file...");

    // Clear cached weights
    clearWeightLocationCaches();

    CP::SystematicSet affSysts = CP::SystematicSet();
    m_calibCache.initialize (std::move (affSysts),
                             [this] (const CP::SystematicSet& sys,
                                     std::size_t& idx) {
                               if (!sys.name().empty()) ATH_MSG_WARNING("Mapping for " << sys.name() << " missing, setting to index 0.");
                               idx = 0;
                               return StatusCode::SUCCESS;
                             });

    // Try to load MC channel number from file metadata
    ATH_CHECK(getMCChannelNumber(m_mcChannelNumber));
    
    if (m_mcChannelNumber == uint32_t(-1)) {
      ATH_MSG_WARNING("... MC channel number could not be loaded");
    } else {
      ATH_MSG_INFO("... MC channel number identified as " << m_mcChannelNumber);
    }

    // Start by trying to load metadata from the store
    m_metaDataContainer = nullptr;
    if (inputMetaStore()->contains<xAOD::TruthMetaDataContainer>(m_metaName)) {
      ATH_CHECK(inputMetaStore()->retrieve(m_metaDataContainer, m_metaName));
      ATH_MSG_INFO("Loaded xAOD::TruthMetaDataContainer");

      // Check for incorrectly stored McChannelNumber
      m_useChannelZeroInMetaData = true;
      for (auto truthMetaData : *m_metaDataContainer) {
        if (truthMetaData->mcChannelNumber() != 0) { m_useChannelZeroInMetaData = false; }
      }
      // If we only have one metadata item take MC channel from there if needed
      if (m_mcChannelNumber == uint32_t(-1) && m_metaDataContainer->size() == 1) {
        m_mcChannelNumber = m_metaDataContainer->at(0)->mcChannelNumber();
        ATH_MSG_WARNING("... MC channel number taken from the metadata as " << m_mcChannelNumber);
      }
      if (m_useChannelZeroInMetaData) { ATH_MSG_WARNING("MC channel number in TruthMetaData is invalid - assuming that channel 0 has the correct information."); }

      // Load metadata from TruthMetaDataContainer if we have a valid channel number or if we're going to use 0 anyway
      // ... otherwise wait until we can load a channel number from EventInfo
      if (m_mcChannelNumber != uint32_t(-1) || m_useChannelZeroInMetaData) {
        if (loadMetaData().isFailure()) {
          ATH_MSG_ERROR("Could not load metadata for MC channel number " << m_mcChannelNumber);
          return StatusCode::FAILURE;
        }
      }
    } else {
      // ... now try to load the weight container using the POOL metadata (not possible in AnalysisBase)
      // see https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/AthAnalysisBase#How_to_read_the_truth_weight_nam
      if (loadPOOLMetaData().isFailure()) {
        ATH_MSG_ERROR("Could not load POOL HepMCWeightNames");
        return StatusCode::FAILURE;
      }
    }
    
    // Add the affecting systematics to the global registry
    CP::SystematicRegistry& registry = CP::SystematicRegistry::getInstance();
    if (!registry.registerSystematics(*this)) {
      ATH_MSG_ERROR("unable to register the systematics");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Successfully initialized!");

    return StatusCode::SUCCESS;
  }


  const std::vector<std::string>& PMGTruthWeightTool::getWeightNames() const {
    return m_weightNames;
  }


  float PMGTruthWeightTool::getWeight(const xAOD::EventInfo* evtInfo, const std::string& weightName) const {
    // Return appropriate weight from EventInfo: this should be identical to the TruthEvent
    try {
      return evtInfo->mcEventWeight(m_weightIndices.at(weightName));
    } catch (const std::out_of_range& e) {
      // Before throwing an exception, try to recover with bad naming conventions
      std::string strippedName = std::regex_replace(weightName, std::regex(" "), "_");
      std::transform(strippedName.begin(), strippedName.end(), strippedName.begin(),
        [](unsigned char c){ return std::tolower(c); });
      for (const std::string &weight : m_weightNames) {
        std::string modifiedName = std::regex_replace(weight, std::regex(" "), "_");
        std::transform(modifiedName.begin(), modifiedName.end(), modifiedName.begin(),
          [](unsigned char c){ return std::tolower(c); });
        if (strippedName == modifiedName){
          ATH_MSG_WARNING("Using weight name \"" << weight << "\" instead of requested \"" << weightName << "\"");
          return getWeight(evtInfo, weight);
        }
      }
      ATH_MSG_FATAL("Weight \"" + weightName + "\" could not be found");
      throw std::runtime_error(name() + ": Weight \"" + weightName + "\" could not be found");
    }
  }


  bool PMGTruthWeightTool::hasWeight(const std::string& weightName) const {
    return (m_weightIndices.count(weightName) > 0);
  }


  float PMGTruthWeightTool::getSysWeight(const xAOD::EventInfo* evtInfo, const CP::SystematicSet& sys) const
  {
    const std::size_t *res;
    ANA_CHECK_THROW (m_calibCache.get(sys,res));
    return evtInfo->mcEventWeight(*res);
  }


  size_t PMGTruthWeightTool::getSysWeightIndex(const CP::SystematicSet& sys) const
  {
    const std::size_t *res;
    ANA_CHECK_THROW (m_calibCache.get(sys,res));
    return *res;
  }


  CP::SystematicSet PMGTruthWeightTool::affectingSystematics() const
  {
    return m_calibCache.affectingSystematics();
  }


  CP::SystematicSet PMGTruthWeightTool::recommendedSystematics() const
  {
    return affectingSystematics();
  }


  StatusCode PMGTruthWeightTool::getMCChannelNumber(uint32_t &mcChannelNumber) {
    mcChannelNumber = static_cast<uint32_t>(-1);

    // Try to load MC channel number from file metadata
    ATH_MSG_INFO("Attempting to retrieve MC channel number...");
    const xAOD::FileMetaData *fmd = nullptr;
    if (inputMetaStore()->contains<xAOD::FileMetaData>("FileMetaData")) {
      ATH_CHECK(inputMetaStore()->retrieve(fmd, "FileMetaData"));
      float fltChannelNumber(-1);
      if (fmd->value(xAOD::FileMetaData::mcProcID, fltChannelNumber)) {
        mcChannelNumber = static_cast<uint32_t>(fltChannelNumber);
        return StatusCode::SUCCESS;
      }
    }
    return StatusCode::FAILURE;
  }

  StatusCode PMGTruthWeightTool::beginInputFile()
  {
    // Detect possible MC channel number change
    uint32_t mcChannelNumber;
    ATH_CHECK(getMCChannelNumber(mcChannelNumber));

    if (m_mcChannelNumber != uint32_t(-1) && mcChannelNumber != uint32_t(-1) && mcChannelNumber != m_mcChannelNumber) {
      ATH_MSG_ERROR("MC channel number from a new file does not match the previously processed files.");
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }


  StatusCode PMGTruthWeightTool::loadMetaData() {
    // Find the correct truth meta data object
    uint32_t targetChannelNumber = (m_useChannelZeroInMetaData ? 0 : m_mcChannelNumber);
    ATH_MSG_INFO("Attempting to load weight meta data from xAOD TruthMetaData for channel " << targetChannelNumber);
    auto itTruthMetaDataPtr = std::find_if(m_metaDataContainer->begin(), m_metaDataContainer->end(),
      [targetChannelNumber] (const auto& it) { return it->mcChannelNumber() == targetChannelNumber; }
    );

    // If no such object is found then return
    if (itTruthMetaDataPtr == m_metaDataContainer->end()) {
      ATH_MSG_ERROR("Could not load weight meta data!");
      return StatusCode::FAILURE;
    }

    // Update cached weight data
    const std::vector<std::string> &truthWeightNames = (*itTruthMetaDataPtr)->weightNames();
    for(std::size_t idx = 0; idx < truthWeightNames.size(); ++idx ) {
      m_weightNames.push_back(truthWeightNames.at(idx));
      m_weightIndices[truthWeightNames.at(idx)] = idx;

      std::string sysName = weightNameWithPrefix(truthWeightNames.at(idx));
      if (!sysName.empty()) {
        ANA_CHECK (m_calibCache.add(CP::SystematicVariation(sysName),idx));
      }
      
      ANA_MSG_VERBOSE("    " << truthWeightNames.at(idx) << " " << sysName);
    }
    return this->validateWeightLocationCaches();
  }


  StatusCode PMGTruthWeightTool::loadPOOLMetaData() {
    // AnalysisBase can only use the xAOD::TruthMetaDataContainer, so skip this
#ifdef XAOD_STANDALONE
    return StatusCode::SUCCESS;
#else
    ATH_MSG_INFO("Looking for POOL HepMC IOVMetaData...");
    std::map<std::string, int> hepMCWeightNamesMap;
    if (AAH::retrieveMetadata("/Generation/Parameters", "HepMCWeightNames", hepMCWeightNamesMap, inputMetaStore()).isFailure()) {
      ATH_MSG_FATAL("Cannot access metadata " << m_metaName << " and failed to get names from IOVMetadata");
      return StatusCode::FAILURE;
    }

    // Use input map to fill the index map and the weight names
    ATH_MSG_INFO("Attempting to load weight meta data from HepMC IOVMetaData container");
    for (auto& kv : hepMCWeightNamesMap) {
      m_weightNames.push_back(kv.first);
      m_weightIndices[kv.first] = kv.second;

      std::string sysName = weightNameWithPrefix(kv.first);
      if (!sysName.empty()) {
        ANA_CHECK (m_calibCache.add(CP::SystematicVariation(sysName), kv.second));
      }

      ANA_MSG_VERBOSE("    " << kv.first << " " << sysName);
    }
    return this->validateWeightLocationCaches();
#endif // XAOD_STANDALONE
  }


  StatusCode PMGTruthWeightTool::validateWeightLocationCaches() {
    // Validate weight caches against one another
    if (m_weightNames.size() != m_weightIndices.size()) {
      ATH_MSG_ERROR("Found " << m_weightNames.size() << " but " << m_weightIndices.size() << " weight indices!");
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Successfully loaded information about " << m_weightNames.size() << " weights");
    return StatusCode::SUCCESS;
  }


  void PMGTruthWeightTool::clearWeightLocationCaches() {
    m_weightNames.clear();
    m_weightIndices.clear();
  }

} // namespace PMGTools
