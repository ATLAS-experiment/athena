/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughPatternTools/MuonHoughPatternFinderTool.h"

#include <map>
#include <memory>
#include <set>

#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonHoughPatternEvent/MuonHoughHitContainer.h"
#include "MuonPrepRawData/MdtDriftCircleStatus.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRIO_OnTrack/CscClusterOnTrack.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonSegment/MuonSegment.h"
#include "MuonSegment/MuonSegmentCombination.h"  // for csc's
#include "TFile.h"
#include "TH1F.h"
#include "TrkDriftCircleMath/DriftCircle.h"
#include "TrkDriftCircleMath/MatchDCWithLine.h"
#include "TrkDriftCircleMath/TangentToCircles.h"
#include "TrkSurfaces/Surface.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
using namespace TrkDriftCircleMath;

namespace Muon {
    using MuonPatternHoughPair = MuonHoughPatternFinderTool::MuonPatternHoughPair;
    struct SegmentData {
        /// Index
        unsigned int index{0};
        const Muon::MdtPrepData* prd{nullptr};
        inline Identifier id() const { return prd->identify(); };  /// Identifier of the PRD
        inline double radius() const { return prd->localPosition()[0]; }
        inline double errradius() const { return Amg::error(prd->localCovariance(), 0); }
        double weights{1.};
        double prob{1.};  // artificial probability that hit belongs to true muon

        bool onsegment{false};  // non-zero if on segment, int indicates how many hits in same
                                // layer are on segment (used in weighting)
        double psi{0.};

        double weighted_trigger{0.};
        bool tr_confirmation{false};

        int layer_number{0};  // layer_number ranges from 0..5/7
    };

    MuonHoughPatternFinderTool::MuonHoughPatternFinderTool(const std::string& t, const std::string& n, const IInterface* p) :
        AthAlgTool(t, n, p) {
        declareInterface<IMuonHoughPatternFinderTool>(this);
    }
    MuonHoughPatternFinderTool::~MuonHoughPatternFinderTool() = default;
    StatusCode MuonHoughPatternFinderTool::initialize() {
        if (m_use_histos) {
            if (Gaudi::Concurrency::ConcurrencyFlags::numThreads() > 1) {
                ATH_MSG_FATAL("Filling histograms not supported in MT jobs.");
                return StatusCode::FAILURE;
            }

            m_h = std::make_unique<Hists>();
            m_h->m_file = std::make_unique<TFile>("Hough_histos.root", "RECREATE");
            m_h->m_weighthistogram = std::make_unique<TH1F>("weighthisto", "weighthisto", 100, -0.5, 2);
            m_h->m_weighthistogrammdt = std::make_unique<TH1F>("weighthistomdt", "weighthistomdt", 100, -0.3, 2.2);
            m_h->m_weighthistogramrpc = std::make_unique<TH1F>("weighthistorpc", "weighthistorpc", 100, -0.3, 2.2);
            m_h->m_weighthistogramcsc = std::make_unique<TH1F>("weighthistocsc", "weighthistocsc", 100, -0.3, 2.2);
            m_h->m_weighthistogramtgc = std::make_unique<TH1F>("weighthistotgc", "weighthistotgc", 100, -0.3, 2.2);
            m_h->m_weighthistogramstgc = std::make_unique<TH1F>("weighthistostgc", "weighthistostgc", 100, -0.3, 2.2);
            m_h->m_weighthistogrammm = std::make_unique<TH1F>("weighthistomm", "weighthistomm", 100, -0.3, 2.2);        
        }

        ATH_MSG_VERBOSE("MuonHoughPatternFinderTool::Initializing");
        ATH_CHECK(m_muonCombinePatternTool.retrieve());
        ATH_MSG_VERBOSE("found Service MuonCombinePatternTool " << m_muonCombinePatternTool);

        ATH_CHECK(m_muonHoughPatternTool.retrieve());
        ATH_MSG_VERBOSE("found Service muonHoughPatternTool: " << m_muonHoughPatternTool);

        ATH_CHECK(m_idHelperSvc.retrieve());
        
        const RpcIdHelper& rpcHelper{m_idHelperSvc->rpcIdHelper()};
        const MdtIdHelper& mdtIdHelper{m_idHelperSvc->mdtIdHelper()};
        m_RpcToMdtOuterStDict[rpcHelper.stationNameIndex("BOL")] = mdtIdHelper.stationNameIndex("BOS");
        m_RpcToMdtOuterStDict[rpcHelper.stationNameIndex("BOS")] = mdtIdHelper.stationNameIndex("BOL");
        m_RpcToMdtOuterStDict[rpcHelper.stationNameIndex("BMS")] = mdtIdHelper.stationNameIndex("BML");
        m_RpcToMdtOuterStDict[rpcHelper.stationNameIndex("BML")] = mdtIdHelper.stationNameIndex("BMS");

        m_RpcToMdtInnerStDict[rpcHelper.stationNameIndex("BMS")] = mdtIdHelper.stationNameIndex("BIS");
        m_RpcToMdtInnerStDict[rpcHelper.stationNameIndex("BML")] = mdtIdHelper.stationNameIndex("BIL");

        ATH_CHECK(m_printer.retrieve());

        if (m_hit_reweights) { ATH_MSG_DEBUG("Hit Reweighting " << m_hit_reweights); }

        ATH_CHECK(m_CosmicPhiPatternsKey.initialize(m_recordAllOutput));
        ATH_CHECK(m_CosmicEtaPatternsKey.initialize(m_recordAllOutput));
        ATH_CHECK(m_COMBINED_PATTERNSKey.initialize(m_recordAllOutput));

        ATH_MSG_VERBOSE("End of Initializing");
        return StatusCode::SUCCESS;
    }

    template <class T> std::vector<const MuonPrepDataCollection<T>*> MuonHoughPatternFinderTool::stdVec(const MuonPrepDataContainerT<T>* cont) const {
        if (!cont) return {};
        std::vector<const MuonPrepDataCollection<T>*> vec;
        vec.reserve(cont->size());
        std::transform(cont->begin(), cont->end(), std::back_inserter(vec), 
                       [](const MuonPrepDataCollection<T>* ptr){return ptr;});
        return vec;
    }
    MuonPatternHoughPair MuonHoughPatternFinderTool::find(
        const MdtPrepDataContainer* mdtCont, const CscPrepDataContainer* cscCont, const TgcPrepDataContainer* tgcCont,
        const RpcPrepDataContainer* rpcCont, const sTgcPrepDataContainer* stgcCont, const MMPrepDataContainer* mmCont,
        const EventContext& ctx) const {        
        return find(ctx, stdVec(mdtCont), stdVec(cscCont),  stdVec(tgcCont), 
                    stdVec(rpcCont), stdVec(stgcCont), stdVec(mmCont), nullptr);
    }
    MuonPatternHoughPair MuonHoughPatternFinderTool::find(const std::vector<const MdtPrepDataCollection*>& mdtCols,
                                     const std::vector<const CscPrepDataCollection*>& cscCols,
                                     const std::vector<const TgcPrepDataCollection*>& tgcCols,
                                     const std::vector<const RpcPrepDataCollection*>& rpcCols,
                                     const MuonSegmentCombinationCollection* cscSegmentCombis, const EventContext& ctx) const {
        return find(ctx, mdtCols, cscCols, tgcCols, rpcCols, {}, {}, cscSegmentCombis);    
    }
   MuonPatternHoughPair MuonHoughPatternFinderTool::find(const EventContext& ctx, 
                                                        const std::vector<const MdtPrepDataCollection*>& mdtCols, 
                                                        const std::vector<const CscPrepDataCollection*>& cscCols,
                                                        const std::vector<const TgcPrepDataCollection*>& tgcCols, 
                                                        const std::vector<const RpcPrepDataCollection*>& rpcCols,
                                                        const std::vector<const sTgcPrepDataCollection*>& stgcCols,
                                                        const std::vector<const MMPrepDataCollection*>& mmCols,
                                                        const MuonSegmentCombinationCollection* cscSegmentCombis) const{
        /** map between mdt chamber identifiers and corresponding rpc hits
         * (hit_no_begin and hit_no_end)*/
        std::map<int, std::vector<std::pair<int, int>>> rpcmdtstationmap;
        /** map between mdt chamber identifiers and corresponding tgc hits
         * (hit_no_begin and hit_no_end)*/
        std::map<int, std::vector<std::pair<int, int>>> tgcmdtstationmap;

        /** map for association between trigger eta hits (first) and phi hits (second)
         * within the same gasgap, used for combining patterns in
         * MuonCombinePatternTool */
        EtaPhiHitAssocMap phietahitassociation{};

        // read event_data:
        std::unique_ptr<MuonHoughHitContainer> hitcontainer{
            getAllHits(mdtCols, tgcCols, rpcCols, cscSegmentCombis, rpcmdtstationmap, tgcmdtstationmap, phietahitassociation)};
        
        if (m_use_csc && !cscSegmentCombis) {
            addCollections(cscCols, *hitcontainer, phietahitassociation);
        }
        if (m_use_mm) {
            addCollections(mmCols, *hitcontainer, phietahitassociation); 
        }
        if (m_use_stgc) {
            addCollections(stgcCols, *hitcontainer, phietahitassociation);
        }
        // analyse data
        std::unique_ptr<MuonPatternCombinationCollection> patCombiCol = analyse(ctx, *hitcontainer, 
                                                                                phietahitassociation);
        

        // ensure we always output a collection
        if (!patCombiCol) {
            ATH_MSG_DEBUG(" NO pattern combinations found, creating empty collection ");
            patCombiCol = std::make_unique<MuonPatternCombinationCollection>();
        }

        if (m_summary || msgLvl(MSG::DEBUG)) {
            ATH_MSG_INFO(" summarizing Combined pattern combination output: " <<std::endl << m_printer->print(*patCombiCol));
        }
    
        ATH_MSG_VERBOSE("execute(end) ");
        // return result
        return {std::move(patCombiCol), nullptr};
    }

