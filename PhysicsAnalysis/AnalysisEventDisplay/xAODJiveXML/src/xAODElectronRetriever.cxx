/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODElectronRetriever.h"
#include "xAODEgamma/ElectronContainer.h"
#include "AthenaKernel/Units.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  xAODElectronRetriever::xAODElectronRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent)
  {}


  StatusCode xAODElectronRetriever::initialize(){
    ATH_CHECK(m_keys.initialize());
    return StatusCode::SUCCESS;
  }

  /**
   * For each electron collection retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODElectronRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG("in retrieve()");

    // Loop through the keys and retrieve the corresponding data
    for (const auto& key : m_keys) {
      SG::ReadHandle<xAOD::ElectronContainer> cont(key);
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
  const DataMap xAODElectronRetriever::getData(const xAOD::ElectronContainer* elCont) {

    ATH_MSG_DEBUG("in getData()");

    DataMap DataMap;

    DataVect pt; pt.reserve(elCont->size());
    DataVect phi; phi.reserve(elCont->size());
    DataVect eta; eta.reserve(elCont->size());
    DataVect mass; mass.reserve(elCont->size());
    DataVect energy; energy.reserve(elCont->size());
    DataVect pdgId; energy.reserve(elCont->size());

    DataVect isEMString; isEMString.reserve(elCont->size());
    DataVect author; author.reserve(elCont->size());
    DataVect label; label.reserve(elCont->size());

    xAOD::ElectronContainer::const_iterator elItr  = elCont->begin();
    xAOD::ElectronContainer::const_iterator elItrE = elCont->end();

    int counter = 0;
    auto prefix = [](const std::string & root)->std::string {
      return '_' + root;
    };
    for (; elItr != elItrE; ++elItr) {

      std::string electronAuthor = "";
      std::string electronIsEMString = "none";
      std::string electronLabel = "";
      phi.emplace_back(DataType((*elItr)->phi()));
      eta.emplace_back(DataType((*elItr)->eta()));
      pt.emplace_back(DataType((*elItr)->pt()/GeV));
      mass.emplace_back(DataType((*elItr)->m()/GeV));
      energy.emplace_back( DataType((*elItr)->e()/GeV ));

      if ((*elItr)->trackParticle()){ // ForwardElectrons have no track !
	pdgId.emplace_back(DataType( -11.*(*elItr)->trackParticle()->charge() )); // pdgId not available anymore in xAOD
      }else{
	pdgId.emplace_back(DataType( 0. ) );
      }

      ATH_MSG_DEBUG("  Electron #" << counter++ << " : eta = "  << (*elItr)->eta() << ", phi = "
		    << (*elItr)->phi()
		    // << ", ntrk = " << (*elItr)->getNumberOfTrackParticles()
		    << ", author = " << (*elItr)->author()
		    // these don't work for ForwardElectrons !
		    // << ", isEM/Tight: " << (*elItr)->passSelection(passesTight, "Tight")
		    // << ", charge = " << (*elItr)->trackParticle()->charge()
		    // << ", pdgId = " << -11.*(*elItr)->trackParticle()->charge()
		    );


      bool passesTight(false);
      bool passesMedium(false);
      bool passesLoose(false);
      static const std::string tightStr{"Tight"};
      const bool tightSelectionExists = (*elItr)->passSelection(passesTight, tightStr);
      ATH_MSG_VERBOSE("tight exists " << tightSelectionExists
		      << " and passes? " << passesTight);
		  static const std::string mediumStr{"Medium"};
      const bool mediumSelectionExists = (*elItr)->passSelection(passesMedium, mediumStr);
      ATH_MSG_VERBOSE("medium exists " << mediumSelectionExists
		      << " and passes? " << passesMedium);
		  static const std::string looseStr{"Loose"};
      const bool looseSelectionExists = (*elItr)->passSelection(passesLoose, looseStr);
      ATH_MSG_VERBOSE("loose exists " << looseSelectionExists
		      << " and passes? " << passesLoose);

      electronAuthor = "author"+DataType( (*elItr)->author() ).toString(); // for odd ones eg FWD
      electronLabel = electronAuthor;
      if (( (*elItr)->author()) == 0){ electronAuthor = "unknown"; electronLabel += "_unknown"; }
      if (( (*elItr)->author()) == 8){ electronAuthor = "forward"; electronLabel += "_forward"; }
      if (( (*elItr)->author()) == 2){ electronAuthor = "softe"; electronLabel += "_softe"; }
      if (( (*elItr)->author()) == 1){ electronAuthor = "egamma"; electronLabel += "_egamma"; }

      if ( passesLoose ){
	electronLabel += prefix(looseStr);
	electronIsEMString = looseStr; // assume that hierarchy is obeyed !
      }
      if ( passesMedium ){
	electronLabel += prefix(mediumStr);
	electronIsEMString = mediumStr; // assume that hierarchy is obeyed !
      }
      if ( passesTight ){
	electronLabel += prefix(tightStr);
	electronIsEMString = tightStr; // assume that hierarchy is obeyed !
      }
      author.emplace_back( DataType( electronAuthor ) );
      label.emplace_back( DataType( electronLabel ) );
      isEMString.emplace_back( DataType( electronIsEMString ) );

    } // end ElectronIterator

    // four-vectors
    std::size_t nEntries = phi.size();
    DataMap["phi"] = std::move(phi);
    DataMap["eta"] = std::move(eta);
    DataMap["pt"] = std::move(pt);
    DataMap["energy"] = std::move(energy);
    DataMap["mass"] = std::move(mass);
    DataMap["pdgId"] = std::move(pdgId);
    DataMap["isEMString"] = std::move(isEMString);
    DataMap["label"] = std::move(label);
    DataMap["author"] = std::move(author);

    ATH_MSG_DEBUG(dataTypeName() << " retrieved with " << nEntries << " entries");

    return DataMap;
  }


} // JiveXML namespace
