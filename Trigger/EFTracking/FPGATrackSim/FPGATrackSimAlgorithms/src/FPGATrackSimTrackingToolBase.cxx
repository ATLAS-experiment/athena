// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimTrackingToolBase.h"


FPGATrackSimTrackingToolBase::FPGATrackSimTrackingToolBase(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type, name, parent)
{
}

StatusCode FPGATrackSimTrackingToolBase::setRoadSectors(std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads)
{
  for (auto& road: roads)
  {
    std::shared_ptr<FPGATrackSimRoad> nonConstRoad = std::const_pointer_cast<FPGATrackSimRoad>(road);
    if (m_useSectors) {
      if(! m_do2ndStage)
        nonConstRoad->setSector(m_FPGATrackSimBank->SectorBank_1st()->findSector(nonConstRoad->getAllHits()));
      else
        nonConstRoad->setSector(m_FPGATrackSimBank->SectorBank_2nd()->findSector(nonConstRoad->getAllHits()));
    }
    else if (m_idealGeoRoads) matchIdealGeoSector(*nonConstRoad);
    std::cout << "Sector:"<< nonConstRoad->getSector() << std::endl;
  }
  // Spacepoint road filter tool. Needed when fitting to spacepoints.
  if (m_useSpacePoints)
  {
    std::vector<std::shared_ptr<const FPGATrackSimRoad>> postfilter_roads;
    ATH_CHECK(m_spRoadFilterTool->filterRoads(roads, postfilter_roads));
    roads = std::move(postfilter_roads);
  }
  return StatusCode::SUCCESS;
}



void FPGATrackSimTrackingToolBase::matchIdealGeoSector(FPGATrackSimRoad & r)
{
    // We now look up the binning information in the sector bank.
    const FPGATrackSimSectorBank* sectorbank = nullptr;
    if(! m_do2ndStage)
      sectorbank = m_FPGATrackSimBank->SectorBank_1st();
    else
      sectorbank = m_FPGATrackSimBank->SectorBank_2nd();

    // Look up q/pt (or |q/pt|) from the Hough road, convert to MeV.
    double qoverpt = r.getY()*0.001;
    if (sectorbank->isAbsQOverPtBinning()) {
       qoverpt = std::abs(qoverpt);
    }

    // Retrieve the bin boundaries from the sector bank; map this onto them.
    const std::vector<double> &qoverpt_bins = sectorbank->getQOverPtBins();
    auto bounds = std::equal_range(qoverpt_bins.begin(), qoverpt_bins.end(), qoverpt);

    // estimate sectorbin
    int sectorbin = fpgatracksim::QPT_SECTOR_OFFSET * (bounds.first - qoverpt_bins.begin() - 1);
    sectorbin = std::clamp(sectorbin, 0, 10 * static_cast<int>(qoverpt_bins.size() - 2));

    if (m_doRegionalMapping){
      int subregion = r.getSubRegion();
      sectorbin += subregion*fpgatracksim::SUBREGION_SECTOR_OFFSET;
    }

    std::vector<module_t> modules(r.getNLayers(), -1);
    layer_bitmask_t wc_layers = r.getWCLayers();
    for (unsigned int il = 0; il < r.getNLayers(); il++) {
        if (r.getNHits_layer()[il] == 0) {

            // set corresponding bit to 1 in case of wc in the current layer
            wc_layers |= (0x1 << il);

            std::unique_ptr<FPGATrackSimHit> wcHit = std::make_unique<FPGATrackSimHit>();
            wcHit->setHitType(HitType::wildcard);
            wcHit->setLayer(il);
            if(! m_do2ndStage)
              wcHit->setDetType(m_FPGATrackSimMapping->PlaneMap_1st(0)->getDetType(il));
            else
              wcHit->setDetType(m_FPGATrackSimMapping->PlaneMap_2nd()->getDetType(il));

            // Now store wc hit in a "std::vector<std::shared_ptr<const FPGATrackSimHit>>" format.
            // We can probably avoid initializing an intermediate variable wcHits as we used to do
            r.setHits(il,{std::move(wcHit)});
        }
        else {
            modules[il]= sectorbin;
        }
    }
    r.setWCLayers(wc_layers);


    // If we are using eta patterns. We need to first run the roads through the road filter.
    // Then the filter will be responsible for setting the actual sector.
    // As a hack, we can store the sector bin ID in the road for now.
    // This is fragile! If we want to store a different ID for each layer, it will break.

    // Similarly, we do the same thing for spacepoints. this probably means we can't combine the two.
    // maybe better to store the module array instead of just a number?
    
    r.setSectorBin(sectorbin);
    if (!m_doEtaPatternConsts && !m_useSpacePoints) {
        r.setSector(sectorbank->findSector(modules));
    }
}
