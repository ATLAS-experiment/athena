/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEGEOG4CALIB_TILEHITVECTORDMBUILDER_H
#define TILEGEOG4CALIB_TILEHITVECTORDMBUILDER_H

#include <memory>
#include <utility>
#include "TileGeoG4SD//TileHitVectorBuilder.hh"
#include "TileGeoG4DMLookupBuilder.h"

/**
 * See TileGeoG4SD/TileHitVectorBuilder documentation.
 * 
 * Because TileGeoG4CalibSDTool doesn't have the required data to construct a TileGeoG4DMLookupBuilder,
 * it must be set by the SD itself at the beginning of a G4Event. 
*/
class TileHitVectorDMBuilder : public TileHitVectorBuilder
{
  public:
    using TileHitVectorBuilder::TileHitVectorBuilder;

    void ResetCells() { 
      // Call ResetCells for the parent
      TileHitVectorBuilder::ResetCells();
      m_dmLookupBuilder->ResetCells(); 
    };

    TileGeoG4DMLookupBuilder* GetDMLookupBuilder() const { return m_dmLookupBuilder.get(); }

    void SetDMLookupBuilder(std::unique_ptr<TileGeoG4DMLookupBuilder> dmLookupBuilder) { 
      m_dmLookupBuilder = std::move(dmLookupBuilder);
    }
  
  private:
    std::unique_ptr<TileGeoG4DMLookupBuilder> m_dmLookupBuilder;
};

#endif