    StatusCode MuonHoughPatternFinderTool::finalize() {
        if (m_use_histos) {
            auto save_histo = [this](std::unique_ptr<TH1>& h_ptr) {
                if (!h_ptr) return;
                m_h->m_file->WriteObject(h_ptr.get(), h_ptr->GetName());
                h_ptr.reset();
            };
            save_histo(m_h->m_weighthistogram);
            save_histo(m_h->m_weighthistogrammdt);
            save_histo(m_h->m_weighthistogramrpc);
            save_histo(m_h->m_weighthistogramcsc);
            save_histo(m_h->m_weighthistogramtgc);
            save_histo(m_h->m_weighthistogramstgc);
            save_histo(m_h->m_weighthistogrammm);
            
        }
        ATH_MSG_VERBOSE("finalize()");

        return StatusCode::SUCCESS;
    }

    std::unique_ptr<MuonPatternCombinationCollection> MuonHoughPatternFinderTool::analyse(const EventContext& ctx,
        const MuonHoughHitContainer& hitcontainer,
        const EtaPhiHitAssocMap& phietahitassociation) const {
        ATH_MSG_DEBUG("size of event: " << hitcontainer.size());

        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream sstr{};
            for (size_t h = 0; h < hitcontainer.size() ; ++h){
                const auto& houghHit = hitcontainer.getHit(h);
                sstr<<m_printer->print(*houghHit->getPrd())<<" weight: "<<houghHit->getWeight()<<std::endl;    
            }
            ATH_MSG_VERBOSE("Dump Hough container "<<std::endl<<sstr.str());
        }
       
        
        /** reconstructed patterns stored per [number_of_ids][level][which_segment] */
        MuonHoughPatternContainerShip houghpattern = m_muonHoughPatternTool->emptyHoughPattern();
        //  pass through hitcontainer (better still: preprawdata and only after make
        //  internal hitcontainer)
       
        m_muonHoughPatternTool->makePatterns(hitcontainer, houghpattern);
        
        std::unique_ptr<MuonPrdPatternCollection> phipatterns{m_muonHoughPatternTool->getPhiMuonPatterns(houghpattern)};        
        std::unique_ptr<MuonPrdPatternCollection> etapatterns{m_muonHoughPatternTool->getEtaMuonPatterns(houghpattern)};

        if (m_summary || msgLvl(MSG::DEBUG)) {
            if (phipatterns->empty())
                ATH_MSG_INFO(" summarizing input: Phi pattern combination empty");
            else
                ATH_MSG_INFO(" summarizing Phi pattern combination input: " << std::endl << m_printer->print(*phipatterns));
            if (etapatterns->empty())
                ATH_MSG_INFO(" summarizing input: Eta pattern combination empty");
            else
                ATH_MSG_INFO(" summarizing Eta pattern combination input: " << std::endl << m_printer->print(*etapatterns));
        }

        ATH_MSG_DEBUG("writePatterns");
        ATH_MSG_DEBUG("size: phi: " << phipatterns->size() << " eta: " << etapatterns->size());

        std::unique_ptr<MuonPrdPatternCollection> combinedpatterns;
        std::unique_ptr<MuonPatternCombinationCollection> patterncombinations{};

        // make + write muonpatterncombinations
        if (!etapatterns->empty()) {
            combinedpatterns = m_muonCombinePatternTool->combineEtaPhiPatterns(*phipatterns, *etapatterns, phietahitassociation);
        }

        if (combinedpatterns) {
            patterncombinations = m_muonCombinePatternTool->makePatternCombinations(*combinedpatterns);
        } else {
            ATH_MSG_DEBUG("No combined patterns, creating dummy.");
            combinedpatterns = std::make_unique<MuonPrdPatternCollection>();
        }

        record(phipatterns, m_CosmicPhiPatternsKey, ctx);
        record(etapatterns, m_CosmicEtaPatternsKey, ctx);
        record(combinedpatterns, m_COMBINED_PATTERNSKey, ctx);

