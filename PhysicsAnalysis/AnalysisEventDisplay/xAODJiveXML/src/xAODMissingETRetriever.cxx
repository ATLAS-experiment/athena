/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODMissingETRetriever.h"

#include "AthenaKernel/Units.h"
using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  xAODMissingETRetriever::xAODMissingETRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent){}


  StatusCode xAODMissingETRetriever::initialize(){
    ATH_CHECK(m_keys.initialize());
    return StatusCode::SUCCESS;
  }

  
  /**
   * For each MET collection retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODMissingETRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG( "in retrieve()" );

    // Loop through the keys and retrieve the corresponding data
    for (const auto& key : m_keys) {
      SG::ReadHandle<xAOD::MissingETContainer> cont(key);
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
  const DataMap xAODMissingETRetriever::getData(const xAOD::MissingETContainer* metCont) {

    ATH_MSG_DEBUG( "in getData()" );

    DataMap DataMap;

    DataVect etx; etx.reserve(metCont->size());
    DataVect ety; ety.reserve(metCont->size());
    DataVect et; et.reserve(metCont->size());

    float mpx = 0.;
    float mpy = 0.;
    float sumet = 0.;

    xAOD::MissingETContainer::const_iterator metItr  = metCont->begin();
    xAOD::MissingETContainer::const_iterator metItrE = metCont->end();

    // current understanding is that we only need the final value
    // out of the ~9 values within each MET container ('final')

    for (; metItr != metItrE; ++metItr) {
      sumet = (*metItr)->sumet()/GeV;
      mpx = (*metItr)->mpx()/GeV;
      mpy = (*metItr)->mpy()/GeV;

      ATH_MSG_DEBUG( "  Components: MissingET [GeV] mpx= "  << mpx
		     << ", mpy= " << mpy
		     << ", sumet= " << sumet );

    } // end MissingETIterator

    ATH_MSG_DEBUG( "  FINAL: MissingET [GeV] mpx= "  << mpx
		   << ", mpy= " << mpy << ", sumet= " << sumet );

    etx.emplace_back(DataType( mpx ));
    ety.emplace_back(DataType( mpy ));
    et.emplace_back(DataType( sumet ));

    // four-vectors
    const auto n = et.size();
    DataMap["et"] = std::move(et);
    DataMap["etx"] = std::move(etx);
    DataMap["ety"] = std::move(ety);

    ATH_MSG_DEBUG( dataTypeName() << " retrieved with " << n << " entries" );

    //All collections retrieved okay
    return DataMap;
  }

  
} // JiveXML namespace
