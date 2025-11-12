///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// MbtsToVectorsTool.cxx
// Implementation file for class MbtsToVectorsTool
///////////////////////////////////////////////////////////////////

// Tile includes
#include "MbtsToVectorsTool.h"

#include "TileEvent/TileCell.h"
#include "TileEvent/TileContainer.h"
#include "TileIdentifier/TileTBID.h"
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <vector>

namespace DerivationFramework {

  StatusCode MbtsToVectorsTool::initialize() {

    ATH_CHECK( m_cellContainerKey.initialize() );
    ATH_CHECK( m_energyKey.initialize() );
    ATH_CHECK( m_timeKey.initialize() );
    ATH_CHECK( m_qualityKey.initialize() );
    ATH_CHECK( m_typeKey.initialize() );
    ATH_CHECK( m_moduleKey.initialize() );
    ATH_CHECK( m_channelKey.initialize() );
    ATH_CHECK( m_etaKey.initialize(m_saveEtaPhi) );
    ATH_CHECK( m_phiKey.initialize(m_saveEtaPhi) );

    ATH_CHECK( detStore()->retrieve (m_tileTBID) );

    return StatusCode::SUCCESS;
  }


  StatusCode MbtsToVectorsTool::addBranches(const EventContext& ctx) const {


    SG::WriteHandle<std::vector<float> > energy(m_energyKey, ctx);
    ATH_CHECK( energy.record(std::make_unique<std::vector<float> >()) );
    energy->reserve(MAX_MBTS_COUNTER);

    SG::WriteHandle<std::vector<float> > time(m_timeKey, ctx);
    ATH_CHECK( time.record(std::make_unique<std::vector<float> >()) );
    time->reserve(MAX_MBTS_COUNTER);

    SG::WriteHandle<std::vector<int> > quality(m_qualityKey, ctx);
    ATH_CHECK( quality.record(std::make_unique<std::vector<int> >()) );
    quality->reserve(MAX_MBTS_COUNTER);

    SG::WriteHandle<std::vector<int> > module(m_moduleKey, ctx);
    ATH_CHECK( module.record(std::make_unique<std::vector<int> >()) );
    module->reserve(MAX_MBTS_COUNTER);

    SG::WriteHandle<std::vector<int> > type(m_typeKey, ctx);
    ATH_CHECK( type.record(std::make_unique<std::vector<int> >()) );
    type->reserve(MAX_MBTS_COUNTER);

    SG::WriteHandle<std::vector<int> > channel(m_channelKey, ctx);
    ATH_CHECK( channel.record(std::make_unique<std::vector<int> >()) );
    channel->reserve(MAX_MBTS_COUNTER);

    SG::ReadHandle<TileCellContainer> tileCells(m_cellContainerKey, ctx);
    ATH_CHECK( tileCells.isValid() );

    if (m_saveEtaPhi) {
      SG::WriteHandle<std::vector<float> > eta(m_etaKey, ctx);
      ATH_CHECK( eta.record(std::make_unique<std::vector<float> >()) );
      eta->reserve(MAX_MBTS_COUNTER);

      SG::WriteHandle<std::vector<float> > phi(m_phiKey, ctx);
      ATH_CHECK( phi.record(std::make_unique<std::vector<float> >()) );
      phi->reserve(MAX_MBTS_COUNTER);

      for ( const TileCell* cell : *tileCells ){
        energy->push_back( cell->energy() );
        eta->push_back( cell->eta() );
        phi->push_back( cell->phi() );
        time->push_back( cell->time() );
        quality->push_back(cell->quality() );
        module->push_back(m_tileTBID->module( cell->ID() ) );
        type->push_back(m_tileTBID->type( cell->ID() ) );
        channel->push_back(m_tileTBID->channel( cell->ID() ) );
      }

    } else {

      for ( const TileCell* cell : *tileCells ){
        energy->push_back( cell->energy() );
        time->push_back( cell->time() );
        quality->push_back(cell->quality() );
        module->push_back(m_tileTBID->module( cell->ID() ) );
        type->push_back(m_tileTBID->type( cell->ID() ) );
        channel->push_back(m_tileTBID->channel( cell->ID() ) );
      }

    }

    return StatusCode::SUCCESS;
  }

}
