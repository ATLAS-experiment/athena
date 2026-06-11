/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthTrackRetriever.h"

#include "HepPDT/ParticleData.hh"
#include "HepPDT/ParticleDataTable.hh"
#include "GaudiKernel/SystemOfUnits.h" 
#include "EventPrimitives/EventPrimitives.h"

#include "GeneratorObjects/McEventCollection.h"
#include "TrackRecord/TrackRecord.h"
#include "TrackRecord/TrackRecordCollection.h"
#include <cmath>

namespace JiveXML {


  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  TruthTrackRetriever::TruthTrackRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    base_class(type,name,parent),
    m_typeName("STr")
  {
    declareProperty("StoreGateKey", m_McEvtCollName = "TruthEvent", "Name of the McEventCollection");
    declareProperty("UnstableMinPtCut",     m_MinPtCut = 100*Gaudi::Units::MeV, "Minimum pT for an unstable particle to get accepted");
    declareProperty("UnstableMinRhoCut",    m_MinRhoCut = 40*Gaudi::Units::mm, "Minium radius of the end-vertex for unstable particle to get accepted");

  }
  
  /**
   * Initialize before event loop
   */
  StatusCode TruthTrackRetriever::initialize(){
    //Nothing to do here
    return StatusCode::SUCCESS;
  }

  /**
   * Loop over all true particles and get their basic parameters
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode TruthTrackRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    //be verbose
    ATH_MSG_DEBUG( "Retrieving " << dataTypeName() ); 

    //Retrieve the collection
    const McEventCollection* McEvtColl = NULL;
    if ( !evtStore()->contains<McEventCollection>( m_McEvtCollName )){ 
      ATH_MSG_DEBUG( "Could not find McEventCollection " << m_McEvtCollName );
      return StatusCode::SUCCESS;
    }
    if( evtStore()->retrieve(McEvtColl, m_McEvtCollName).isFailure() ){
      ATH_MSG_DEBUG( "Could not retrieve McEventCollection " << m_McEvtCollName );
      return StatusCode::SUCCESS;
    }
    
    // Calculate the size
    long NParticles=0;
    McEventCollection::const_iterator McEvtCollItr = McEvtColl->begin(); 
    for ( ; McEvtCollItr != McEvtColl->end(); ++McEvtCollItr)
      NParticles +=  (*McEvtCollItr)->particles_size();

    //Show in verbose mode
    ATH_MSG_DEBUG(  "Total number of particles in McEventCollection \""  << m_McEvtCollName << "\" is " << NParticles );

    //Reserve space for the output data
    DataVect pt; pt.reserve(NParticles);
    DataVect phi; phi.reserve(NParticles);
    DataVect eta; eta.reserve(NParticles);
    DataVect rhoVertex; rhoVertex.reserve(NParticles);
    DataVect phiVertex; phiVertex.reserve(NParticles);
    DataVect zVertex; zVertex.reserve(NParticles);
    DataVect code; code.reserve(NParticles);
    DataVect id; id.reserve(NParticles);
    DataVect rhoEndVertex; rhoEndVertex.reserve(NParticles);
    DataVect phiEndVertex; phiEndVertex.reserve(NParticles);
    DataVect zEndVertex; zEndVertex.reserve(NParticles);

      
    //Now loop events and retrieve
    for ( McEvtCollItr = McEvtColl->begin(); McEvtCollItr != McEvtColl->end(); ++McEvtCollItr){

      //Loop over particles in the event
#ifdef HEPMC3
      const auto &barcodes = (*McEvtCollItr)->attribute<HepMC::GenEventBarcodes> (HepMCStr::barcodes);
      std::map<int,int> id_to_barcode_map;
      if (barcodes) id_to_barcode_map = barcodes->id_to_barcode_map();
#endif      
      for (const auto& particle:  *(*McEvtCollItr) ) {
        
        //Additional cuts for decaying particles
        if ( particle->end_vertex() ) {
          //Reject particles that fail the pt cut
          if ( particle->momentum().perp() < m_MinPtCut) continue ; 
          //Reject particles that fail the minimum end-vertex cut
          if (particle->end_vertex()->position().perp() < m_MinRhoCut ) continue ;
        }

        //Get basic parameters (eta, phi, pt, ...)
        pt.emplace_back(particle->momentum().perp()/Gaudi::Units::GeV);
        float thePhi = particle->momentum().phi();
        phi.emplace_back( (thePhi<0) ? thePhi+=2*M_PI : thePhi );
        eta.emplace_back( particle->momentum().pseudoRapidity() );
        code.emplace_back( particle->pdg_id() );
#ifdef HEPMC3
        id.emplace_back( id_to_barcode_map.at(particle->id() ));
#else
        id.emplace_back( HepMC::barcode(*particle) );
#endif

        // Get the vertex information
        const auto& vertexprod =  particle->production_vertex();
        if (vertexprod) {
          const auto& pos=vertexprod->position();
          rhoVertex.emplace_back( std::sqrt(pos.x()*pos.x()+pos.y()*pos.y()+pos.z()*pos.z())*Gaudi::Units::mm/Gaudi::Units::cm );
          float vtxPhi = pos.phi();
          phiVertex.emplace_back( (vtxPhi<0)? vtxPhi+=2*M_PI : vtxPhi );
          zVertex.emplace_back( pos.z()*Gaudi::Units::mm/Gaudi::Units::cm ); 
        } else {
          rhoVertex.emplace_back( 0. );
          phiVertex.emplace_back( 0. );
          zVertex.emplace_back( 0. ); 
        }
        //Do the same for the end vertex
        const auto& vertexend =  particle->end_vertex();
        if ( vertexend ) {
         const auto& pos=vertexend->position();
         rhoEndVertex.emplace_back(std::sqrt(pos.x()*pos.x()+pos.y()*pos.y()+pos.z()*pos.z())*Gaudi::Units::mm/Gaudi::Units::cm);
         float vtxPhi = pos.phi();
         phiEndVertex.emplace_back( (vtxPhi<0)? vtxPhi+=2*M_PI : vtxPhi );
         zEndVertex.emplace_back(pos.z()*Gaudi::Units::mm/Gaudi::Units::cm); 
        } else {
         rhoEndVertex.emplace_back( 0. );
         phiEndVertex.emplace_back( 0. );
         zEndVertex.emplace_back( 0. ); 
        }
      }
    }

    DataMap myDataMap;
    const auto nEntries = pt.size();
    myDataMap["pt"] = std::move(pt);
    myDataMap["phi"] = std::move(phi);
    myDataMap["eta"] = std::move(eta);
    myDataMap["code"] = std::move(code);
    myDataMap["id"] = std::move(id);
    myDataMap["rhoVertex"] = std::move(rhoVertex);
    myDataMap["phiVertex"] = std::move(phiVertex);
    myDataMap["zVertex"] = std::move(zVertex);
    myDataMap["rhoEndVertex"] = std::move(rhoEndVertex);
    myDataMap["phiEndVertex"] = std::move(phiEndVertex);
    myDataMap["zEndVertex"] = std::move(zEndVertex);

    //Be verbose
    ATH_MSG_DEBUG( dataTypeName() << ": "<< nEntries );

    //forward data to formating tool
    return FormatTool->AddToEvent(dataTypeName(), m_McEvtCollName, &myDataMap);
  }
}
