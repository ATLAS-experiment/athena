/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * File: TrackTools.h
 * Author: Marco van Woerden <mvanwoer@cern.ch>
 * Description: Track tools.
 *
 * Created in February 2013, based on TrackInCaloTools.
 */

#ifndef TrackTools_H
#define TrackTools_H
#include "ITrackTools.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"

#include <memory>

class TileID;
class TileDetDescrManager;

namespace TileCal {

//==========================================================================================================================
class TrackTools: public extends<AthAlgTool, ITrackTools> {
//==========================================================================================================================
  public:
    using base_class::base_class;

    StatusCode initialize() override final;
    StatusCode finalize() override final;

    // METHOD FOR CALOCELL FILTERING
    void getCellsWithinConeAroundTrack(const xAOD::TrackParticle* track,
                                       const CaloCellContainer* input,
                                       ConstDataVector<CaloCellContainer>* output,
                                       double cone,
                                       bool includelar) const override;

    std::vector< double > getXYZEtaPhiInCellSampling(const TRACK* track, const CaloCell *cell) const override;
    std::vector< double > getXYZEtaPhiInCellSampling(const TRACK* track, CaloSampling::CaloSample sampling) const override;
    std::unique_ptr<const Trk::TrackParameters> getTrackInCellSampling(const TRACK* track, CaloSampling::CaloSample sampling) const override;
    std::vector< std::vector<double> > getXYZEtaPhiPerLayer(const TRACK* track) const override;
    std::vector< std::vector<double> > getXYZEtaPhiPerSampling(const TRACK* track) const override;
    double getPathInsideCell(const TRACK *track, const CaloCell *cell) const override;
    double getPath(const CaloCell* cell, const Trk::TrackParameters *entrance, const Trk::TrackParameters *exit) const override;
    int retrieveIndex(int sampling, float eta) const override;

    std::vector<float> getEnergyInCones(const xAOD::TrackParticle* track,
                                        const xAOD::CaloClusterContainer* clusters,
                                        const std::set<xAOD::CaloCluster::CaloSample>& samplings,
                                        const std::vector<double>& drCones,
                                        const EventContext& ctx) const override;

  private:

    Gaudi::Property<bool> m_isCollision{this, "IsCollision", true};

    ToolHandle <Trk::IParticleCaloExtensionTool> m_caloExtensionTool{this,
       "ParticleCaloExtensionTool", "Trk::ParticleCaloExtensionTool/ParticleCaloExtensionTool"};

    const TileID* m_tileID{nullptr};
    const TileDetDescrManager* m_tileMgr{nullptr};
    enum TILE_RAW{TILE_RAW_FIRST, TILE_RAW_SECOND, TILE_RAW_THIRD, TILE_RAW_FOURTH, TILE_RAW_FIFTH, TILE_RAW_SIXTH};
};

} // TileCal namespace
#endif //TrackTools_H
