/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef NSWOCCUPANCYALG_H
#define NSWOCCUPANCYALG_H
#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <StoreGate/ReadHandleKey.h>
#include <MuonPattern/MuonPatternCombinationCollection.h>
#include <TH1.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

class NswOccupancyAlg : public AthHistogramAlgorithm {

public:
    NswOccupancyAlg(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) override;
    unsigned int cardinality() const override final { return 1; }

private:    
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{ this, "MuonIdHelperSvc",
                                        "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
    using Layer = std::vector<Identifier>;
    using LayerVec = std::vector<Layer>;
    LayerVec sortByLayer(std::vector<Identifier>& micromegaHits) const;

    void fillHistograms(const Layer& layers);


    
    SG::ReadHandleKey<MuonPatternCombinationCollection> m_patternCollKey{
        this,
        "MuonLayerHoughCombisKey",
        "MuonLayerHoughCombis",
        "Hough combinations",
    };

    Gaudi::Property<std::string> m_fileStream{this, "FileStream", "NSWSTORIES"};
    Gaudi::Property<unsigned int> m_binWidth{this, "BinWidth", 100};

    TH1* m_singleBinOccupancy{nullptr};
    TH1* m_pairBinOccupancy{nullptr};
    TH1* m_pairVsSingleOccupancy{nullptr};


};
#endif
