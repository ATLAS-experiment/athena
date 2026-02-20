/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ITkPixelOfflineCalibCondAlg.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "GaudiKernel/EventIDRange.h"
#include "PathResolver/PathResolver.h"
#include <memory>
#include <sstream>

namespace ITk
{

PixelOfflineCalibCondAlg::PixelOfflineCalibCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthCondAlgorithm(name, pSvcLocator)
{
}


StatusCode PixelOfflineCalibCondAlg::initialize() {
  ATH_MSG_DEBUG("ITkPixelOfflineCalibCondAlg::initialize()");

  ATH_CHECK(detStore()->retrieve(m_pixelid, "PixelID")) ;

  if (m_inputSource==2 && m_readKey.key().empty()) {
    ATH_MSG_ERROR("The database is set to be input source (2) but the ReadKey is empty.");
  }
  ATH_CHECK(m_readKey.initialize());

  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}



StatusCode PixelOfflineCalibCondAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("ITkPixelOfflineCalibCondAlg::execute()");

  SG::WriteCondHandle<PixelOfflineCalibData> writeHandle(m_writeKey, ctx);
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }

  // Construct the output Cond Object and fill it in
  std::unique_ptr<PixelOfflineCalibData> writeCdo(std::make_unique<PixelOfflineCalibData>());

  if (m_inputSource==0) {
    ATH_MSG_WARNING("So far do nothing!! return StatusCode::FAILURE");
    return StatusCode::FAILURE;
  }
  else if (m_inputSource==1) {
    ATH_MSG_WARNING("Pixel ITk constants read from text file. Only supported for local developments and debugging!");

    auto calibData = std::make_unique<PixelOfflineCalibData>();

    // Find and open the text file
    ATH_MSG_INFO("Load ITkPixelErrorData constants from text file");
    std::string fileName = PathResolver::find_file(m_textFileName, "DATAPATH");
    if (fileName.empty()) { ATH_MSG_WARNING("Input file " << fileName << " not found! Default (hardwired) values to be used!"); }
    else {
       unsigned int n_entries = calibData->getClusterErrorData()->load(fileName);
       ATH_MSG_DEBUG("Loaded " << n_entries << " from " << fileName << ".");
    }

    const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 0,                       0,                       EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
    const EventIDBase stop {EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
    const EventIDRange rangeW{start, stop};

    ATH_MSG_DEBUG("Range of input is " << rangeW);

    if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
      ATH_MSG_FATAL("Could not record PixelCalib::ITkPixelOfflineCalibData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

    if (m_dump!=0) {
      ATH_MSG_DEBUG("Dump the constants to file");
      calibData->dump();
    }
  }

  else if (m_inputSource==2) {

    SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey, ctx};
    const CondAttrListCollection* readCdo{*readHandle};
    if (readCdo==nullptr) {
      ATH_MSG_FATAL("Null pointer to the read conditions object");
      return StatusCode::FAILURE;
    }

    // Get the validitiy range
    EventIDRange rangeW;
    if (not readHandle.range(rangeW)) {
       ATH_MSG_FATAL("Failed to retrieve validity range for " << readHandle.key());
       return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Range of input is " << rangeW);

    for(const auto & attrList : *readCdo){

      std::ostringstream attrStr;
      attrList.second.toOutputStream(attrStr);
      ATH_MSG_DEBUG( "ChanNum " << attrList.first << " Attribute list " << attrStr.str() );

      std::string stringData = attrList.second["data_array"].data<std::string>();
      std::string delimiter = "],";
      size_t pos = 0;
      std::vector<std::string> component;
      std::string buffer;
      while ((pos = stringData.find(delimiter)) != std::string::npos) {
	buffer = stringData.substr(0,pos);
	component.push_back(buffer);
	stringData.erase(0, pos + delimiter.length());
      }
      component.push_back(stringData);
      ATH_MSG_INFO("Last component="<<stringData);

      for (auto & i : component) {
	std::string checkModule = i;
	std::vector<std::string> moduleString;
	delimiter = ":[";
	pos = 0;
	while ((pos = checkModule.find(delimiter)) != std::string::npos) {
	  buffer = checkModule.substr(0,pos);
	  moduleString.push_back(buffer);
	  checkModule.erase(0, pos + delimiter.length());
	}
	moduleString.push_back(std::move(checkModule));

	if (moduleString.size()!=2) {
	  ATH_MSG_FATAL("String size (moduleString) is not 2. " << moduleString.size() << " in " << i << " channel " <<  attrList.first << " read from " << readHandle.fullKey());
	  return StatusCode::FAILURE;
	}

	std::stringstream checkModuleHash(moduleString[0]);
	std::vector<std::string> moduleStringHash;
	while (std::getline(checkModuleHash,buffer,'"')) { moduleStringHash.push_back(buffer); }

	int waferHash   = std::atoi(moduleStringHash[1].c_str());
	IdentifierHash waferID_hash(waferHash);

	std::stringstream moduleConstants(moduleString[1]);
	std::vector<double> moduleConstantsVec;
	while (std::getline(moduleConstants,buffer,',')) {  moduleConstantsVec.emplace_back(std::atof(buffer.c_str())); }


        std::array<double, ITk::PixelClusterErrorData::kNParam> param{};
        std::span<double> param_src(param);

	// Format v1 with no incident angle dependance
	if(moduleConstantsVec.size()==4){
	  param[ITk::PixelClusterErrorData::kDelta_x_offset]= moduleConstantsVec[0]; // delta_x_offset
	  param[ITk::PixelClusterErrorData::kError_x] = moduleConstantsVec[1]; // delta_error_x
	  param[ITk::PixelClusterErrorData::kDelta_y_offset] = moduleConstantsVec[2]; // delta_y_offset
	  param[ITk::PixelClusterErrorData::kError_y] = moduleConstantsVec[3]; // delta_error_y
	}

	else if(moduleConstantsVec.size()==7){
          assert( moduleConstantsVec.size()+1 == ITk::PixelClusterErrorData::kNParam);
          assert( ITk::PixelClusterErrorData::kPeriod_phi == 0 );
          assert( ITk::PixelClusterErrorData::kPeriod_sinheta == 1 );
          param[0] = moduleConstantsVec[0]; // first value used for kPeriod_phi and kPeriod_sinheta
          for (unsigned int idx=0; idx<moduleConstantsVec.size(); ++idx) {
             param[idx+1] = moduleConstantsVec[idx];
          }
	}

	// Format v3 with incident angle dependance + different eta-phi periods
	else if(moduleConstantsVec.size()==8){
           param_src = std::span<double>(moduleConstantsVec.begin(), moduleConstantsVec.end());
	}
        assert(writeCdo->getClusterErrorData());
        writeCdo->getClusterErrorData()->setDeltaError(waferID_hash,param_src);
      }

    }


    if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
      ATH_MSG_FATAL("Could not record PixelCalib::ITkPixelOfflineCalibData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

  }

  return StatusCode::SUCCESS;
}

} // namespace ITk