        return patterncombinations;
    }

    std::unique_ptr<MuonHoughHitContainer> MuonHoughPatternFinderTool::getAllHits(
        const std::vector<const MdtPrepDataCollection*>& mdtCols,
        const std::vector<const TgcPrepDataCollection*>& tgcCols, const std::vector<const RpcPrepDataCollection*>& rpcCols,
        const MuonSegmentCombinationCollection* cscSegmentCombis, std::map<int, std::vector<std::pair<int, int>>>& rpcmdtstationmap,
        std::map<int, std::vector<std::pair<int, int>>>& tgcmdtstationmap,
        EtaPhiHitAssocMap& phietahitassociation) const {
        ATH_MSG_VERBOSE("getAllHits()");

        std::unique_ptr<MuonHoughHitContainer> hitcontainer = std::make_unique<MuonHoughHitContainer>();
        // reserve space for 5000 hits (arbitrary), this should gain some cpu/memory
        // for background, but will lose for lower occupancy. If anyone knows a way to
        // predict the number of muon hits, I'd like to hear it.
        hitcontainer->reserve(5000);
        
        if (cscSegmentCombis && m_use_csc) {
            std::set<Identifier> csc_set;  // set to make sure every csc hit is only
                                           // passed to hitcontainer once
            std::pair<std::set<Identifier>::iterator, bool> csc_pair;
            std::map<int, int> nHitsPerLayer;  // map that connect layer number (1000*eta +
                                                          // 100*phi + 10*chamberlayer+ 2*wirelayer +
                                                          // eta/phi)

            std::vector<const Muon::CscClusterOnTrack*> csc_rots;  // csc rots          

            std::vector<int> layer_ids;  // if 0 then prd already added

            csc_rots.reserve(400);  // again arbitrary, atm (May 2008), the max number of
                                    // csc segments is 50 (times 8 hits = 400)
            layer_ids.reserve(400);

            // two loops needed as number of hits per layer needs to be known
            for (const Muon::MuonSegmentCombination* msc : *cscSegmentCombis) {
                ATH_MSG_VERBOSE("CSC combo segments loop, segmentcombo " << msc);
                for (unsigned int ss = 0; ss < msc->numberOfStations(); ++ss) {
                    for (const std::unique_ptr<MuonSegment>& ms : *msc->stationSegments(ss)) {
                        if (!ms) {
                            ATH_MSG_DEBUG("Segment has been already skimmed");
                            continue;
                        }
                        ATH_MSG_VERBOSE("CSC segments loop, segment: " << ms.get());
                        PrepDataSet phi_set;
                        std::vector<const Trk::PrepRawData*> eta_vector;                       

                        int nRoTs = ms->numberOfContainedROTs();
                        for (int i = 0; i < nRoTs; ++i) {
                            const Muon::CscClusterOnTrack* cscOnSeg = dynamic_cast<const Muon::CscClusterOnTrack*>(ms->rioOnTrack(i));
                            if (!cscOnSeg) {
                                ATH_MSG_INFO("Dynamic cast to CscClusterOnTrack failed!");
                                continue;
                            }
                            csc_rots.push_back(cscOnSeg);
                            Identifier id = cscOnSeg->identify();
                            bool channel_type = m_idHelperSvc->cscIdHelper().measuresPhi(id);
                            csc_pair = csc_set.insert(id);
                            if (!csc_pair.second) {
                                ATH_MSG_DEBUG(" CSC hit was already added, weight set to 0");
                                layer_ids.push_back(0);
                            } else {
                                const int layer_id = 1000 * m_idHelperSvc->cscIdHelper().stationEta(id) +
                                                     100 * m_idHelperSvc->cscIdHelper().stationPhi(id) +
                                                     10 * m_idHelperSvc->cscIdHelper().chamberLayer(id) +
                                                     2 * m_idHelperSvc->cscIdHelper().wireLayer(id) + channel_type;
                                ATH_MSG_DEBUG("csc layer_id: " << layer_id);
                                ++nHitsPerLayer[layer_id];
                                layer_ids.push_back(layer_id);
                            }

                            if (channel_type) {  // phi hit
                                if (!phi_set.insert(cscOnSeg->prepRawData()).second) { ATH_MSG_INFO(" CSC phi hit was already added"); }
                            } else {  // eta hit
                                eta_vector.push_back(cscOnSeg->prepRawData());
                            }
                        }  // rots
                        // add hit association from segment to map:
                        if (!phi_set.empty()) {
                            ATH_MSG_VERBOSE("Number of Phi Csc hits in segment: " << phi_set.size());
                            for (const Trk::PrepRawData* prd : eta_vector) { phietahitassociation.insert(std::make_pair(prd, phi_set)); }
                        }
                    }
                }
            }

            for (unsigned int i = 0; i < csc_rots.size(); i++) {
                const Muon::CscPrepData* prd = csc_rots[i]->prepRawData();

                const Amg::Vector3D& globalpos = csc_rots[i]->globalPosition();
                bool channel_type = m_idHelperSvc->cscIdHelper().measuresPhi(csc_rots[i]->identify());

                double weight = 0.;
                if (layer_ids[i] != 0) {  // not yet added
                    double number_of_hits = (double)nHitsPerLayer[layer_ids[i]];
                    weight = m_weight_csc_on_segment / (0.75 * std::sqrt(number_of_hits) + 0.25 * number_of_hits);
                }

                ATH_MSG_DEBUG(m_printer->print(*prd) << " weight " << weight);
                std::shared_ptr<MuonHoughHit> hit = std::make_shared<MuonHoughHit>(globalpos, channel_type, MuonHough::CSC, 1., weight, prd);

                hitcontainer->addHit(hit);
                if (m_use_histos) {
                    Hists& h = getHists();
                    h.m_weighthistogram->Fill(weight);
                    h.m_weighthistogramcsc->Fill(weight);
                }
            }
        }  // use_csc_segments
        // taken and modified from
        // DetectorDescription/GeoModel/HitDisplay/src/HitDisplaySystem.cxx

        if (m_use_rpc) {
            for (const RpcPrepDataCollection* rpc_coll : rpcCols) {
                addRpcCollection(rpc_coll, *hitcontainer, rpcmdtstationmap, phietahitassociation);
            }
        }

        if (m_use_tgc) {
            for (const TgcPrepDataCollection* tgc_coll : tgcCols) {
                addTgcCollection(tgc_coll, *hitcontainer, tgcmdtstationmap, phietahitassociation);
            }
        }

        if (m_use_mdt) {
            for (const MdtPrepDataCollection* prep_coll : mdtCols) {
                addMdtCollection(prep_coll, *hitcontainer, rpcmdtstationmap, tgcmdtstationmap);
            }
        }

       

        if (msgLevel(MSG::VERBOSE)) {
            ATH_MSG_VERBOSE("MuonHoughPatternFinderTool::getAllHits() saving " << hitcontainer->size() << " converted hits");
            for (unsigned int i = 0; i < hitcontainer->size(); i++) {
                ATH_MSG_VERBOSE(" hit " << hitcontainer->getHit(i)->getWhichDetector() << " (" << hitcontainer->getHit(i)->getHitx() << ","
                                        << hitcontainer->getHit(i)->getHity() << "," << hitcontainer->getHit(i)->getHitz() << ") "
                                        << " weight: " << hitcontainer->getHit(i)->getWeight()
                                        << " measures phi: " << hitcontainer->getHit(i)->getMeasuresPhi());
            }
        }

        ATH_MSG_VERBOSE("MuonHoughPatternFinderTool::getAllHits() saving " << phietahitassociation.size() << "associated hits ");
        return hitcontainer;

    }  // getAllHits

    void MuonHoughPatternFinderTool::record(std::unique_ptr<MuonPrdPatternCollection>& patCol,
                                            const SG::WriteHandleKey<MuonPrdPatternCollection>& key, const EventContext& ctx) const {
        if (!patCol) {
            ATH_MSG_WARNING("Zero pointer, could not save patterns!!! ");
            return;
        }

        // check whether we are writing patterns to storegate, if not delete pattern
        if (!m_recordAllOutput) {
            ATH_MSG_DEBUG("Deleted patterns: " << patCol->size() << "  at " << key.key());
            // since patCol Datavector, it owns (by defaults its elements)

        } else {
            SG::WriteHandle<MuonPrdPatternCollection> handle(key, ctx);
            StatusCode sc = handle.record(std::move(patCol));
            if (sc.isFailure()) {
                ATH_MSG_WARNING("Could not save patterns at " << key.key());
            } else {
                ATH_MSG_DEBUG("Saved patterns: " << patCol->size() << "  at " << key.key());
            }
        }
    }
    void MuonHoughPatternFinderTool::addRpcCollection(
        const RpcPrepDataCollection* rpc_coll, MuonHoughHitContainer& hitcontainer,
        std::map<int, std::vector<std::pair<int, int>>>& rpcmdtstationmap,
        EtaPhiHitAssocMap& phietahitassociation) const {
        std::set<int> layers;  // different layer definition between the two!!

        int size_begin = hitcontainer.size();
        addCollection(*rpc_coll, hitcontainer, phietahitassociation);
        int size_end = hitcontainer.size();

        updateRpcMdtStationMap((*rpc_coll->begin())->identify(), size_begin, size_end, rpcmdtstationmap);
    }

    void MuonHoughPatternFinderTool::addTgcCollection(
        const Muon::TgcPrepDataCollection* tgc_coll, MuonHoughHitContainer& hitcontainer,
        std::map<int, std::vector<std::pair<int, int>>>& tgcmdtstationmap,
        EtaPhiHitAssocMap& phietahitassociation) const {
       
        int size_begin = hitcontainer.size();
        addCollection(*tgc_coll, hitcontainer, phietahitassociation);        
        int size_end = hitcontainer.size();
        updateTgcMdtStationMap((*tgc_coll->begin())->identify(), size_begin, size_end, tgcmdtstationmap);
    }

    void MuonHoughPatternFinderTool::addMdtCollection(const MdtPrepDataCollection* mdt_coll, MuonHoughHitContainer& hitcontainer,
                                                      std::map<int, std::vector<std::pair<int, int>>>& rpcmdtstationmap,
                                                      std::map<int, std::vector<std::pair<int, int>>>& tgcmdtstationmap) const {
        const unsigned int size = mdt_coll->size();
        if (!size) return;
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};

        auto new_mdt_hit = [](const Muon::MdtPrepData* mdt_hit, double prob, double weight) {
            return std::make_shared<MuonHoughHit>(mdt_hit->globalPosition(), false /*measures_phi*/, MuonHough::MDT, prob, weight, mdt_hit);  // getPrd
        };
        if (m_showerskip) {
            const Muon::MdtPrepData* mdt = (*mdt_coll->begin());
            const MuonGM::MdtReadoutElement* detEl = mdt->detectorElement();
            unsigned int channels = 2 * detEl->getNLayers() * detEl->getNtubesperlayer();  // Factor 2 for number of multilayers, should
                                                                                           // be changed when only 1 detector element per
                                                                                           // chamber (the chambers with only 1
                                                                                           // multilayer have a twice less severe cut
                                                                                           // (for convenience))
            double occupancy = (double)size / (double)channels;

            ATH_MSG_DEBUG(" size: " << size << " channels: " << channels << " occupancy: " << occupancy);

            // if more than m_showerskipperc (default 30%) of all hits in the chamber is
            // hit then all weights to 0 only done for large chambers (more than 50
            // hits)
            if (occupancy > m_showerskipperc && size > 50) {
                ATH_MSG_DEBUG("Chamber skipped! Too high occupancy (>" << m_showerskipperc << "%): " << occupancy
                                                                       << " association to pattern still possible");

                for (const MdtPrepData* mdt_hit : *mdt_coll) {
                    if (m_mdt_tdc_cut && mdt_hit->status() != Muon::MdtStatusDriftTime) continue;
                    if ((m_mdt_adc_cut && (mdt_hit->adc() > m_mdt_adc_min)) || !m_mdt_adc_cut) {
                        ATH_MSG_DEBUG(m_printer->print(*mdt_hit));
                        hitcontainer.addHit(new_mdt_hit(mdt_hit, 0., 0.));
                    }
                }
                return;
            }
        }

        std::map<int, int> nHitsPerLayer;
        std::map<int, int> number_of_hots_per_layer;  // number of trigger confirmed or hits on segment
                                                      // within layer (key)

        std::vector<SegmentData> collected_data{};
        collected_data.reserve(size);

        std::vector<double> tubecount(idHelper.tubeMax() + 2);



        for (const Muon::MdtPrepData* mdt : *mdt_coll)  // first
        {
            
            if (m_mdt_tdc_cut && mdt->status() != Muon::MdtStatusDriftTime) {
                ATH_MSG_VERBOSE("Skip Mdt hit "<<m_printer->print(*mdt)<<" due to out of time tdc");
                continue;
            }
            if (m_mdt_adc_cut && (mdt->adc() <= m_mdt_adc_min))  {
                ATH_MSG_VERBOSE("Skip Mdt hit "<<m_printer->print(*mdt)<< "due to too low adc:"<<mdt->adc()<<". Required "<<m_mdt_adc_min);
                continue; 
            }
            SegmentData prd_data{};
            prd_data.index = collected_data.size();
            prd_data.prd = mdt;

            const int tube = idHelper.tube(prd_data.id());
            const int multi_layer = idHelper.multilayer(prd_data.id());
            const int tube_layer = idHelper.tubeLayer(prd_data.id());

            prd_data.layer_number =
                (multi_layer - 1) * idHelper.tubeLayerMax() + (tube_layer - 1);  // layer_number ranges from 0..5/7

            tubecount[tube] += 1.;
            tubecount[tube - 1] += 0.5;
            tubecount[tube + 1] += 0.5;

            ATH_MSG_VERBOSE(" layer_number: " << prd_data.layer_number << " multi_layer: " << multi_layer
                                                << " tube_layer: " << tube_layer);
            collected_data.push_back(std::move(prd_data));
        }

        const unsigned int prdsize = collected_data.size();

        if (!prdsize) return;

        if (!m_hit_reweights) {
            for (const SegmentData& mdt_hit : collected_data) {
                ATH_MSG_DEBUG(m_printer->print(*mdt_hit.prd));
                hitcontainer.addHit(new_mdt_hit(mdt_hit.prd, 1., 1.));
            }
            return;
        }

        double tubem = *(std::max_element(tubecount.begin(), tubecount.end()));

        // allweights 0
        if (tubem < 2.01) {
            ATH_MSG_VERBOSE(" TOO SMALL tubem : " << tubem);
            for (const SegmentData& mdt_hit : collected_data) {
                ATH_MSG_DEBUG(m_printer->print(*mdt_hit.prd) << " weight " << 0 << " adc: " << mdt_hit.prd->adc());
                hitcontainer.addHit(new_mdt_hit(mdt_hit.prd, 0., 0.));
                if (m_use_histos) {
                    Hists& h = getHists();
                    h.m_weighthistogram->Fill(0);
                    h.m_weighthistogrammdt->Fill(0);
                }

            }  // collection
            return;
        }

        // fast segment search:

        for (SegmentData& mdt_hit : collected_data) {
            const int tube = idHelper.tube(mdt_hit.id());
            if (tubecount[tube] > 1) ++nHitsPerLayer[mdt_hit.layer_number];

            // KILL 1 hit cases
            if (tubecount[tube] <= 1.) mdt_hit.prob = 0.;
        }  // end hit loop i

        int ml1{0}, ml2{0};
        for (const auto& map_it : nHitsPerLayer) {
            const bool count_1 = map_it.first >= idHelper.tubeLayerMax();
            ml1 += count_1;
            ml2 += !count_1;
        }

        // allweights = 0
        if (ml1 + ml2 < 2.01) {
            ATH_MSG_VERBOSE(" TOO SMALL ml1 + ml2 : " << ml1 << " ml2 " << ml2);
            for (const SegmentData& mdt_hit : collected_data) {
                ATH_MSG_DEBUG(m_printer->print(*mdt_hit.prd) << " weight " << 0);
                hitcontainer.addHit(new_mdt_hit(mdt_hit.prd, 0., 0.));
                if (m_use_histos) {
                    Hists& h = getHists();
                    h.m_weighthistogram->Fill(0);
                    h.m_weighthistogrammdt->Fill(0);
                }
            }  // collection
            return;
        }

        DCVec dcs;
        dcs.reserve(prdsize);
        const MdtIdHelper& mdtHelper = m_idHelperSvc->mdtIdHelper();
        for (const SegmentData& mdt_hit : collected_data) {
            if (mdt_hit.prob < 0.01) continue;

            // create new DriftCircircleMath::DriftCircle::DriftState
            const Amg::Vector3D& globalpos = mdt_hit.prd->globalPosition();
            
            const Identifier hitId = mdt_hit.id();
            TrkDriftCircleMath::MdtId mdtid(mdtHelper.isBarrel(hitId), 
                                            mdtHelper.multilayer(hitId) - 1, 
                                            mdtHelper.tubeLayer(hitId) - 1,
                                            mdtHelper.tube(hitId) - 1);
            TrkDriftCircleMath::DriftCircle dc(TrkDriftCircleMath::LocVec2D(globalpos.perp(), globalpos.z()), 
                                               mdt_hit.radius(),
                                               mdt_hit.errradius(), 
                                               TrkDriftCircleMath::DriftCircle::InTime, 
                                               mdtid, 
                                               nullptr,
                                               mdt_hit.index);
            dcs.emplace_back(std::move(dc));
        }

        bool seg_found = true;
        while (seg_found) {
            std::vector<int> sel(dcs.size());
            double angleDif = 0.;

            fastSegmentFinder(dcs, ml1, ml2, angleDif, sel);

            if (ml1 + ml2 >= 2.1) {
                int removed_hits = 0;  // keeps track of number of removed hits
                for (unsigned int i = 0; i < sel.size(); ++i) {
                    if (sel[i] != 0) {
                        unsigned int j = dcs[i - removed_hits].index();  // index of position in prd vec
                        SegmentData& mdt_hit = collected_data[j];
                        mdt_hit.onsegment = 1;
                        mdt_hit.psi = angleDif;
                        ++number_of_hots_per_layer[mdt_hit.layer_number];

                        // remove hit from dcs container for next iteration!!
                        dcs.erase(dcs.begin() + i - removed_hits);
                        ++removed_hits;
                    }
                }
            } else {
                seg_found = false;
            }
        }

        // trigger confirmation checks:

        int stationcode = stationCode(collected_data[0].id());
        const bool barrel = idHelper.isBarrel(collected_data[0].id());
        // rpc:

        std::map<int, std::vector<std::pair<int, int>>>::const_iterator stationmap_it = rpcmdtstationmap.find(stationcode);

        if (stationmap_it != rpcmdtstationmap.end()) {
            const std::vector<std::pair<int, int>>& stationhits = (*stationmap_it).second;

            // stationloop
            for (unsigned int i = 0; i < stationhits.size(); i++) {
                // rpc hit loop
                for (int j = stationhits[i].first; j < stationhits[i].second; j++) {
                    const std::shared_ptr<MuonHoughHit> rpchit = hitcontainer.getHit(j);
                    if (rpchit->getWeight() < 0.01) continue;
                    const Amg::Vector3D& rpcPos = rpchit->getPosition();
                    const double rpc_radius = rpcPos.perp();
                    const double rpc_rz_ratio = rpc_radius / rpcPos.z();
                    const double rpc_inv_rz_ratio = 1. / rpc_rz_ratio;

                    for (SegmentData& mdt_hit : collected_data) {
                        // Mdt hit loop
                        double dis = 0.;
                        const Amg::Vector3D& globalpos = mdt_hit.prd->globalPosition();
                        if (barrel) {
                            dis = globalpos.z() - globalpos.perp() * rpc_inv_rz_ratio;
                        } else {  // can that happen?
                            dis = globalpos.perp() - rpc_rz_ratio * globalpos.z();
                        }

                        /// if (mdt_hit.weighted_trigger < 0.1) { mdt_hit.weighted_trigger = 1.; }

                        if (std::abs(dis) < 250.) {
                            double wnew = 1.5 + (250. - std::abs(dis)) / 251.;
                            mdt_hit.weighted_trigger = std::max(mdt_hit.weighted_trigger, wnew);
                        }
                    }
                }
            }
        }

        // tgc:

        stationmap_it = tgcmdtstationmap.find(stationcode);

        if (stationmap_it != tgcmdtstationmap.end()) {
            const std::vector<std::pair<int, int>>& stationhits = (*stationmap_it).second;

            // stationloop
            for (unsigned int i = 0; i < stationhits.size(); i++) {
                // tgc hit loop
                for (int j = stationhits[i].first; j < stationhits[i].second; j++) {
                    const std::shared_ptr<MuonHoughHit> tgchit = hitcontainer.getHit(j);
                    if (!tgchit || tgchit->getWeight() < 0.01) continue;
                    const Amg::Vector3D& tgcPos = tgchit->getPosition();
                    const double tgc_rz_ratio = tgcPos.perp() / tgcPos.z();

                    for (SegmentData& mdt_hit : collected_data) {
                        // Mdt hit loop
                        if (mdt_hit.weighted_trigger < 0.1) mdt_hit.weighted_trigger = 3.;
                        const Amg::Vector3D& globalpos = mdt_hit.prd->globalPosition();
                        double dis = globalpos.perp() - tgc_rz_ratio * globalpos.z();  // only endcap extrapolation
                        if (std::abs(dis) < 250.) {
                            double wnew = 3.5 + (250. - std::abs(dis)) / 251.;
                            mdt_hit.weighted_trigger = std::max(mdt_hit.weighted_trigger, wnew);
                        }
                    }
                }
            }
        }

        // define trigger confirmation:

        for (SegmentData& mdt_hit : collected_data) {
            // for MDTs require trigger chamber confirmation
            //                  or segment with selected hits

            mdt_hit.tr_confirmation = (mdt_hit.weighted_trigger > 1.5 && mdt_hit.weighted_trigger < 2.55) ||
                                      (mdt_hit.weighted_trigger > 3.5 && mdt_hit.weighted_trigger < 4.55);

            // add confirmed hits to hots layer count:
            if (mdt_hit.tr_confirmation && !mdt_hit.onsegment) {  // else already added
                ++number_of_hots_per_layer[mdt_hit.layer_number];
            }
        }

        // calculate final weights:

        for (SegmentData& mdt_hit : collected_data) {
            if (mdt_hit.prob < 0.01) {
                mdt_hit.weights = 0;
                continue;
            }  // throw away hits that are not significant

            // correct for several number of hits in layer:
            std::map<int, int>::const_iterator map_it = nHitsPerLayer.find(mdt_hit.layer_number);
            if (map_it != nHitsPerLayer.end()) {
                int layerhits = (*map_it).second;
                double layer_weight = 1. / (0.25 * layerhits + 0.75 * std::sqrt(layerhits));

                if (!mdt_hit.tr_confirmation && !mdt_hit.onsegment) {
                    // downweighting for non-confirmed hits:
                    mdt_hit.prob = std::max(0., mdt_hit.prob - 0.2);
                    // correct for several number of hits in layer:
                    mdt_hit.weights = mdt_hit.prob * layer_weight;
                }

                else {
                    // Correct probabilities for hits on segment or confirmed by RPC/TGC
                    double rej = 1. / (1. - layer_weight + 0.10);
                    double rej0 = 1.;  // irrevelant value

                    if (mdt_hit.onsegment && mdt_hit.tr_confirmation) {
                        rej0 = 30;
                    } else if (mdt_hit.onsegment) {
                        rej0 = 1.75 / (mdt_hit.psi + 0.05);
                    }  // 1.75 = 5*0.35
                    else if (mdt_hit.tr_confirmation) {
                        rej0 = 8;
                    }

                    double rej_total = rej * rej0;
                    mdt_hit.prob = rej_total / (1. + rej_total);

                    // correct for several number of confirmed hits in layer:
                    map_it = number_of_hots_per_layer.find(mdt_hit.layer_number);
                    if (map_it != number_of_hots_per_layer.end()) {
                        int layerhits_conf = (*map_it).second;
                        mdt_hit.weights = mdt_hit.prob / (0.25 * layerhits_conf + 0.75 * std::sqrt(layerhits_conf));
                    } else {
                        ATH_MSG_INFO("Entry not in map! This should not happen");
                        mdt_hit.weights = mdt_hit.prob;
                    }
                }
            } else {
                ATH_MSG_INFO("Entry not in map! This should not happen");
                mdt_hit.weights = mdt_hit.prob;
            }

            /// and finally add hits to container:

            ATH_MSG_DEBUG(m_printer->print(*mdt_hit.prd)
                          << " trigger weight " << mdt_hit.weighted_trigger << " on segment " << mdt_hit.onsegment << " psi " << mdt_hit.psi
                          << " prob " << mdt_hit.prob << " weight " << mdt_hit.weights);
            hitcontainer.addHit(new_mdt_hit(mdt_hit.prd, mdt_hit.prob, mdt_hit.weights));
            if (m_use_histos) {
                Hists& h = getHists();
                h.m_weighthistogram->Fill(mdt_hit.weights);
                h.m_weighthistogrammdt->Fill(mdt_hit.weights);
            }

        }  // collection
    }
    template <class CollContainer> void MuonHoughPatternFinderTool::addCollections(const std::vector<const CollContainer*>& colls,
                                                          MuonHoughHitContainer& hitcontainer,
                                                          EtaPhiHitAssocMap& phietahitassociation) const {
        for (const auto* cont_ptr : colls) addCollection(*cont_ptr, hitcontainer, phietahitassociation);
    }
    template <class CollContainer> void MuonHoughPatternFinderTool::addCollection(const CollContainer& cont,
                                                          MuonHoughHitContainer& hitcontainer,
                                                          EtaPhiHitAssocMap& phietahitassociation) const{
        if (cont.empty()) return;
        std::map<Identifier, unsigned> nHitsPerLayer{};        
        hitcontainer.reserve(cont.size() + hitcontainer.size());
        ///Define the maximum channel
        unsigned channel_max{0};

        if constexpr (std::is_same<CollContainer, CscPrepDataCollection>::value) {
            channel_max = m_idHelperSvc->cscIdHelper().stripMax();
        } else if constexpr (std::is_same<CollContainer, TgcPrepDataCollection>::value) {
            channel_max = m_idHelperSvc->tgcIdHelper().channelMax();
        } else if constexpr (std::is_same<CollContainer, RpcPrepDataCollection>::value) {
            channel_max = m_idHelperSvc->rpcIdHelper().stripMax();
        } else if constexpr(std::is_same<CollContainer, sTgcPrepDataCollection>::value) {
            /// Enlarge the vector artifically by 3 to make enough space for Pads, strip and wires which
            /// are all aligned in the same channel vector
            channel_max = m_idHelperSvc->stgcIdHelper().channelMax(cont.front()->identify()) * 3;
        } else if constexpr(std::is_same<CollContainer, MMPrepDataCollection>::value) {            
            channel_max = m_idHelperSvc->mmIdHelper().channelMax(cont.front()->identify());
        }

        auto layer_channel = [this](const Identifier& id) {
            if constexpr (std::is_same<CollContainer, CscPrepDataCollection>::value) {
                return m_idHelperSvc->cscIdHelper().channel(id);
            } else if constexpr(std::is_same<CollContainer, TgcPrepDataCollection>::value) {
                return m_idHelperSvc->tgcIdHelper().channel(id);
            } else if constexpr(std::is_same<CollContainer, RpcPrepDataCollection>::value) {
                return m_idHelperSvc->rpcIdHelper().channel(id);
            } else if constexpr(std::is_same<CollContainer, sTgcPrepDataCollection>::value) {
                return m_idHelperSvc->stgcIdHelper().channel(id);
            } else  if constexpr(std::is_same<CollContainer, MMPrepDataCollection>::value) {
                return m_idHelperSvc->mmIdHelper().channel(id);
            }
            return 1;
        };

        std::set<int> layers{};

        auto layer_number = [this, &layers]( const Identifier& id ) -> int{
            if constexpr (std::is_same<CollContainer,RpcPrepDataCollection>::value) {
                const int n_gasGaps = m_idHelperSvc->rpcIdHelper().gasGapMax(id);
                const int n_doubR = m_idHelperSvc->rpcIdHelper().doubletRMax(id);
                return n_gasGaps* (m_idHelperSvc->rpcIdHelper().doubletR(id) - 1) + 
                       (m_idHelperSvc->rpcIdHelper().gasGap(id) -1) + n_doubR*n_gasGaps*m_idHelperSvc->measuresPhi(id);
            } else if constexpr(std::is_same<CollContainer,TgcPrepDataCollection>::value) {
                const int n_gasGaps = m_idHelperSvc->tgcIdHelper().gasGapMax(id);
                return (m_idHelperSvc->tgcIdHelper().gasGap(id) - 1) + n_gasGaps * m_idHelperSvc->measuresPhi(id);
            } else if constexpr(std::is_same<CollContainer,CscPrepDataCollection>::value) {
                const int n_layer = m_idHelperSvc->cscIdHelper().chamberLayerMax(id);
                const int n_chlay = m_idHelperSvc->cscIdHelper().wireLayerMax(id);
                return n_layer*(m_idHelperSvc->cscIdHelper().wireLayer(id) -1) + 
                       (m_idHelperSvc->cscIdHelper().chamberLayer(id) -1) + n_layer*n_chlay * m_idHelperSvc->measuresPhi(id);
            } else if constexpr(std::is_same<CollContainer,sTgcPrepDataCollection>::value) {
                const int n_lay = m_idHelperSvc->stgcIdHelper().gasGapMax(id);
                const int n_chtype = m_idHelperSvc->stgcIdHelper().channelTypeMax(id);
                return n_chtype*n_lay*(m_idHelperSvc->stgcIdHelper().multilayer(id) - 1) + 
                        n_lay*(m_idHelperSvc->stgcIdHelper().channelType(id) -1) + 
                        (m_idHelperSvc->stgcIdHelper().gasGap(id) -1);
            } else if constexpr(std::is_same<CollContainer,MMPrepDataCollection>::value) {
                const int n_lay = m_idHelperSvc->mmIdHelper().gasGapMax(id);
                return n_lay*(m_idHelperSvc->mmIdHelper().multilayer(id) - 1) + 
                        (m_idHelperSvc->mmIdHelper().gasGap(id) -1);
            }
            ATH_MSG_VERBOSE("Layer numbers not implemented for "<<m_idHelperSvc->toString(id));
            return static_cast<int>(layers.size());
        };
       

        std::vector<float> channelWeights;

        /// Find channel accumulations
        if (!m_hit_reweights) {
            channelWeights.assign(2*channel_max + 2, 2.);            
        } else {
            channelWeights.assign(2*channel_max + 2, 0);
            for (const auto* prd : cont) {
                const bool measures_phi = m_idHelperSvc->measuresPhi(prd->identify());
                layers.insert(layer_number(prd->identify()));
                /// NSW prepdata has the pecularity that the prds are already clustered
                if constexpr (std::is_same<CollContainer, sTgcPrepDataCollection>::value ||
                            std::is_same<CollContainer, MMPrepDataCollection>::value) {
                    for (uint16_t ch : prd->stripNumbers()) {
                        const int channel = ch + measures_phi * channel_max;
                        channelWeights.at(channel -1) += 0.55;
                        channelWeights.at(channel) += 1.;
                        channelWeights.at(channel + 1) +=0.55;
                    }
                } else {
                    /// Find channels that have accumulated hits
                    const int channel = layer_channel(prd->identify()) + measures_phi * channel_max;
                    channelWeights[channel -1] += 0.55;
                    channelWeights[channel] += 1.;
                    channelWeights[channel + 1] +=0.55;
                }
            }
        }
        
        std::map<Identifier, PrepDataSet> gasgapphimap{}; 
        for (const auto* prd : cont) {
            const bool measures_phi = m_idHelperSvc->measuresPhi(prd->identify());
            const int channel = layer_channel(prd->identify()) + measures_phi * channel_max;
            /// Require that there is an adjacent channel
            nHitsPerLayer[m_idHelperSvc->layerId(prd->identify())] += layers.size() > 1 && (channelWeights[channel] > 1.);
          
            if (!measures_phi)  continue;            
            gasgapphimap[m_idHelperSvc->gasGapId(prd->identify())].insert(prd);
         }
        
        MuonHough::DetectorTechnology det_tech{MuonHough::DetectorTechnology::CSC};
        if constexpr(std::is_same<CollContainer, TgcPrepDataCollection>::value) {
            det_tech = MuonHough::TGC;
        } else if constexpr(std::is_same<CollContainer, RpcPrepDataCollection>::value) {
            det_tech = MuonHough::RPC;            
        }
        /// Second loop over the contrainer to fill the map
        for (const auto* prd : cont) {
            double weight = 1.;
            if (m_hit_reweights) {
                double number_of_hits = nHitsPerLayer[m_idHelperSvc->layerId(prd->identify())];
                weight = number_of_hits ? 1. / (0.25 * std::sqrt(number_of_hits) + 0.75 * number_of_hits) : 0.; 
                if( layers.size() == 2) weight /= 2.;               
            }

            const Identifier id = prd->identify();
            const bool measuresPhi = m_idHelperSvc->measuresPhi(id);

            std::shared_ptr<MuonHoughHit> hit = std::make_shared<MuonHoughHit>(prd->globalPosition(), 
                                                                    measuresPhi, det_tech, (weight > 0.), weight,  prd);
            
            hitcontainer.addHit(hit);
            ATH_MSG_DEBUG(m_printer->print(*prd) << " weight " << weight);            
            if (m_use_histos) {
                Hists& h = getHists();
                h.m_weighthistogram->Fill(weight);
                 if constexpr (std::is_same<CollContainer, CscPrepDataCollection>::value) {
                    h.m_weighthistogramcsc->Fill(weight);
                } else if constexpr(std::is_same<CollContainer, TgcPrepDataCollection>::value) {
                    h.m_weighthistogramtgc->Fill(weight);
                } else if constexpr(std::is_same<CollContainer, RpcPrepDataCollection>::value) {
                    h.m_weighthistogramrpc->Fill(weight);
                } else if constexpr(std::is_same<CollContainer, sTgcPrepDataCollection>::value) {
                   h.m_weighthistogramstgc->Fill(weight);
                } else  if constexpr(std::is_same<CollContainer, MMPrepDataCollection>::value) {
                   h.m_weighthistogrammm->Fill(weight);
                }                
            }
            if (!measuresPhi) {
                const PrepDataSet& phi_prds = gasgapphimap[m_idHelperSvc->gasGapId(prd->identify())];
                if (!phi_prds.empty()) phietahitassociation.insert(std::make_pair(prd, phi_prds));
            }
        }
    }
    void MuonHoughPatternFinderTool::updateRpcMdtStationMap(const Identifier rpcid, const int hit_begin, const int hit_end,
                                                            std::map<int, std::vector<std::pair<int, int>>>& rpcmdtstationmap) const {
        //  input is a RPC identifier, begin container and end container
        //  rpcmdtstationmap is updated
        //
        // called once per rpc collection/station

        ATH_MSG_VERBOSE("updateRpcMdtStationMap" << m_idHelperSvc->toString(rpcid));
        if (!m_idHelperSvc->isRpc(rpcid)) return;
        std::map<int, std::vector<std::pair<int, int>>>::iterator it;
        int stationcode = stationCode(rpcid);

        // store station code

        addToStationMap(rpcmdtstationmap, it, stationcode, hit_begin, hit_end);

        int idphi = m_idHelperSvc->rpcIdHelper().stationPhi(rpcid);
        int ideta = m_idHelperSvc->rpcIdHelper().stationEta(rpcid);

        int idphi1 = idphi - 1;
        if (idphi1 == 0) idphi1 = 8;
        int idphi2 = idphi + 1;
        if (idphi2 > 8) idphi2 = 1;

        std::map<int, int>::const_iterator station_itr = m_RpcToMdtOuterStDict.find(m_idHelperSvc->rpcIdHelper().stationName(rpcid));
        if (station_itr == m_RpcToMdtOuterStDict.end()) return;

        // store Neighbouring station codes
        int stationNameMDT = station_itr->second;

        stationcode = stationCode(stationNameMDT, idphi1, ideta);
        addToStationMap(rpcmdtstationmap, it, stationcode, hit_begin, hit_end);

        stationcode = stationCode(stationNameMDT, idphi2, ideta);
        addToStationMap(rpcmdtstationmap, it, stationcode, hit_begin, hit_end);

        //  Also look into Inner station

        // std::map<int, int> m_RpcToMdtInnerStDict{};
        station_itr = m_RpcToMdtInnerStDict.find(m_idHelperSvc->rpcIdHelper().stationName(rpcid));
        if (station_itr == m_RpcToMdtInnerStDict.end()) return;
        stationNameMDT = station_itr->second;
        stationcode = stationCode(stationNameMDT, idphi, ideta);
        addToStationMap(rpcmdtstationmap, it, stationcode, hit_begin, hit_end);
    }

    void MuonHoughPatternFinderTool::updateTgcMdtStationMap(const Identifier tgcid, int hit_begin, int hit_end,
                                                            std::map<int, std::vector<std::pair<int, int>>>& tgcmdtstationmap) const {
        //  input is a TGC identifier, begin container and end container
        //  tgcmdtstationmap is updated
        //
        // called once per tgc collection/station
        std::string st = m_idHelperSvc->tgcIdHelper().stationNameString(m_idHelperSvc->tgcIdHelper().stationName(tgcid));
        if (st[0] != 'T') return;

        constexpr std::array<int, 5> T31{2, 3, 3, 4, 4};
        constexpr std::array<int, 5> T32{3, 4, 4, 5, 5};
        constexpr std::array<int, 5> T11{2, 3, 4, 4, 4};
        constexpr std::array<int, 5> T12{3, 4, 5, 5, 5};

        std::map<int, std::vector<std::pair<int, int>>>::iterator it;

        // Determine station phi in MDT

        int modphiTGC = 48;
        if (st[2] == 'F') modphiTGC = 24;
        if (st[1] == '4') modphiTGC = 24;

        int idphi = m_idHelperSvc->tgcIdHelper().stationPhi(tgcid);
        int ideta = m_idHelperSvc->tgcIdHelper().stationEta(tgcid);
        int index = abs(ideta) - 1;
        int idphi1MDT = 1 + int(8. * (idphi + 1) / modphiTGC);
        int idphi2MDT = 1 + int(8. * (idphi - 1) / modphiTGC);
        if (idphi1MDT > 8) idphi1MDT = 1;
        if (idphi2MDT > 8) idphi2MDT = 1;

        int sign = 1;
        if (ideta < 0) sign = -1;

        // Determine two station etas  in MDT

        int ideta1MDT = 0;
        int ideta2MDT = 0;
        if (st[2] == 'F') {
            ideta1MDT = sign * 1;
            ideta2MDT = sign * 2;
        }
        if (st[2] == 'E') {
            if (st[1] == '4') {
                // T4
                ideta1MDT = sign * 4;
                ideta2MDT = sign * 5;
            } else if (st[1] == '3') {
                // T3
                ideta1MDT = sign * T31[index];
                ideta2MDT = sign * T32[index];
            } else {
                // T1 or T2
                ideta1MDT = sign * T11[index];
                ideta2MDT = sign * T12[index];
            }
        }
        std::string station1 = "EML";
        std::string station2 = "EMS";
        if (st[1] == '4') {
            station1 = "EIL";
            station2 = "EIS";
        }
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        int stationNameMDT1 = idHelper.stationNameIndex(station1);
        int stationNameMDT2 = idHelper.stationNameIndex(station2);

        // store station Inner and Middle codes

        int stationcode = stationCode(stationNameMDT1, idphi1MDT, ideta1MDT);
        addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
        stationcode = stationCode(stationNameMDT2, idphi1MDT, ideta1MDT);
        addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
        if (ideta1MDT != ideta2MDT) {
            stationcode = stationCode(stationNameMDT1, idphi1MDT, ideta2MDT);
            addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
            stationcode = stationCode(stationNameMDT2, idphi1MDT, ideta2MDT);
            addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
        }
        if (idphi1MDT != idphi2MDT) {
            stationcode = stationCode(stationNameMDT1, idphi2MDT, ideta1MDT);
            addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
            stationcode = stationCode(stationNameMDT2, idphi2MDT, ideta1MDT);
            addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
            if (ideta1MDT != ideta2MDT) {
                stationcode = stationCode(stationNameMDT1, idphi2MDT, ideta2MDT);
                addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
                stationcode = stationCode(stationNameMDT2, idphi2MDT, ideta2MDT);
                addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
            }
        }
        // Store corresponding Outer stations

        if (station1 == "EMS") { station1 = "EOS"; }
        if (station2 == "EML") {
            station2 = "EOL";
        } else
            return;

        stationNameMDT1 = idHelper.stationNameIndex(station1);
        stationNameMDT2 = idHelper.stationNameIndex(station2);

        stationcode = stationCode(stationNameMDT1, idphi1MDT, ideta1MDT);
        addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
        stationcode = stationCode(stationNameMDT2, idphi1MDT, ideta1MDT);
        addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
        if (ideta1MDT != ideta2MDT) {
            stationcode = stationCode(stationNameMDT1, idphi1MDT, ideta2MDT);
            addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
            stationcode = stationCode(stationNameMDT2, idphi1MDT, ideta2MDT);
            addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
        }
        if (idphi1MDT != idphi2MDT) {
            stationcode = stationCode(stationNameMDT1, idphi2MDT, ideta1MDT);
            addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
            stationcode = stationCode(stationNameMDT2, idphi2MDT, ideta1MDT);
            addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);

            if (ideta1MDT != ideta2MDT) {
                stationcode = stationCode(stationNameMDT1, idphi2MDT, ideta2MDT);
                addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
                stationcode = stationCode(stationNameMDT2, idphi2MDT, ideta2MDT);
                addToStationMap(tgcmdtstationmap, it, stationcode, hit_begin, hit_end);
            }
        }
    }

    int MuonHoughPatternFinderTool::stationCode(const Identifier& id) const {
        return stationCode(m_idHelperSvc->stationName(id), m_idHelperSvc->stationPhi(id),
                           m_idHelperSvc->stationEta(id));
    }

    int MuonHoughPatternFinderTool::stationCode(int stationname, int phi, int eta) {
        return 10000000 * stationname + 100000 * phi + 1000 * (eta + 10);
    }

    void MuonHoughPatternFinderTool::addToStationMap(std::map<int, std::vector<std::pair<int, int>>>& stationmap,
                                                     std::map<int, std::vector<std::pair<int, int>>>::iterator& it, int& stationcode,
                                                     const int& hit_begin, const int& hit_end) {
        it = stationmap.find(stationcode);
        if (it == stationmap.end()) {
            std::vector<std::pair<int, int>> dummyvec;
            dummyvec.emplace_back(hit_begin, hit_end);
            stationmap[stationcode] = dummyvec;
        } else {
            (*it).second.emplace_back(hit_begin, hit_end);
        }
    }

    void MuonHoughPatternFinderTool::fastSegmentFinder(TrkDriftCircleMath::DCVec& dcs, int& nl1, int& nl2, double& angleDif,
                                                       std::vector<int>& sel) const {
        //
        // Input:  vector of driftcircles per chamber
        // Output: nl1 = segment hits in multilayer 1 and nl2 = segment hits in
        // multilayer 2
        //       : sel(1:dcs.size)  = 0 NOT selected  = 1 on segment
        //
        // Method: constructs the tangent lines to all driftcircle combinations and
        // counts hits in a road of 1.5 mm
        //         segment = combination with most hits
        //         uses TrkDriftCircleMath software
        //

        // Layers with more than 10 hits are skipped as seed, if all layers have more
        // than 10 hits, no fits are tried

        nl1 = 0;
        nl2 = 0;
        angleDif = -1.;
        if (dcs.empty()) return;

        DCCit it_end = dcs.end();
        DCCit it1 = dcs.begin();
        std::map<int, DCVec> layerHits;  // map between layer and driftcircles
        std::map<int, int> dcsId;        // map between 'idnumber' and position

        std::map<int, DCVec>::iterator map_it;
        int nhits = 0;
        for (; it1 != it_end; ++it1, nhits++) {
            sel[nhits] = 0;
            int isort = MdtIdHelper::maxNTubesPerLayer * (4 * (it1->id().ml()) + it1->id().lay()) + it1->id().tube();
            dcsId[isort] = nhits;
            int ilay = 4 * (it1->id().ml()) + it1->id().lay();
            ATH_MSG_VERBOSE(" ilay " << ilay << " isort " << isort);

            map_it = layerHits.find(ilay);
            if (map_it != layerHits.end()) {
                (*map_it).second.push_back(*it1);
            } else {
                DCVec dcl;
                dcl.reserve(dcs.size());
                dcl.push_back(*it1);
                layerHits[ilay] = dcl;
            }
        }

        unsigned int nHits = 0;  // is maximalized
        unsigned int nHitsLine = 0;
        unsigned int nPassedTubes = 0;
        double roadWidth = 1.5;
        TrkDriftCircleMath::DCOnTrackVec hitsOnLineSel;
        TrkDriftCircleMath::MatchDCWithLine matchWithLine;
        bool stop = false;
        for (int i = 0; i < 8; i++) {
            if (layerHits.count(i) != 1) continue;
            DCVec& dci = layerHits[i];
            if (dci.size() > 10) continue;
            DCCit iti = dci.begin();
            DCCit iti_end = dci.end();
            for (; iti != iti_end; ++iti) {
                // One seed selected
                float tubeRadius = 14.6;
                if ((*iti).rot()) {  // if no access to rot, can't do anything here
                    tubeRadius = (*iti).rot()->detectorElement()->innerTubeRadius();
                }
                for (int j = 7; j > i; j--) {
                    if (layerHits.count(j) != 1) continue;
                    DCVec& dcj = layerHits[j];
                    if (dcj.size() > 10) continue;
                    DCCit itj = dcj.begin();
                    DCCit itj_end = dcj.end();
                    for (; itj != itj_end; ++itj) {
                        // Second seed selected
                        double hitx = (*itj).x();
                        double hity = (*itj).y();
                        double norm = std::hypot(hitx, hity);
                        double cphi = hitx / norm;
                        double sphi = hity / norm;
                        TrkDriftCircleMath::TangentToCircles::LineVec lines = TrkDriftCircleMath::TangentToCircles::tangentLines(*iti, *itj);
                        for (TrkDriftCircleMath::TangentToCircles::LineVec::const_iterator lit = lines.begin(); lit != lines.end(); ++lit) {
                            double coshit = std::cos((*lit).phi());
                            double sinhit = std::sin((*lit).phi());
                            const double cospsi = std::min(std::max(-1.,coshit * cphi + sinhit * sphi), 1.);
                            double psi = std::acos(cospsi);
                            if (psi > 0.3) continue;
                            matchWithLine.set(*lit, roadWidth, TrkDriftCircleMath::MatchDCWithLine::Road, tubeRadius);
                            const TrkDriftCircleMath::DCOnTrackVec& hitsOnLine = matchWithLine.match(dcs);
                            unsigned int matchedHits = matchWithLine.hitsOnTrack();
                            ATH_MSG_VERBOSE(" Summary nHits " << matchedHits << " nl1 " << matchWithLine.hitsMl1() << " nl2 "
                                                              << matchWithLine.hitsMl2());
                            if (matchedHits > nHits || (matchedHits == nHits && psi < angleDif)) {
                                int dnl = std::abs(static_cast<int>(matchWithLine.hitsMl1()) - static_cast<int>(matchWithLine.hitsMl2()));
                                ATH_MSG_DEBUG(" matchWithLine.hitsOnTrack() >  nHits old " << nHits << " new: " << matchedHits);
                                ATH_MSG_DEBUG(" dnl " << dnl << " old dnl " << std::abs(nl1 - nl2));
                                ATH_MSG_DEBUG(" hit cos phi " << cphi << " line " << coshit << " sin phi " << sphi << " line " << sinhit
                                                              << " psi " << psi);

                                // update of variables:
                                nHits = matchedHits;
                                nl1 = matchWithLine.hitsMl1();
                                nl2 = matchWithLine.hitsMl2();
                                nHitsLine = hitsOnLine.size();
                                nPassedTubes = matchWithLine.passedTubes();
                                hitsOnLineSel = hitsOnLine;
                                angleDif = psi;
                            }

                            ATH_MSG_VERBOSE(" Select nHits " << nHits << " nl1 " << nl1 << " nl2 " << nl2);
                            if (nHits >= dcs.size()) stop = true;
                        }  // end lines
                        if (stop) break;
                    }  // end itj
                    if (stop) break;
                }  // end j
                if (stop) break;
            }  // end iti
            if (stop) break;
        }  // end i

        ATH_MSG_DEBUG(" Fast segment finder Max Layers hit " << dcs.size() << " nHitsLine - nHits " << nHitsLine - nl1 - nl2
                                                             << " passed Tubes -nHits " << nPassedTubes - nl1 - nl2 << " nl1 " << nl1
                                                             << " nl2 " << nl2 << " angleDif " << angleDif);

        TrkDriftCircleMath::DCOnTrackIt itt = hitsOnLineSel.begin();
        TrkDriftCircleMath::DCOnTrackIt itt_end = hitsOnLineSel.end();
        int i = 0;
        for (; itt != itt_end; ++itt, i++) {
            int isort = MdtIdHelper::maxNTubesPerLayer * (4 * (itt->id().ml()) + itt->id().lay()) + itt->id().tube();
            if (dcsId.count(isort) == 1) {
                int dcsIndex = dcsId[isort];
                sel[dcsIndex] = 1;

                ATH_MSG_DEBUG(" Selected Hit index " << dcsIndex << " MultiLayer " << itt->id().ml() << " layer " << itt->id().lay()
                                                     << " tube " << itt->id().tube());
            } else {
                ATH_MSG_WARNING(" ALARM fastSegmentFinder hit NOT found " << i << " isort " << isort);
            }
        }
    }

    MuonHoughPatternFinderTool::Hists& MuonHoughPatternFinderTool::getHists() const {
        // We earlier checked that no more than one thread is being used.
        Hists* h ATLAS_THREAD_SAFE = m_h.get();
        return *h;
    }

}  // namespace Muon
