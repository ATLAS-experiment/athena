/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelDistortionAlg.h"
#include "GaudiKernel/EventIDRange.h"

// Random Number Generation
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"

#include "CLHEP/Random/RandGaussZiggurat.h"
#include "PathResolver/PathResolver.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "InDetIdentifier/PixelID.h"
#include <cmath>

#include <cstdint>
#include <istream>
#include <map>
#include <string>

PixelDistortionAlg::PixelDistortionAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthAlgorithm(name, pSvcLocator)
{
}

StatusCode PixelDistortionAlg::initialize() {
  ATH_MSG_DEBUG("PixelDistortionAlg::initialize()");

  ATH_CHECK(detStore()->retrieve(m_pixelID,"PixelID"));

  ATH_CHECK(m_rndmSvc.retrieve());
  ATH_CHECK(m_readKey.initialize());
  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PixelDistortionAlg::execute() {
  ATH_MSG_DEBUG("PixelDistortionAlg::execute()");

  SG::WriteCondHandle<PixelDistortionData> writeHandle(m_writeKey);
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS; 
  }

  // Construct the output Cond Object and fill it in
  std::unique_ptr<PixelDistortionData> writeCdo(std::make_unique<PixelDistortionData>());



  constexpr int nmodule_max = 2048;
  std::unordered_map<uint32_t,std::vector<float>> distortionMap;
  std::unordered_map<uint32_t,unsigned long long> ids;
  if (m_distortionInputSource==0) { // no bow correction
    ATH_MSG_DEBUG("No bow correction");
    writeCdo -> setVersion(m_distortionVersion);
    for (int i=0; i<nmodule_max; i++) {
      distortionMap[i].push_back(0.0);
      distortionMap[i].push_back(0.0);
      distortionMap[i].push_back(0.0);
    }
  }
  else if (m_distortionInputSource==1) { // constant bow
    ATH_MSG_DEBUG("Using constant pixel distortions ");
    writeCdo -> setVersion(m_distortionVersion);
    for (int i=0; i<nmodule_max; i++) {
      distortionMap[i].push_back(0.1); //  1/mm
      distortionMap[i].push_back(0.1); //  1/mm
      distortionMap[i].push_back(2.0*std::atan(0.0005)/CLHEP::degree); // convert to degree
    }
  }
  else if (m_distortionInputSource==2) { // read from file
    const std::string &file_name = m_inputFileName;
    if (file_name.empty()) {
      ATH_MSG_ERROR("Distortion filename is empty  not found! No pixel distortion will be applied.");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Reading pixel distortions from file: " << file_name);
    writeCdo -> setVersion(m_distortionVersion);

    if (file_name[0] != '/') {
      PathResolver::find_file(file_name, "DATAPATH");
    }
    std::ifstream input(file_name);
    if (!input.good()) {
      ATH_MSG_ERROR("Cannot open " << file_name);
      return StatusCode::FAILURE;
    }

    int distosize;
    if (m_distortionVersion < 2) distosize = 3;
    else distosize = 441;

    while (!input.eof()) {
      unsigned int idmod{};
      unsigned int hashID {};
      float data{};

      if (m_distortionVersion == 1) {
        input >> idmod;
        hashID = idmod;
      } else {
        input >> std::hex >> idmod >> std::dec;
        hashID = m_pixelID->wafer_hash((Identifier)idmod); 
      }
      Identifier modId = m_pixelID->wafer_id((IdentifierHash)hashID);
      ids[hashID] = modId.get_compact();
      ATH_MSG_DEBUG("Identifier = 0x" << std::hex << ids[hashID] << std::dec);

      std::stringstream s;
      for (int i = 0; i < distosize; ++i) {
        input >> data;
        s << data << " ";
        distortionMap[hashID].push_back(data);
      }
      ATH_MSG_DEBUG(s.str());
    }
    input.close();
  }
  else if (m_distortionInputSource==3) { // random generation
    ATH_MSG_DEBUG("Using random pixel distortions");
    writeCdo -> setVersion(m_distortionVersion);

    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this);
    rngWrapper->setSeed(name(),Gaudi::Hive::currentContext());
    CLHEP::HepRandomEngine *rndmEngine = *rngWrapper;
    //these numbers could become properties, but seems unnecessary now (they are the same in all cases):
    constexpr double distortionMeanR{0.12/CLHEP::meter};
    constexpr double distortionRMSR{0.08};
    constexpr double distortionMeanTwist{-0.0005};
    
    for (int i=0; i<nmodule_max; i++) {
      float r1    = CLHEP::RandGaussZiggurat::shoot(rndmEngine,distortionMeanR,distortionRMSR);
      float r2    = CLHEP::RandGaussZiggurat::shoot(rndmEngine,r1,distortionRMSR/10.);//to implement a correlation between distortions on 2 sides of the module
      float twist = CLHEP::RandGaussZiggurat::shoot(rndmEngine,distortionMeanTwist,distortionMeanTwist);
      distortionMap[i].push_back(r1*CLHEP::meter); // convert to 1/mm
      distortionMap[i].push_back(r2*CLHEP::meter); // convert to 1/mm
      distortionMap[i].push_back(2.0*std::atan(twist)/CLHEP::degree); // convert to degree
    }
  }
  else if (m_distortionInputSource==4) { // read from database here 
    ATH_MSG_DEBUG("Using pixel distortions from database");
    SG::ReadCondHandle<DetCondCFloat> readHandle(m_readKey);
    const DetCondCFloat* readCdo = *readHandle; 
    if (readCdo==nullptr) {
      ATH_MSG_FATAL("Null pointer to the read conditions object");
      return StatusCode::FAILURE;
    }
    // Get the validitiy range
    ATH_MSG_DEBUG("Size of DetCondCFloat " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());

    int version = 0;
    if (readCdo->tag()=="/Indet/PixelDist") {
      version = 0; // For reproducing bug in earlier versions for backward compatibility 
      ATH_MSG_INFO("Detected old version of pixel distortions data.");
    } 
    else {
      bool gotVersion = false;
      // Get version number, expecting string to have the form /Indet/PixelDist_v#
      // If not recongnized will default to latest version.
      std::string baseStr = "/Indet/PixelDist_v";
      if (readCdo->tag().compare(0,baseStr.size(),baseStr)==0) {
        std::istringstream istr(readCdo->tag().substr(baseStr.size()));
        int version_tmp = 0;
        istr >> version_tmp;
        if (istr.eof()) { // Should be have read whole stream if its a number
          version = version_tmp;
          gotVersion = true;
        }
      }
      if (!gotVersion) {
        ATH_MSG_WARNING("Unable to determine version number of pixel distortions data. Version string:  " << readCdo->tag());
      }
    }
    writeCdo -> setVersion(version);
    ATH_MSG_DEBUG("Distortions data version = " << version);

    int distosize = readCdo->size();

    for (int i=0; i<nmodule_max; i++) {
      if (readCdo->find(m_pixelID->wafer_id(IdentifierHash(i)))) {
        Identifier modId = m_pixelID->wafer_id((IdentifierHash)i);
        const float *disto = readCdo->find(modId);
        ids[i] = modId.get_compact();

        ATH_MSG_DEBUG("Identifier = 0x" << std::hex << ids[i] << std::dec);
        std::stringstream s;
        for (int j = 0; j < distosize; ++j) {
          distortionMap[i].push_back(disto[j]);
          s << disto[j] << " ";
        }
        ATH_MSG_DEBUG(s.str());
      }
    }
  }
  writeCdo -> setDistortionMap(distortionMap);
  writeCdo -> setIds(ids);

  if (m_writeToFile) {
    std::ofstream* outfile = new std::ofstream("output_distortion.txt"); 
    for (int i=0; i<nmodule_max; i++) {
      if (!distortionMap[i].empty()) {
        if (m_distortionVersion==0) {
          *outfile << m_pixelID->wafer_id(IdentifierHash(i)) << " " << distortionMap[i].at(0) << " " << distortionMap[i].at(1) << " " << distortionMap[i].at(2) << std::endl;
        }
        else if (m_distortionVersion>0) {
          *outfile << i << " " << distortionMap[i].at(0) << " " << distortionMap[i].at(1) << " " << distortionMap[i].at(2) << std::endl;
        }
      }
    }
    outfile->close(); 
    delete outfile; 
  }

  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 0,                       0,                       EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop {EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDRange rangeW{start, stop};

  if (writeHandle.record(rangeW, std::move(writeCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record PixelDistortionData " << writeHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

  return StatusCode::SUCCESS;
}

