/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODPhotonRetriever.h"
#include "xAODEgamma/PhotonContainer.h"
#include "AthenaKernel/Units.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  xAODPhotonRetriever::xAODPhotonRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent){}


  StatusCode xAODPhotonRetriever::initialize(){
    ATH_CHECK(m_keys.initialize());
    return StatusCode::SUCCESS;
  }

  
  /**
   * For each photon collection retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODPhotonRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG("In retrieve()");

    // Loop through the keys and retrieve the corresponding data
    for (const auto& key : m_keys) {
      SG::ReadHandle<xAOD::PhotonContainer> cont(key);
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
  const DataMap xAODPhotonRetriever::getData(const xAOD::PhotonContainer* phCont) {

    ATH_MSG_DEBUG("in getData()");

    DataMap DataMap;

    DataVect pt; pt.reserve(phCont->size());
    DataVect phi; phi.reserve(phCont->size());
    DataVect eta; eta.reserve(phCont->size());
    DataVect mass; mass.reserve(phCont->size());
    DataVect energy; energy.reserve(phCont->size());

    DataVect isEMString; isEMString.reserve(phCont->size());
    DataVect author; author.reserve(phCont->size());
    DataVect label; label.reserve(phCont->size());

    xAOD::PhotonContainer::const_iterator phItr  = phCont->begin();
    xAOD::PhotonContainer::const_iterator phItrE = phCont->end();

    int counter = 0;
    const std::string mediumStr{"Medium"};
    const std::string tightStr{"Tight"};
    const std::string looseStr{"Loose"};
    auto prefix = [](const std::string & root)->std::string{
      return '_'+root;
    };
    for (; phItr != phItrE; ++phItr) {
      ATH_MSG_DEBUG("  Photon #" << counter++ << " : eta = "  << (*phItr)->eta() << ", phi = "
		    << (*phItr)->phi());

      std::string photonAuthor = "";
      std::string photonIsEMString = "none";
      std::string photonLabel = "";

      phi.emplace_back(DataType((*phItr)->phi()));
      eta.emplace_back(DataType((*phItr)->eta()));
      pt.emplace_back(DataType((*phItr)->pt()/GeV));

      bool passesTight(false);
      bool passesMedium(false);
      bool passesLoose(false);
      const bool tightSelectionExists = (*phItr)->passSelection(passesTight, tightStr);
      ATH_MSG_VERBOSE("tight exists " << tightSelectionExists
		      << " and passes? " << passesTight);
      const bool mediumSelectionExists = (*phItr)->passSelection(passesMedium, mediumStr);
      ATH_MSG_VERBOSE("medium exists " << mediumSelectionExists
		      << " and passes? " << passesMedium);
      const bool looseSelectionExists = (*phItr)->passSelection(passesLoose, looseStr);
      ATH_MSG_VERBOSE("loose exists " << looseSelectionExists
		      << " and passes? " << passesLoose);

      photonAuthor = "author"+DataType( (*phItr)->author() ).toString(); // for odd ones eg FWD
      photonLabel = photonAuthor;
      if (( (*phItr)->author()) == 0){ photonAuthor = "unknown"; photonLabel += "_unknown"; }
      if (( (*phItr)->author()) == 8){ photonAuthor = "forward"; photonLabel += "_forward"; }
      if (( (*phItr)->author()) == 2){ photonAuthor = "softe"; photonLabel += "_softe"; }
      if (( (*phItr)->author()) == 1){ photonAuthor = "egamma"; photonLabel += "_egamma"; }

      if ( passesLoose ){
	photonLabel += prefix(looseStr);
	photonIsEMString = looseStr; // assume that hierarchy is obeyed !
      }
      if ( passesMedium ){
	photonLabel += prefix(mediumStr);
	photonIsEMString = mediumStr; // assume that hierarchy is obeyed !
      }
      if ( passesTight ){
	photonLabel += prefix(tightStr);
	photonIsEMString = tightStr; // assume that hierarchy is obeyed !
      }
      author.emplace_back( DataType( photonAuthor ) );
      label.emplace_back( DataType( photonLabel ) );
      isEMString.emplace_back( DataType( photonIsEMString ) );

      mass.emplace_back(DataType((*phItr)->m()/GeV));
      energy.emplace_back( DataType((*phItr)->e()/GeV ) );
    } // end PhotonIterator

    // four-vectors
    std::size_t nEntries  = phi.size();
    DataMap["phi"] = std::move(phi);
    DataMap["eta"] = std::move(eta);
    DataMap["pt"] = std::move(pt);
    DataMap["energy"] = std::move(energy);
    DataMap["mass"] = std::move(mass);
    DataMap["isEMString"] = std::move(isEMString);
    DataMap["label"] = std::move(label);
    DataMap["author"] = std::move(author);

    ATH_MSG_DEBUG(dataTypeName() << " retrieved with " << nEntries << " entries");
    return DataMap;

  }


} // JiveXML namespace
