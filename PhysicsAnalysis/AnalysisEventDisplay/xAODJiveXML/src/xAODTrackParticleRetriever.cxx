/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODTrackParticleRetriever.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include "xAODTracking/TrackParticleContainer.h"

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  xAODTrackParticleRetriever::xAODTrackParticleRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent){}

  /**
   * For each TrackParticle collections retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODTrackParticleRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG("In retrieve()");

    std::vector<std::string> keys = getKeys();

    if(keys.empty()){
      ATH_MSG_WARNING("No StoreGate keys found");
      return StatusCode::SUCCESS;
    }

    // Loop through the keys and retrieve the corresponding data
    for (const std::string& key : keys) {
      SG::ReadHandle<xAOD::TrackParticleContainer> cont(key);
      if (cont.isValid()) {
	DataMap data = getData(&(*cont));
	if (FormatTool->AddToEvent(dataTypeName(), key + "_xAOD", &data).isFailure()) {
	  ATH_MSG_WARNING("Failed to retrieve Collection " << key);
	} else {
	  ATH_MSG_DEBUG(" (" << key << ") retrieved");
	}
      } else {
	ATH_MSG_WARNING("Collection " << key << " not found in SG");
      }
    }
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve basic parameters, mainly four-vectors, for each collection.
   * Also association with clusters and tracks (ElementLink).
   */
  const DataMap xAODTrackParticleRetriever::getData(const xAOD::TrackParticleContainer* TrackParticleCont) {

    ATH_MSG_DEBUG("in getData()");

    DataMap DataMap;

    DataVect d0; d0.reserve(TrackParticleCont->size());
    DataVect z0; z0.reserve(TrackParticleCont->size());
    DataVect pt; pt.reserve(TrackParticleCont->size());
    DataVect phi0; phi0.reserve(TrackParticleCont->size());
    DataVect cotTheta; cotTheta.reserve(TrackParticleCont->size());
    DataVect label; label.reserve(TrackParticleCont->size());
    DataVect nBLayerHits; nBLayerHits.reserve(TrackParticleCont->size());
    DataVect nPixHits; nPixHits.reserve(TrackParticleCont->size());
    DataVect nSCTHits; nSCTHits.reserve(TrackParticleCont->size());
    DataVect nTRTHits; nTRTHits.reserve(TrackParticleCont->size());

    xAOD::TrackParticleContainer::const_iterator TrackParticleItr  = TrackParticleCont->begin();
    xAOD::TrackParticleContainer::const_iterator TrackParticleItrE = TrackParticleCont->end();

    int counter = 0;
    float charge = 0.;
    float myQOverP = 0.;
    double countHits = 0.;
    std::string labelStr = "unknownHits";

    for (; TrackParticleItr != TrackParticleItrE; ++TrackParticleItr) {

      ATH_MSG_VERBOSE(" TrackParticle #" << counter++ << " : d0 = "  << (*TrackParticleItr)->d0() << ", z0 = "
		      << (*TrackParticleItr)->z0() << ", pt[GeV] = "  << (*TrackParticleItr)->pt()*(1./CLHEP::GeV)
		      << ", phi = "  << (*TrackParticleItr)->phi()
		      << ", qOverP = "  << (*TrackParticleItr)->qOverP()
		      << ", abs(qOverP) = "  << fabs((*TrackParticleItr)->qOverP()));

      // Event/xAOD/xAODTrackingCnv/trunk/src/TrackParticleCnvAlg.cxx#L190 Info from Nick Styles

      uint8_t numberOfBLayerHits=0;
      uint8_t numberOfBLayerHits_tmp=0;
      if ( (*TrackParticleItr)->summaryValue(numberOfBLayerHits_tmp,xAOD::numberOfBLayerHits) ){
	numberOfBLayerHits += numberOfBLayerHits_tmp;
      }
      uint8_t numberOfPixelHits = 0;
      uint8_t numberOfPixelHits_tmp = 0;
      if ( (*TrackParticleItr)->summaryValue(numberOfPixelHits_tmp,xAOD::numberOfPixelHits)){
	numberOfPixelHits += numberOfPixelHits_tmp;
      }
      uint8_t numberOfTRTHits = 0;
      uint8_t numberOfTRTHits_tmp = 0;
      if ( (*TrackParticleItr)->summaryValue(numberOfTRTHits_tmp,xAOD::numberOfTRTHits)){
	numberOfTRTHits += numberOfTRTHits_tmp;
      }
      uint8_t numberOfSCTHits = 0;
      uint8_t numberOfSCTHits_tmp = 0;
      if ( (*TrackParticleItr)->summaryValue(numberOfSCTHits_tmp,xAOD::numberOfSCTHits)){
	numberOfSCTHits += numberOfSCTHits_tmp;
      }
      labelStr = "_PixelHits"+DataType( (double)numberOfPixelHits ).toString()
	+ "_SCTHits"+DataType( (double)numberOfSCTHits ).toString()
	+ "_BLayerHits"+DataType( (double)numberOfBLayerHits ).toString()
	+ "_TRTHits"+DataType( (double)numberOfTRTHits ).toString() ;

      countHits = (double)numberOfBLayerHits + (double)numberOfPixelHits
	+ (double)numberOfSCTHits + (double)numberOfTRTHits;

      ATH_MSG_VERBOSE(" TrackParticle #" << counter
		      << " BLayer hits: " << (double)numberOfBLayerHits
		      << ", Pixel hits: " << (double)numberOfPixelHits
		      << ", SCT hits: " << (double)numberOfSCTHits
		      << ", TRT hits: " << (double)numberOfTRTHits
		      << ", Total hits: " << countHits
		      << ";  Label: " << labelStr);

      nBLayerHits.emplace_back( DataType( (double)numberOfBLayerHits ));
      nPixHits.emplace_back( DataType( (double)numberOfPixelHits ));
      nSCTHits.emplace_back( DataType( (double)numberOfSCTHits ));
      nTRTHits.emplace_back( DataType( (double)numberOfTRTHits ));

      label.emplace_back( DataType( labelStr ).toString() );
      d0.emplace_back(DataType((*TrackParticleItr)->d0()/CLHEP::cm));
      z0.emplace_back(DataType((*TrackParticleItr)->z0()/CLHEP::cm));
      phi0.emplace_back(DataType((*TrackParticleItr)->phi()));

      // pt is always positive, get charge from qOverP
      myQOverP = (*TrackParticleItr)->qOverP() ;
      if (fabs(myQOverP) != myQOverP){
	charge = -1.;
      }else{
	charge = +1.;
      }
      pt.emplace_back(DataType(( charge*(*TrackParticleItr)->pt() )/CLHEP::GeV));

      if ( (*TrackParticleItr)->theta()  == 0.) {
	cotTheta.emplace_back(DataType(9999.));
      } else {
	cotTheta.emplace_back(DataType(1./tan((*TrackParticleItr)->theta())));
      }
    } // end TrackParticleIterator

      // four-vectors
    DataMap["d0"] = d0;
    DataMap["z0"] = z0;
    DataMap["pt"] = pt;
    DataMap["phi0"] = phi0;
    DataMap["cotTheta"] = cotTheta;
    DataMap["label"] = label;
    DataMap["nBLayerHits"] = nBLayerHits;
    DataMap["nPixHits"] = nPixHits;
    DataMap["nSCTHits"] = nSCTHits;
    DataMap["nTRTHits"] = nTRTHits;

    ATH_MSG_DEBUG(dataTypeName() << " retrieved with " << d0.size() << " entries");

    //All collections retrieved okay
    return DataMap;

  }


  const std::vector<std::string> xAODTrackParticleRetriever::getKeys() {
    ATH_MSG_DEBUG("in getKeys()");

    std::vector<std::string> keys = {};

    // Remove m_priorityKey from m_otherKeys if it exists, we don't want to write it twice
    auto it = std::find(m_otherKeys.begin(), m_otherKeys.end(), m_priorityKey);
    if(it != m_otherKeys.end()){
      m_otherKeys.erase(it);
    }

    // Add m_priorityKey as the first element if it is not ""
    if(m_priorityKey!=""){
      keys.push_back(m_priorityKey);
    }

    if(!m_otherKeys.empty()){
      keys.insert(keys.end(), m_otherKeys.begin(), m_otherKeys.end());
    }

    // If all collections are requested, obtain all available keys from StoreGate
    std::vector<std::string> allKeys;
    if(m_doWriteAllCollections){
      evtStore()->keys<xAOD::TrackParticleContainer>(allKeys);
      // Add keys that are not the priority key and do not add containers with "HLT" in their name if requested
      for(const std::string& key : allKeys){
	// Don't include key if it's already in keys
	auto it2 = std::find(keys.begin(), keys.end(), key);
	if(it2 != keys.end())continue;
	if(key.find("HLT") == std::string::npos || m_doWriteHLT){
	  keys.emplace_back(key);
	}
      }
    }
    return keys;
  }

} // JiveXML namespace
