/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "NswOccupancyAlg.h"
#include <StoreGate/ReadHandle.h>

#include <TH1I.h>
#include <TH2I.h>
#include <set>

NswOccupancyAlg::NswOccupancyAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthHistogramAlgorithm(name, pSvcLocator){}

StatusCode NswOccupancyAlg::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_patternCollKey.initialize());
    std::stringstream selection_title{};
    selection_title<<"Bin size "<<m_binWidth.value()<<";";
    selection_title<<"Hit occupancy;";
    m_singleBinOccupancy = new TH1I("SingleBinOccupancy", selection_title.str().c_str(), 50, 0, 50);
    m_pairBinOccupancy = new TH1I("PairBinOccupancy", selection_title.str().c_str(), 50, 0, 50);
    selection_title<<"; Pair occupancy";
    m_pairVsSingleOccupancy = new TH2I("PairVsSingleOccupancy",selection_title.str().c_str(), 50,0,50,50,0,50);
    std::stringstream folderName{};
    folderName<<"/"<<m_fileStream.value()<<"/OccupancyBinWidth"<<m_binWidth.value()<<"/";
    ATH_MSG_INFO("Write out histograms in "<<folderName.str());
    ATH_CHECK(histSvc()->regHist(folderName.str() + m_singleBinOccupancy->GetName(), m_singleBinOccupancy));
    ATH_CHECK(histSvc()->regHist(folderName.str() + m_pairBinOccupancy->GetName(), m_pairBinOccupancy));
    ATH_CHECK(histSvc()->regHist(folderName.str() + m_pairVsSingleOccupancy->GetName(), m_pairVsSingleOccupancy));
    
    return StatusCode::SUCCESS;
}
StatusCode NswOccupancyAlg::execute(const EventContext& ctx) { 
    SG::ReadHandle<MuonPatternCombinationCollection> pattCol{m_patternCollKey, ctx};
    if (!pattCol.isValid()){
        ATH_MSG_FATAL("Failed to retrieve the pattern collection "<<m_patternCollKey.fullKey());
        return StatusCode::FAILURE;
    }
       
    for (const Muon::MuonPatternCombination* patComb : *pattCol ) {
         /// Collect all the hit identifiers
        std::map<unsigned int, Layer> sectorsHits{};
        for (const Muon::MuonPatternChamberIntersect&  it :patComb->chamberData()) {
            if (it.prepRawDataVec().empty()) continue;
            const Identifier id = it.prepRawDataVec().front()->identify();
            if (!m_idHelperSvc->isMM(id) && !m_idHelperSvc->issTgc(id)) continue;
            Layer& hitIds = sectorsHits[m_idHelperSvc->sector(id)];
            hitIds.reserve(it.prepRawDataVec().size() + hitIds.size());
            
            for (const Trk::PrepRawData* prd : it.prepRawDataVec()) {
                if (prd->type(Trk::PrepRawDataType::MMPrepData)) hitIds.push_back(prd->identify());
            }
        } 
        for (auto& [sector, hitIds] : sectorsHits ) {
            LayerVec sortedHits = sortByLayer(hitIds);
            for (Layer& lay : sortedHits) {
                fillHistograms(lay);   
            }
        }             
    }
    
    return StatusCode::SUCCESS;
}
std::vector<std::vector<Identifier>> NswOccupancyAlg::sortByLayer(Layer& micromegaHits) const{
    LayerVec layerVec;
    std::stable_sort(micromegaHits.begin(), micromegaHits.end(),[this](const Identifier& a, const Identifier& b){
        return m_idHelperSvc->gasGapId(a) < m_idHelperSvc->gasGapId(b);
    });
    std::set<Identifier> unique_set{};
    for (const Identifier& id : micromegaHits) {
        if (!unique_set.insert(id).second) continue;
        if (layerVec.empty() || m_idHelperSvc->gasGapId(id) != m_idHelperSvc->gasGapId(layerVec.back().back()) ){
            layerVec.emplace_back();
            layerVec.back().reserve(micromegaHits.size());
        }
        layerVec.back().push_back(id);
    }
    for (Layer& lay : layerVec) {
        std::sort(lay.begin(),lay.end(),[this] (const Identifier&a ,const Identifier& b){
            return m_idHelperSvc->mmIdHelper().channel(a) < m_idHelperSvc->mmIdHelper().channel(b);
        });
    }
    return layerVec;
}
void NswOccupancyAlg::fillHistograms(const Layer& clustInLay) {
    if (clustInLay.size() < 2) return;
    std::vector<unsigned int> histogram{};
    const unsigned int firstCh = m_idHelperSvc->mmIdHelper().channel(clustInLay[0]);
    const unsigned int lastCh  = m_idHelperSvc->mmIdHelper().channel(clustInLay[clustInLay.size() -1]);
    /// Define the number of bins
    const unsigned int deltaCh = lastCh - firstCh;
    const unsigned int nBins = deltaCh / m_binWidth + (deltaCh % m_binWidth > 0);
    if (!nBins) return;
    histogram.resize(nBins);
    for (const Identifier&id : clustInLay) {
        const int bin_num = (m_idHelperSvc->mmIdHelper().channel(id) - firstCh) %nBins;
        ++(histogram[bin_num]);
    }
    for (unsigned int occup : histogram){
       if (occup) m_singleBinOccupancy->Fill(occup);
    }
    for (unsigned int i = 0 ; i < histogram.size() -1 ; ++i){
        unsigned int pair = histogram[i]+ histogram[i+1];
        if (!pair) continue;
        m_pairBinOccupancy->Fill(pair);

        if (histogram[i]) m_pairVsSingleOccupancy->Fill(histogram[i], pair);
        if (histogram[i+1]) m_pairVsSingleOccupancy->Fill(histogram[i+1], pair);
        
    }
    
}

