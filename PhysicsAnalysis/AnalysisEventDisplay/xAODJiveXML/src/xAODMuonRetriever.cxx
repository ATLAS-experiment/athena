/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODMuonRetriever.h"
#include "xAODMuon/MuonContainer.h"
#include "AthenaKernel/Units.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  xAODMuonRetriever::xAODMuonRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent){}


  StatusCode xAODMuonRetriever::initialize(){
    ATH_CHECK(m_keys.initialize());
    return StatusCode::SUCCESS;
  }


  /**
   * For each muon collections retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODMuonRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG("In retrieve()");

    // Loop through the keys and retrieve the corresponding data
    for (const auto& key : m_keys) {
      SG::ReadHandle<xAOD::MuonContainer> cont(key);
      if (cont.isValid()) {
	DataMap data = getData(&(*cont));
	if (FormatTool->AddToEvent(dataTypeName(), key.key() + "_xAOD", &data).isFailure()) {
	  ATH_MSG_WARNING("Failed to add collection " << key.key());
	} else {
	  ATH_MSG_DEBUG(" (" << key.key() << ") retrieved");
	}
      } else {
	ATH_MSG_WARNING("Collection " << key.key() << " not found in SG");
      }
    }
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve basic parameters, mainly four-vectors, for each collection.
   * Also association with clusters and tracks (ElementLink).
   */
  const DataMap xAODMuonRetriever::getData(const xAOD::MuonContainer* muCont) {

    ATH_MSG_DEBUG("in getData()");

    DataMap DataMap;

    DataVect pt; pt.reserve(muCont->size());
    DataVect phi; phi.reserve(muCont->size());
    DataVect eta; eta.reserve(muCont->size());
    DataVect mass; mass.reserve(muCont->size());
    DataVect energy; energy.reserve(muCont->size());
    DataVect chi2; chi2.reserve(muCont->size());
    DataVect pdgId; pdgId.reserve(muCont->size());

    xAOD::MuonContainer::const_iterator muItr  = muCont->begin();
    xAOD::MuonContainer::const_iterator muItrE = muCont->end();

    int counter = 0;

    for (; muItr != muItrE; ++muItr) {

      ATH_MSG_DEBUG("  Muon #" << counter++ << " : eta = "  << (*muItr)->eta()
		    << ", phi = "  << (*muItr)->phi() << ", pt = " <<  (*muItr)->pt()
		    << ", pdgId = " << -13.*(*muItr)->trackParticle(xAOD::Muon::TrackParticleType::Primary)->charge());

      phi.emplace_back(DataType((*muItr)->phi()));
      eta.emplace_back(DataType((*muItr)->eta()));
      pt.emplace_back(DataType((*muItr)->pt()/GeV));

      mass.emplace_back(DataType((*muItr)->m()/GeV));
      energy.emplace_back( DataType((*muItr)->e()/GeV ) );
      chi2.emplace_back( 1.0 ); //placeholder
      pdgId.emplace_back(DataType( -13.*(*muItr)->trackParticle(xAOD::Muon::TrackParticleType::Primary)->charge() )); // pdgId not available anymore in xAOD
    } // end MuonIterator

    // four-vectors
    const std::size_t nEntries = phi.size();
    DataMap["phi"] = std::move(phi);
    DataMap["eta"] = std::move(eta);
    DataMap["pt"] = std::move(pt);
    DataMap["energy"] = std::move(energy);
    DataMap["mass"] = std::move(mass);
    DataMap["chi2"] = std::move(chi2);
    DataMap["pdgId"] = std::move(pdgId);

    ATH_MSG_DEBUG(" retrieved with " << nEntries << " entries");
    return DataMap;
  }
} // JiveXML namespace
