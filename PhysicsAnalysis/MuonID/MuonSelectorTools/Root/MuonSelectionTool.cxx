/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSelectorTools/MuonSelectionTool.h"

#include "AsgDataHandles/ReadHandle.h"
#include "PathResolver/PathResolver.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "CxxUtils/trapping_fp.h"

namespace {
    static constexpr double const MeVtoGeV = 1. / 1000.;
    // This function defines the order of chamber indices for the low-pT MVA,
    // i.e. defining the meaning of "the first two segments", which are used in the BDT
    std::vector<int> initializeChamberIdxOrder() {
        // This vector defines the order. The current order follows the order of the enum "ChIndex"
        // except for the CSCs, which appear first. Since the order is not strictly innermost-to-outermost,
        // a reordering could be considered for a rel. 22 retuning, which can then easily be achieved by
        // swapping around the elements in the below initialization.
        using ChIdx = Muon::MuonStationIndex::ChIndex;
        using namespace Muon::MuonStationIndex;
        const std::vector<ChIdx> orderedChIndices{
            ChIdx::CSS, ChIdx::CSL, ChIdx::BIS, ChIdx::BIL,
            ChIdx::BMS, ChIdx::BML, ChIdx::BOS, ChIdx::BOL,
            ChIdx::BEE, ChIdx::EIS, ChIdx::EIL, ChIdx::EMS,
            ChIdx::EML, ChIdx::EOS, ChIdx::EOL, ChIdx::EES,
            ChIdx::EEL};

        // This vector will hold the equivalent information in a form that can be efficiently accessed in the
        // below function "chamberIndexCompare", using the chamber index as the vector index
        std::vector<int> chamberIndexOrder(orderedChIndices.size());

        for (unsigned int i = 0; i < orderedChIndices.size(); i++) {
            chamberIndexOrder[toInt(orderedChIndices[i])] = i;
        }
        return chamberIndexOrder;
    }

    // This is the comparison function for the sorting of segments according to the chamber index
    bool chamberIndexCompare(const xAOD::MuonSegment* first, const xAOD::MuonSegment* second) {
        static const std::vector<int> chamberIndexOrder = initializeChamberIdxOrder();
        return (chamberIndexOrder[toInt(first->chamberIndex())] < 
                chamberIndexOrder[toInt(second->chamberIndex())]);
    }

    static const SG::AuxElement::Accessor<float> mePt_acc("MuonSpectrometerPt");
    static const SG::AuxElement::Accessor<float> idPt_acc("InnerDetectorPt");
    static const SG::AuxElement::Accessor<uint8_t> eta1stgchits_acc("etaLayer1STGCHits");
    static const SG::AuxElement::Accessor<uint8_t> eta2stgchits_acc("etaLayer2STGCHits");
    static const SG::AuxElement::Accessor<uint8_t> mmhits_acc("MMHits");
}  // namespace

namespace CP {

    MuonSelectionTool::MuonSelectionTool(const std::string& tool_name): 
        asg::AsgTool(tool_name){}

    MuonSelectionTool::~MuonSelectionTool() = default;

    StatusCode MuonSelectionTool::initialize() {
    
        // Greet the user:
        ATH_MSG_INFO("Initialising...");
        
        m_geoOnTheFly ? ATH_MSG_INFO("Is Run-3 geometry: On-the-fly determination. THIS OPTION IS DEPRECATED AND WILL BE REMOVED SOON. Use IsRun3Geo property instead.") 
                      : ATH_MSG_INFO("Is Run-3 geometry: " << m_isRun3.value());
        ATH_MSG_INFO("Maximum muon |eta|: " << m_maxEta.value());
        ATH_MSG_INFO("Muon quality: "<< m_quality.value());
        if (m_toroidOff) ATH_MSG_INFO("!! CONFIGURED FOR TOROID-OFF COLLISIONS !!");
        if (m_SctCutOff) ATH_MSG_WARNING("!! SWITCHING SCT REQUIREMENTS OFF !! FOR DEVELOPMENT USE ONLY !!");
        if (m_PixCutOff) ATH_MSG_WARNING("!! SWITCHING PIXEL REQUIREMENTS OFF !! FOR DEVELOPMENT USE ONLY !!");
        if (m_SiHolesCutOff) ATH_MSG_WARNING("!! SWITCHING SILICON HOLES REQUIREMENTS OFF !! FOR DEVELOPMENT USE ONLY !!");
        if (m_custom_dir != "")
            ATH_MSG_WARNING("!! SETTING UP WITH USER SPECIFIED INPUT LOCATION \"" << m_custom_dir << "\"!! FOR DEVELOPMENT USE ONLY !! ");
        if (!m_useAllAuthors)
            ATH_MSG_WARNING(
                "Not using allAuthors variable as currently missing in many derivations; LowPtEfficiency working point will always return "
                "false, but this is expected at the moment. Have a look here: "
                "https://twiki.cern.ch/twiki/bin/view/Atlas/MuonSelectionToolR21#New_LowPtEfficiency_working_poin");

        // Print message to ensure that users excluding 2-station muons in the high-pT selection are aware of this
        if (!m_use2stationMuonsHighPt)
            ATH_MSG_INFO("You have opted to select only 3-station muons in the high-pT selection! "
                         << "Please feed 'HighPt3Layers' to the 'WorkingPoint' property to retrieve the appropriate scale-factors");

        // Only an MVA-based selection is defined for segment-tagged muons for the Low-pT working point
        if (m_useSegmentTaggedLowPt && !m_useMVALowPt) {
            ATH_MSG_WARNING("No cut-based selection is defined for segment-tagged muons in the Low-pT working point. "
                            << "Please set UseMVALowPt=true if you want to try the UseSegmentTaggedLowPt=true option.");
            m_useSegmentTaggedLowPt = false;
        }
        if (m_useLRT) { 
            ATH_MSG_INFO("MuonSelectionTool will assume both Standard and LRT Muons are being used, and that the necessary information is available to identify the type (standard or LRT).");
            if (m_quality!=1) ATH_MSG_WARNING("Currently, only Medium quality is supported for LRT muons. Your chosen WP will be applied (w/o ID cuts), but no recommendations are available for this quality.");
        }

        // Set up the TAccept object:
        m_acceptInfo.addCut("Eta", "Selection of muons according to their pseudorapidity");
        m_acceptInfo.addCut("IDHits", "Selection of muons according to whether they passed the MCP ID Hit cuts");
        m_acceptInfo.addCut("Preselection", "Selection of muons according to their type/author");
        m_acceptInfo.addCut("Quality", "Selection of muons according to their tightness");
        // Sanity check
        if (m_quality > 5) {
            ATH_MSG_ERROR(
                "Invalid quality (i.e. selection WP) set: "
                << m_quality
                << " - it must be an integer between 0 and 5! (0=Tight, 1=Medium, 2=Loose, 3=Veryloose, 4=HighPt, 5=LowPtEfficiency)");
            return StatusCode::FAILURE;
        }
        if (m_quality == 5 && !m_useAllAuthors) {
            ATH_MSG_ERROR("Cannot use lowPt working point if allAuthors is not available!");
            return StatusCode::FAILURE;
        }
        
        if(m_caloScoreWP<1 || m_caloScoreWP>4){
          ATH_MSG_FATAL("CaloScoreWP property must be set to 1, 2, 3 or 4");
          return StatusCode::FAILURE;
        }

        // Load Tight WP cut-map
        ATH_MSG_INFO("Initialising tight working point histograms...");
        std::string tightWP_rootFile_fullPath;
        if (!m_custom_dir.empty()) {
            tightWP_rootFile_fullPath = PathResolverFindCalibFile(m_custom_dir + "/muonSelection_tightWPHisto.root");
        } else {
            tightWP_rootFile_fullPath = PathResolverFindCalibFile(
                Form("MuonSelectorTools/%s/muonSelection_tightWPHisto.root", m_calibration_version.value().c_str()));
        }

        ATH_MSG_INFO("Reading muon tight working point histograms from " << tightWP_rootFile_fullPath);
        //
        std::unique_ptr<TFile> file(TFile::Open(tightWP_rootFile_fullPath.c_str(), "READ"));

        if (!file->IsOpen()) {
            ATH_MSG_ERROR("Cannot read tight working point file from " << tightWP_rootFile_fullPath);
            return StatusCode::FAILURE;
        }

        // Retrieve all the relevant histograms
        ATH_CHECK(getHist(file.get(), "tightWP_lowPt_rhoCuts", m_tightWP_lowPt_rhoCuts));
        ATH_CHECK(getHist(file.get(), "tightWP_lowPt_qOverPCuts", m_tightWP_lowPt_qOverPCuts));
        ATH_CHECK(getHist(file.get(), "tightWP_mediumPt_rhoCuts", m_tightWP_mediumPt_rhoCuts));
        ATH_CHECK(getHist(file.get(), "tightWP_highPt_rhoCuts", m_tightWP_highPt_rhoCuts));
        //
        file->Close();

        // Read bad muon veto efficiency histograms
        std::string BMVcutFile_fullPath = PathResolverFindCalibFile(m_BMVcutFile);

        ATH_MSG_INFO("Reading bad muon veto cut functions from " << BMVcutFile_fullPath);
        //
        std::unique_ptr<TFile> BMVfile(TFile::Open(BMVcutFile_fullPath.c_str(), "READ"));

        if (!BMVfile->IsOpen()) {
            ATH_MSG_ERROR("Cannot read bad muon veto cut function file from " << BMVcutFile_fullPath);
            return StatusCode::FAILURE;
        }

        m_BMVcutFunction_barrel = std::unique_ptr<TF1>((TF1*)BMVfile->Get("BMVcutFunction_barrel"));
        m_BMVcutFunction_endcap = std::unique_ptr<TF1>((TF1*)BMVfile->Get("BMVcutFunction_endcap"));

        BMVfile->Close();

        if (!m_BMVcutFunction_barrel || !m_BMVcutFunction_endcap) {
            ATH_MSG_ERROR("Cannot read bad muon veto cut functions");
            return StatusCode::FAILURE;
        }

        if (m_useMVALowPt) {
            if (m_isRun3) {
                // Helper lambda to load BDT model and scaler
                auto loadLowPtMVABDTModel = [&](const std::string& calibFile, 
                                    std::unique_ptr<MVAUtils::BDT>& bdt,
                                    std::vector<double>& means,
                                    std::vector<double>& scales) -> StatusCode {
                    const std::string modelFile = PathResolverFindCalibFile(calibFile);
                    std::unique_ptr<TFile> rootFile(TFile::Open(modelFile.c_str(), "READ"));
                    
                    if (!rootFile || rootFile->IsZombie()) {
                        ATH_MSG_ERROR("Failed to open model file for Run-3 LowPtMVA: " << modelFile);
                        return StatusCode::FAILURE;
                    }
                    
                    // Load BDT model
                    auto* bdt_tree = static_cast<TTree*>(rootFile->Get("xgboost"));
                    bdt = std::make_unique<MVAUtils::BDT>(bdt_tree);
                    
                    // Load scaler
                    std::unique_ptr<TTree> scaler_tree(static_cast<TTree*>(rootFile->Get("scaler")));
                    std::vector<double>* mean_ptr = nullptr;
                    std::vector<double>* scale_ptr = nullptr;
                    scaler_tree->SetBranchAddress("mean", &mean_ptr);
                    scaler_tree->SetBranchAddress("scale", &scale_ptr);
                    
                    for (Long64_t i = 0; i < scaler_tree->GetEntries(); ++i) {
                        scaler_tree->GetEntry(i);
                        for (size_t j = 0; j < mean_ptr->size(); ++j) {
                            means.push_back((*mean_ptr)[j]);
                            scales.push_back((*scale_ptr)[j]);
                        }
                    }
                    return StatusCode::SUCCESS;
                };

                // Load MuidCO model for Run-3 LowPtMVA
                ATH_CHECK(loadLowPtMVABDTModel("MuonSelectorTools/260130_LowPtMVA_r24run3/xgb_lowPtMVA_MuidCO_wScaler.root",
                            m_MuidCO, m_lowPtMuidCO_means, m_lowPtMuidCO_scaler));

                // Load MuGirl model for Run-3 LowPtMVA
                ATH_CHECK(loadLowPtMVABDTModel("MuonSelectorTools/260130_LowPtMVA_r24run3/xgb_lowPtMVA_MuGirl_wScaler.root",
                            m_MuGirl, m_lowPtMuGirl_means, m_lowPtMuGirl_scaler));
            }
            else{
                // Set up TMVA readers for MVA-based low-pT working point
                // E and O refer to even and odd event numbers to avoid applying the MVA on events used for training
                TString weightPath_EVEN_MuidCB = PathResolverFindCalibFile(m_MVAreaderFile_EVEN_MuidCB);
                TString weightPath_ODD_MuidCB = PathResolverFindCalibFile(m_MVAreaderFile_ODD_MuidCB);
                TString weightPath_EVEN_MuGirl = PathResolverFindCalibFile(m_MVAreaderFile_EVEN_MuGirl);
                TString weightPath_ODD_MuGirl = PathResolverFindCalibFile(m_MVAreaderFile_ODD_MuGirl);

                auto make_mva_reader = [](TString file_path) {
                    std::vector<std::string> mva_var_names{"momentumBalanceSignificance",
                                                        "scatteringCurvatureSignificance",
                                                        "scatteringNeighbourSignificance",
                                                        "EnergyLoss",
                                                        "middleLargeHoles+middleSmallHoles",
                                                        "muonSegmentDeltaEta",
                                                        "muonSeg1ChamberIdx",
                                                        "muonSeg2ChamberIdx"};
                    std::unique_ptr<TMVA::Reader> reader = std::make_unique<TMVA::Reader>(mva_var_names);
                    reader->BookMVA("BDTG", file_path);
                    return reader;
                };
                m_readerE_MUID = make_mva_reader(weightPath_EVEN_MuidCB);

                m_readerO_MUID = make_mva_reader(weightPath_ODD_MuidCB);

                m_readerE_MUGIRL = make_mva_reader(weightPath_EVEN_MuGirl);

                m_readerO_MUGIRL = make_mva_reader(weightPath_ODD_MuGirl);

                if (m_useSegmentTaggedLowPt) {
                    TString weightPath_MuTagIMO_etaBin1 = PathResolverFindCalibFile(m_MVAreaderFile_MuTagIMO_etaBin1);
                    TString weightPath_MuTagIMO_etaBin2 = PathResolverFindCalibFile(m_MVAreaderFile_MuTagIMO_etaBin2);
                    TString weightPath_MuTagIMO_etaBin3 = PathResolverFindCalibFile(m_MVAreaderFile_MuTagIMO_etaBin3);

                    auto make_mva_reader_MuTagIMO = [](TString file_path, bool useSeg2ChamberIndex) {
                        std::vector<std::string> mva_var_names;
                        if (useSeg2ChamberIndex) mva_var_names.push_back("muonSeg2ChamberIndex");
                        mva_var_names.push_back("muonSeg1ChamberIndex");
                        mva_var_names.push_back("muonSeg1NPrecisionHits");
                        mva_var_names.push_back("muonSegmentDeltaEta");
                        mva_var_names.push_back("muonSeg1GlobalR");
                        mva_var_names.push_back("muonSeg1Chi2OverDoF");
                        mva_var_names.push_back("muonSCS");

                        std::unique_ptr<TMVA::Reader> reader = std::make_unique<TMVA::Reader>(mva_var_names);
                        reader->BookMVA("BDT", file_path);
                        return reader;
                    };

                    m_reader_MUTAGIMO_etaBin1 = make_mva_reader_MuTagIMO(weightPath_MuTagIMO_etaBin1, false);
                    m_reader_MUTAGIMO_etaBin2 = make_mva_reader_MuTagIMO(weightPath_MuTagIMO_etaBin2, false);
                    m_reader_MUTAGIMO_etaBin3 = make_mva_reader_MuTagIMO(weightPath_MuTagIMO_etaBin3, true);
                }
            }
        }
        
        ATH_MSG_DEBUG("TightNNScore calculation is " << m_calculateTightNNScore);
        if (m_calculateTightNNScore) {
            ATH_CHECK(m_onnxTool.retrieve());
        } else {
            m_onnxTool.disable();
        }
        ATH_MSG_DEBUG("Finished ONNX tool setup");
        
        ATH_CHECK(m_eventInfo.initialize());
        // Initialize columnar accessor infrastructure (must be last in initialize)
        ATH_CHECK(initializeColumns());
        // Return gracefully:
        return StatusCode::SUCCESS;
    }

    StatusCode MuonSelectionTool::getHist(TFile* file, const std::string& histName, std::unique_ptr<TH1>& hist) const {
        //
        if (!file) {
            ATH_MSG_ERROR(" getHist(...) TFile is nullptr! Check that the Tight cut map is loaded correctly");
            return StatusCode::FAILURE;
        }
        TH1* h_ptr = nullptr;
        file->GetObject(histName.c_str(), h_ptr);
        //
        //
        if (!h_ptr) {
            ATH_MSG_ERROR("Cannot retrieve histogram " << histName);
            return StatusCode::FAILURE;
        }
        hist = std::unique_ptr<TH1>{h_ptr};
        hist->SetDirectory(nullptr);
        ATH_MSG_INFO("Successfully read tight working point histogram: " << hist->GetName());
        //
        return StatusCode::SUCCESS;
    }

    const asg::AcceptInfo& MuonSelectionTool::getAcceptInfo() const { return m_acceptInfo; }

    asg::AcceptData MuonSelectionTool::accept(const xAOD::IParticle* p) const {
        // Check if this is a muon:
        if (p->type() != xAOD::Type::Muon) {
            ATH_MSG_ERROR("accept(...) Function received a non-muon");
            return asg::AcceptData(&m_acceptInfo);
        }

        // Cast it to a muon:
        const xAOD::Muon* mu = dynamic_cast<const xAOD::Muon*>(p);
        if (!mu) {
            ATH_MSG_FATAL("accept(...) Failed to cast particle to muon");
            return asg::AcceptData(&m_acceptInfo);
        }

        // Let the specific function do the work:
        return accept(*mu);
    }

    //============================================================================
    void MuonSelectionTool::checkSanity() const {

        static std::atomic<bool> checkDone{false};

        if(!checkDone) {
            // Check that the user either set the correct geometry or enabled on-the-fly determination
            // This can happen intentionally in developer mode. In any case don't throw an exception
            // (we should either trust the user or ignore in the first place and force on-the-fly determination).
            // Note: isRun3(true) uses the xAOD event store, only available in xAOD mode (mode 0).
            if (columnar::columnarAccessMode == 0 && isRun3() != isRun3(true)) {
                ATH_MSG_WARNING("MuonSelectionTool is configured with isRun3Geo="<<isRun3()
                              <<" while on-the fly check for runNumber "<<getRunNumber(true)<<" indicates isRun3Geo="<<isRun3(true));
            }

            // Check that the requested WP is currently supported by the MCP group.
            if (isRun3()) {
                if(m_quality!=0 && m_quality!=1 && m_quality!=2 && m_quality!=4 && m_quality!=5) {
                    ATH_MSG_WARNING("MuonSelectionTool currently supports Loose, Medium, Tight, HighPt, and LowPtEfficiency WPs for Run3; all other WPs can only be used in ExpertDevelopMode mode");
                }
                
                if(m_quality==0 && !m_developMode && (m_excludeNSWFromPrecisionLayers || !m_recalcPrecisionLayerswNSW)) {
                    ATH_MSG_WARNING("For Run3, Tight WP is supported only when ExcludeNSWFromPrecisionLayers=False and RecalcPrecisionLayerswNSW=True");
                }
            }

            checkDone = true;
        }
    }

    //============================================================================
    asg::AcceptData MuonSelectionTool::accept(const xAOD::Muon& mu) const {
        SG::ReadHandle<xAOD::EventInfo> ei(m_eventInfo);
        return accept(columnar::MuonId(mu), columnar::EventInfoId(*ei));
    }

    asg::AcceptData MuonSelectionTool::accept(columnar::MuonId mu, columnar::EventInfoId event) const {
        // Verbose information
        ATH_MSG_VERBOSE("-----------------------------------");
        ATH_MSG_VERBOSE("New muon passed to accept function:");
        if (m_muonTypeAcc(mu) == xAOD::Muon::Combined)
            ATH_MSG_VERBOSE("Muon type: combined");
        else if (m_muonTypeAcc(mu) == xAOD::Muon::MuonStandAlone)
            ATH_MSG_VERBOSE("Muon type: stand-alone");
        else if (m_muonTypeAcc(mu) == xAOD::Muon::SegmentTagged)
            ATH_MSG_VERBOSE("Muon type: segment-tagged");
        else if (m_muonTypeAcc(mu) == xAOD::Muon::CaloTagged)
            ATH_MSG_VERBOSE("Muon type: calorimeter-tagged");
        else if (m_muonTypeAcc(mu) == xAOD::Muon::SiliconAssociatedForwardMuon)
            ATH_MSG_VERBOSE("Muon type: silicon-associated forward");
        ATH_MSG_VERBOSE("Muon pT [GeV]: " << m_ptAcc(mu) * MeVtoGeV);
        ATH_MSG_VERBOSE("Muon eta: "      << m_etaAcc(mu));
        ATH_MSG_VERBOSE("Muon phi: "      << m_phiAcc(mu));

        checkSanity();

        asg::AcceptData acceptData(&m_acceptInfo);

        // Do the eta cut:
        if (std::abs(m_etaAcc(mu)) >= m_maxEta) {
            ATH_MSG_VERBOSE("Failed eta cut");
            return acceptData;
        }
        acceptData.setCutResult("Eta", true);

        // Passes ID hit cuts
        bool passIDCuts = passedIDCuts(mu);
        ATH_MSG_VERBOSE("Passes ID Hit cuts " << passIDCuts);
        acceptData.setCutResult("IDHits", passIDCuts);

        // passes muon preselection
        bool passMuonCuts = passedMuonCuts(mu);
        ATH_MSG_VERBOSE("Passes preselection cuts " << passMuonCuts);
        acceptData.setCutResult("Preselection", passMuonCuts);

        if (!passIDCuts || !passMuonCuts) { return acceptData; }

        // Passes quality requirements
        xAOD::Muon::Quality thisMu_quality = getQuality(mu);
        bool thisMu_highpt = passedHighPtCuts(mu);
        bool thisMu_lowptE = passedLowPtEfficiencyCuts(mu, thisMu_quality, event);
        ATH_MSG_VERBOSE("Summary of quality information for this muon: ");
        ATH_MSG_VERBOSE("Muon quality: " << thisMu_quality << " passes HighPt: " << thisMu_highpt
                                         << " passes LowPtEfficiency: " << thisMu_lowptE);
        if (m_quality < 4 && thisMu_quality > m_quality) { return acceptData; }
        if (m_quality == 4 && !thisMu_highpt) { return acceptData; }
        if (m_quality == 5 && !thisMu_lowptE) { return acceptData; }
        acceptData.setCutResult("Quality", true);
        // Return the result:
        return acceptData;
    }

    void MuonSelectionTool::setQuality(xAOD::Muon& mu) const {
        mu.setQuality(getQuality(mu));
        return;
    }
    void MuonSelectionTool::IdMsPt(const xAOD::Muon& mu, float& idPt, float& mePt) const {
        IdMsPt(columnar::MuonId(mu), idPt, mePt);
    }

    void MuonSelectionTool::IdMsPt(columnar::MuonId mu, float& idPt, float& mePt) const {
        auto idtrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_idTrackLinkAcc).opt_value());
        auto metrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_meTrackLinkAcc));
        if (!idtrack || !metrack) { idPt = mePt = -1.; return; }
        if (m_turnOffMomCorr) {
            idPt = m_trkMomentumAcc.pt(*idtrack, 0.);
            mePt = m_trkMomentumAcc.pt(*metrack, 0.);
        } else {
            if (!m_idPtAcc.isAvailable(mu) || !m_mePtAcc.isAvailable(mu)) {
                ATH_MSG_FATAL("The muon with pT " << m_ptAcc(mu) * MeVtoGeV << " eta: " << m_etaAcc(mu)
                    << ", phi:" << m_phiAcc(mu) << " q:" << m_trkChargeAcc(*idtrack)
                    << ", author:" << m_authorAcc(mu)
                    << " is not decorated with calibrated momenta. Please fix");
                throw std::runtime_error("MuonSelectionTool() - qOverP significance calculation failed");
            }
            idPt = m_idPtAcc(mu);
            mePt = m_mePtAcc(mu);
        }
    }

    float MuonSelectionTool::qOverPsignificance(const xAOD::Muon& muon) const {
        return qOverPsignificance(columnar::MuonId(muon));
    }
    float MuonSelectionTool::qOverPsignificance(columnar::MuonId mu) const {
        // Avoid spurious FPEs in the clang build.
        CXXUTILS_TRAPPING_FP;

        if (m_disablePtCuts) {
            ATH_MSG_VERBOSE(__FILE__ << ":" << __LINE__
                                     << " Momentum dependent cuts are disabled. Return 0.");
            return 0.;
        }
        auto idtrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_idTrackLinkAcc).opt_value());
        auto metrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_meTrackLinkAcc));
        if (!idtrack || !metrack) {
            ATH_MSG_VERBOSE("No ID / MS track. Return dummy large value of 1 mio");
            return 1.e6;
        }
        float mePt{-1.}, idPt{-1.};
        IdMsPt(mu, idPt, mePt);

        const float meP = mePt / std::sin(m_trkThetaAcc(*metrack));
        const float idP = idPt / std::sin(m_trkThetaAcc(*idtrack));

        const float qOverPsigma = std::sqrt(m_trkCovAcc(*idtrack)(4, 4) + m_trkCovAcc(*metrack)(4, 4));
        return std::abs((m_trkChargeAcc(*metrack) / meP) - (m_trkChargeAcc(*idtrack) / idP)) / qOverPsigma;
    }

    float MuonSelectionTool::rhoPrime(const xAOD::Muon& muon) const {
        return rhoPrime(columnar::MuonId(muon));
    }

    float MuonSelectionTool::rhoPrime(columnar::MuonId mu) const {
        if (m_disablePtCuts) {
            ATH_MSG_VERBOSE(__FILE__ << ":" << __LINE__
                                     << " Momentum dependent cuts are disabled. Return 0.");
            return 0.f;
        }
        float mePt{-1.f}, idPt{-1.f};
        IdMsPt(mu, idPt, mePt);
        return std::abs(idPt - mePt) / m_ptAcc(mu);
    }

    xAOD::Muon::Quality MuonSelectionTool::getQuality(const xAOD::Muon& mu) const {
        return getQuality(columnar::MuonId(mu));
    }

    xAOD::Muon::Quality MuonSelectionTool::getQuality(columnar::MuonId mu) const {
        ATH_MSG_VERBOSE("Evaluating muon quality...");

        const auto  muonType = m_muonTypeAcc(mu);
        const float eta      = m_etaAcc(mu);
        const auto  author   = m_authorAcc(mu);

        // SegmentTagged muons
        if (muonType == xAOD::Muon::SegmentTagged) {
            ATH_MSG_VERBOSE("Muon is segment-tagged");
            if (std::abs(eta) < 0.1) {
                ATH_MSG_VERBOSE("Muon is loose");
                return xAOD::Muon::Loose;
            }
            ATH_MSG_VERBOSE("Do not allow segment-tagged muon at |eta| > 0.1 - return VeryLoose");
            return xAOD::Muon::VeryLoose;
        }

        // CaloTagged muons
        if (muonType == xAOD::Muon::CaloTagged) {
            ATH_MSG_VERBOSE("Muon is calorimeter-tagged");
            if (std::abs(eta) < 0.1 && passedCaloTagQuality(mu)) {
                ATH_MSG_VERBOSE("Muon is loose");
                return xAOD::Muon::Loose;
            }
        }

        // MS hit summary (fully columnar)
        hitSummary summary{};
        fillSummary(mu, summary);

        // Combined muons
        if (muonType == xAOD::Muon::Combined) {
            ATH_MSG_VERBOSE("Muon is combined");
            if (author == xAOD::Muon::STACO) {
                ATH_MSG_VERBOSE("Muon is STACO - return VeryLoose");
                return xAOD::Muon::VeryLoose;
            }

            // rejection muons with out-of-bounds hits
            const uint8_t combinedTrackOutBoundsPrecisionHits = m_combinedTrackOutBoundsHitsAcc(mu);
            if (combinedTrackOutBoundsPrecisionHits > 0) {
                ATH_MSG_VERBOSE("Muon has out-of-bounds precision hits - return VeryLoose");
                return xAOD::Muon::VeryLoose;
            }

            // LOOSE / MEDIUM / TIGHT WP
            auto idtrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_idTrackLinkAcc).opt_value());
            auto metrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_meTrackLinkAcc));
            if (idtrack && metrack && m_trkCovAcc(*metrack)(4, 4) > 0) {
                const float qOverPsignif = qOverPsignificance(mu);
                const float rho          = rhoPrime(mu);
                auto cbLink = mu(m_cbTrackLinkAcc);
                const float reducedChi2  = cbLink
                    ? m_trkChi2Acc(*cbLink) / m_trkNDoFAcc(*cbLink)
                    : 0.f;

                ATH_MSG_VERBOSE("Relevant cut variables:");
                ATH_MSG_VERBOSE("number of precision layers = " << (int)summary.nprecisionLayers);
                ATH_MSG_VERBOSE("reduced Chi2 = " << reducedChi2);
                ATH_MSG_VERBOSE("qOverP significance = " << qOverPsignif);

                // NEW TIGHT WP
                if (summary.nprecisionLayers > 1 && reducedChi2 < 8 && std::abs(qOverPsignif) < 7) {
                    if (passTight(mu, rho, qOverPsignif)) {
                        ATH_MSG_VERBOSE("Muon is tight");
                        return xAOD::Muon::Tight;
                    }
                }
                ATH_MSG_VERBOSE("Muon did not pass requirements for tight combined muon");

                // MEDIUM WP
                if ((std::abs(qOverPsignif) < 7 || m_toroidOff) &&
                    (summary.nprecisionLayers > 1 ||
                     (summary.nprecisionLayers == 1 && summary.nprecisionHoleLayers < 2 && std::abs(eta) < 0.1))) {
                    ATH_MSG_VERBOSE("Muon is medium");
                    return xAOD::Muon::Medium;
                }
                ATH_MSG_VERBOSE("Muon did not pass requirements for medium combined muon");
            } else {
                // CB muons with missing ID or ME track
                ATH_MSG_VERBOSE("Muon is missing the ID and/or ME tracks...");
                if (summary.nprecisionLayers > 1 ||
                    (summary.nprecisionLayers == 1 && summary.nprecisionHoleLayers < 2 && std::abs(eta) < 0.1)) {
                    // In toroid-off data ME/MS tracks often missing - need special treatment  => flagging as "Medium"
                    // In toroid-on data ME/MS tracks missing only for <1% of CB muons, mostly MuGirl (to be fixed) => flagging as "Loose"
                    if (m_toroidOff) {
                        ATH_MSG_VERBOSE("...this is toroid-off data - returning medium");
                        return xAOD::Muon::Medium;
                    }
                    ATH_MSG_VERBOSE("...this is not toroid-off data - returning loose");
                    return xAOD::Muon::Loose;
                }
            }

            // Improvement for Loose targeting low-pT muons (pt<7 GeV)
            const float pt = m_ptAcc(mu);
            if ((m_disablePtCuts || pt * MeVtoGeV < 7.) && std::abs(eta) < 1.3 &&
                summary.nprecisionLayers > 0 &&
                (author == xAOD::Muon::MuGirl && ((m_allAuthorsAcc(mu) >> xAOD::Muon::MuTagIMO) & 1))) {
                ATH_MSG_VERBOSE("Muon passed selection for loose working point at low pT");
                return xAOD::Muon::Loose;
            }

            // didn't pass the set of requirements for a medium or tight combined muon
            ATH_MSG_VERBOSE("Did not pass selections for combined muon - returning VeryLoose");
            return xAOD::Muon::VeryLoose;
        }

        // SA muons
        if (author == xAOD::Muon::MuidSA) {
            ATH_MSG_VERBOSE("Muon is stand-alone");

            if (std::abs(eta) > 2.5) {
                ATH_MSG_VERBOSE("number of precision layers = " << (int)summary.nprecisionLayers);

                // 3 station requirement for medium
                if (summary.nprecisionLayers > 2 && !m_toroidOff) {
                    ATH_MSG_VERBOSE("Muon is medium");
                    return xAOD::Muon::Medium;
                }
            }

            // didn't pass the set of requirements for a medium SA muon
            ATH_MSG_VERBOSE("Muon did not pass selection for medium stand-alone muon - return VeryLoose");
            return xAOD::Muon::VeryLoose;
        }

        // SiliconAssociatedForward (SAF) muons
        if (muonType == xAOD::Muon::SiliconAssociatedForwardMuon) {
            ATH_MSG_VERBOSE("Muon is silicon-associated forward muon");

            auto cbtrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_cbTrackLinkAcc));
            auto metrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_meTrackLinkAcc));
            if (cbtrack && metrack) {
                const float cbEta = m_trkMomentumAcc.eta(*cbtrack, 0.);
                if (std::abs(cbEta) > 2.5) {
                    ATH_MSG_VERBOSE("number of precision layers = " << (int)summary.nprecisionLayers);
                    if (summary.nprecisionLayers > 2 && !m_toroidOff) {
                        if constexpr (columnar::ColumnarModeDefault::isXAOD) {
                            const xAOD::Muon& xmu = mu.getXAODObject();
                            if (xmu.trackParticle(xAOD::Muon::Primary) == xmu.trackParticle(xAOD::Muon::InnerDetectorTrackParticle) &&
                                !m_developMode) {
                                ATH_MSG_FATAL(
                                    "SiliconForwardAssociated muon has ID track as primary track particle. "
                                    << "This is a bug fixed starting with xAODMuon-00-17-07, which should be present in this release. "
                                    << "Please report this to the Muon CP group!");
                            }
                        }
                        ATH_MSG_VERBOSE("Muon is medium");
                        return xAOD::Muon::Medium;
                    }
                }
            }

            // didn't pass the set of requirements for a medium SAF muon
            ATH_MSG_VERBOSE("Muon did not pass selection for medium silicon-associated forward muon - return VeryLoose");
            return xAOD::Muon::VeryLoose;
        }

        ATH_MSG_VERBOSE("Muon did not pass selection for loose/medium/tight for any muon type - return VeryLoose");
        return xAOD::Muon::VeryLoose;
    }

    void MuonSelectionTool::setPassesIDCuts(xAOD::Muon& mu) const { mu.setPassesIDCuts(passedIDCuts(mu)); }

    bool MuonSelectionTool::passedIDCuts(const xAOD::Muon& mu) const {
        if (m_useLRT) {
            // LRT handling accesses the ID track's patternRecoInfo — not yet migrated to columnar
            static const SG::AuxElement::Accessor<char> isLRTmuon("isLRT");
            if (isLRTmuon.isAvailable(mu)) {
                if (isLRTmuon(mu)) return true; /// No ID cuts should be applied on LRT muons, so always set this flag to true.
            } else { /// If the isLRT decor is not available, try to see if patternRecoInfo is available for the corresponding ID track.
                static const SG::AuxElement::Accessor<uint64_t> patternAcc("patternRecoInfo");
                const xAOD::TrackParticle* idtrack = mu.trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
                if (idtrack) { /// All LRT muons should have ID tracks. The muons without ID tracks have to come from the standard muon container.
                    if (!patternAcc.isAvailable(*idtrack)) {
                        ATH_MSG_FATAL("No information available to tell if the muon is LRT or standard. Either run MuonLRTMergingAlg to decorate with `isLRT` flag, or supply the patternRecoInfo for the original ID track.");
                        throw std::runtime_error("MuonSelectionTool() - isLRT decor and patternRecoInfo both unavailable for a muon.");
                    }
                    std::bitset<xAOD::NumberOfTrackRecoInfo> patternBitSet(patternAcc(*idtrack));
                    if (patternBitSet.test(xAOD::SiSpacePointsSeedMaker_LargeD0)) return true;
                }
            }
        }
        // Delegate the main ID-cut logic to the columnar overload
        return passedIDCuts(columnar::MuonId(mu));
    }

    bool MuonSelectionTool::passedIDCuts(columnar::MuonId mu) const {
        // Note: LRT handling lives in the xAOD overload; this function assumes
        // the muon is already known to be non-LRT (or m_useLRT is false).
        const auto  author   = m_authorAcc(mu);
        const float eta      = m_etaAcc(mu);
        const auto  muonType = m_muonTypeAcc(mu);

        // SA muons beyond the ID acceptance do not require ID hits
        if (author == xAOD::Muon::MuidSA && std::abs(eta) > 2.5) return true;

        if (muonType == xAOD::Muon::SiliconAssociatedForwardMuon) {
            auto cbtrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_cbTrackLinkAcc));
            if (cbtrack && std::abs(m_trkMomentumAcc.eta(*cbtrack, 0.)) > 2.5) return true;
            return false;
        }

        // Read ID-track hit counts via the inDetTrackParticleLink, which points to
        // InDetTrackParticles (central) or InDetForwardTrackParticles (forward).
        // SiliconAssociatedForwardMuon and out-of-acceptance MuidSA cases already
        // returned above, so all remaining muons carry a valid central ID-track link.
        auto idtrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_idTrackLinkAcc).opt_value());
        if (!idtrack) return false;
        return passedIDCuts(*idtrack);
    }

    bool MuonSelectionTool::isBadMuon(const xAOD::Muon& mu) const {
        return isBadMuon(columnar::MuonId(mu));
    }

    bool MuonSelectionTool::isBadMuon(columnar::MuonId mu) const {
        if (m_muonTypeAcc(mu) != xAOD::Muon::Combined) return false;
        // ::
        const auto idtrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_idTrackLinkAcc).opt_value());
        const auto metrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_meTrackLinkAcc));
        const auto cbLink  = mu(m_cbTrackLinkAcc);
        const auto cbtrack = columnar::OptObjectId<columnar::MuonTrackDef>(cbLink);
        // ::
        // Some spurious muons are found to have negative ME track fit covariance, and are typically poorly reconstructed
        if (metrack && m_trkCovAcc(*metrack)(4, 4) < 0.0) return true;
        // ::
        if (idtrack && metrack && cbtrack) {
            // ::
            const double qOverP_ID    = m_trkQOverPAcc(*idtrack);
            const double qOverPerr_ID = std::sqrt(m_trkCovAcc(*idtrack)(4, 4));
            const double qOverP_ME    = m_trkQOverPAcc(*metrack);
            const double qOverPerr_ME = std::sqrt(m_trkCovAcc(*metrack)(4, 4));
            const double qOverP_CB    = m_qOverPCBTrackAcc(*cbLink);
            const double qOverPerr_CB = std::sqrt(m_trkCovAcc(*cbtrack)(4, 4));
            // ::
            if (m_quality == 4) {
                // recipe for high-pt selection
                bool IsBadMuon = !passedErrorCutCB(mu);

                hitSummary summary{};
                fillSummary(mu, summary);

                // temporarily apply same recipe as for other working points in addition to CB error
                // cut for 2-station muons, pending better treatment of ID/MS misalignments
                if (m_use2stationMuonsHighPt && summary.nprecisionLayers == 2) {
                    const double IdCbRatio = std::abs((qOverPerr_ID / qOverP_ID) / (qOverPerr_CB / qOverP_CB));
                    const double MeCbRatio = std::abs((qOverPerr_ME / qOverP_ME) / (qOverPerr_CB / qOverP_CB));
                    IsBadMuon = (IdCbRatio < 0.8 || MeCbRatio < 0.8 || IsBadMuon);
                }
                return IsBadMuon;
            } else {
                // recipe for other WP
                const double IdCbRatio = std::abs((qOverPerr_ID / qOverP_ID) / (qOverPerr_CB / qOverP_CB));
                const double MeCbRatio = std::abs((qOverPerr_ME / qOverP_ME) / (qOverPerr_CB / qOverP_CB));
                return (IdCbRatio < 0.8 || MeCbRatio < 0.8);
            }
        } else {
            return true;
        }
    }

    bool MuonSelectionTool::passedLowPtEfficiencyCuts(const xAOD::Muon& mu) const {
        xAOD::Muon::Quality thisMu_quality = getQuality(mu);
        return passedLowPtEfficiencyCuts(mu, thisMu_quality);
    }

    bool MuonSelectionTool::passedLowPtEfficiencyCuts(const xAOD::Muon& mu, xAOD::Muon::Quality thisMu_quality) const {
        SG::ReadHandle<xAOD::EventInfo> ei(m_eventInfo);
        return passedLowPtEfficiencyCuts(columnar::MuonId(mu), thisMu_quality, columnar::EventInfoId(*ei));
    }

    bool MuonSelectionTool::passedLowPtEfficiencyCuts(columnar::MuonId mu,
                                                       xAOD::Muon::Quality thisMu_quality,
                                                       columnar::EventInfoId event) const {
        ATH_MSG_VERBOSE("Checking whether muon passes low-pT selection...");

        if (!m_useAllAuthors) {  // no allAuthors, always fail the WP
            ATH_MSG_VERBOSE("Do not have allAuthors variable - fail low-pT");
            return false;
        }

        // requiring combined muons, unless segment-tags are included
        const auto muonType = m_muonTypeAcc(mu);
        if (!m_useSegmentTaggedLowPt) {
            if (muonType != xAOD::Muon::Combined) {
                ATH_MSG_VERBOSE("Muon is not combined - fail low-pT");
                return false;
            }
        } else {
            if (muonType != xAOD::Muon::Combined && muonType != xAOD::Muon::SegmentTagged) {
                ATH_MSG_VERBOSE("Muon is not combined or segment-tagged - fail low-pT");
                return false;
            }
        }

        // author check
        const auto author = m_authorAcc(mu);
        if (!m_useSegmentTaggedLowPt) {
            if (author != xAOD::Muon::MuGirl && author != xAOD::Muon::MuidCo) {
                ATH_MSG_VERBOSE("Muon is neither MuGirl nor MuidCo - fail low-pT");
                return false;
            }
        } else {
            if (author != xAOD::Muon::MuGirl && author != xAOD::Muon::MuidCo &&
                author != xAOD::Muon::MuTagIMO) {
                ATH_MSG_VERBOSE("Muon is neither MuGirl / MuidCo / MuTagIMO - fail low-pT");
                return false;
            }
        }

        // applying Medium selection above pT = 18 GeV
        if (m_ptAcc(mu) * MeVtoGeV > 18.) {
            ATH_MSG_VERBOSE("pT > 18 GeV - apply medium selection");
            if (thisMu_quality <= xAOD::Muon::Medium) {
                ATH_MSG_VERBOSE("Muon passed low-pT selection");
                return true;
            } else {
                ATH_MSG_VERBOSE("Muon failed low-pT selection");
                return false;
            }
        }

        // requiring Medium in forward regions
        if (!m_useMVALowPt && std::abs(m_etaAcc(mu)) > 1.55 && thisMu_quality > xAOD::Muon::Medium) {
            ATH_MSG_VERBOSE("Not using MVA selection, failing low-pT selection due to medium requirement in forward region");
            return false;
        }

        // rejection of muons with out-of-bounds hits
        if (m_combinedTrackOutBoundsHitsAcc(mu) > 0) {
            ATH_MSG_VERBOSE("Muon has out-of-bounds precision hits - fail low-pT");
            return false;
        }

        // requiring explicitely >=1 station (2 in the |eta|>1.3 region when Medium selection is not explicitely required)
        if (muonType == xAOD::Muon::Combined) {
            hitSummary summary{};
            fillSummary(mu, summary);
            uint nStationsCut = (std::abs(m_etaAcc(mu)) > 1.3 && std::abs(m_etaAcc(mu)) < 1.55) ? 2 : 1;
            if (summary.nprecisionLayers < nStationsCut) {
                ATH_MSG_VERBOSE("number of precision layers = " << (int)summary.nprecisionLayers
                                << " is lower than cut value " << nStationsCut << " - fail low-pT");
                return false;
            }
        }

        // reject MuGirl muon if not found also by MuTagIMO
        if (m_useAllAuthors) {
            if (author == xAOD::Muon::MuGirl &&
                !((m_allAuthorsAcc(mu) >> xAOD::Muon::MuTagIMO) & 1)) {
                ATH_MSG_VERBOSE("MuGirl muon is not confirmed by MuTagIMO - fail low-pT");
                return false;
            }
        } else
            return false;

        if (m_useMVALowPt) {
            if (isRun3()) {
                ATH_MSG_VERBOSE("Applying Run-3 MVA-based selection");
                return passedLowPtEfficiencyMVACutRun3(mu);
            } else {
                ATH_MSG_VERBOSE("Applying Run-2 MVA-based selection");
                return passedLowPtEfficiencyMVACut(mu, event);
            }
        }

        ATH_MSG_VERBOSE("Applying cut-based selection");

        // apply some loose quality requirements
        float momentumBalanceSignificance{0.}, scatteringCurvatureSignificance{0.}, scatteringNeighbourSignificance{0.};

        if (m_momentumBalanceSigAcc.isAvailable(mu))
            momentumBalanceSignificance = m_momentumBalanceSigAcc(mu);
        if (m_scatteringCurvatureSigAcc.isAvailable(mu))
            scatteringCurvatureSignificance = m_scatteringCurvatureSigAcc(mu);
        if (m_scatteringNeighbourSigAcc.isAvailable(mu))
            scatteringNeighbourSignificance = m_scatteringNeighbourSigAcc(mu);

        ATH_MSG_VERBOSE("momentum balance significance: " << momentumBalanceSignificance);
        ATH_MSG_VERBOSE("scattering curvature significance: " << scatteringCurvatureSignificance);
        ATH_MSG_VERBOSE("scattering neighbour significance: " << scatteringNeighbourSignificance);

        if (std::abs(momentumBalanceSignificance) > 3. || std::abs(scatteringCurvatureSignificance) > 3. ||
            std::abs(scatteringNeighbourSignificance) > 3.) {
            ATH_MSG_VERBOSE("Muon failed cut-based low-pT selection");
            return false;
        }

        // passed low pt selection
        ATH_MSG_VERBOSE("Muon passed cut-based low-pT selection");
        return true;
    }

    std::vector<const xAOD::MuonSegment*> MuonSelectionTool::getSegmentsSorted(const xAOD::Muon& mu) const {
        std::vector<const xAOD::MuonSegment*> segments_sorted;
        segments_sorted.reserve(mu.nMuonSegments());

        for (unsigned int i = 0; i < mu.nMuonSegments(); i++) {
            if (!mu.muonSegmentLink(i).isValid()) continue;  // thinned in derived formats (e.g. PHYSLITE)
            const xAOD::MuonSegment* seg = mu.muonSegment(i);
            if (!seg)
                ATH_MSG_WARNING("The muon reports more segments than are available. Please report this to the muon software community!");
            else
                segments_sorted.push_back(seg);
        }

        std::sort(segments_sorted.begin(), segments_sorted.end(), chamberIndexCompare);

        return segments_sorted;
    }

    bool MuonSelectionTool::passedLowPtEfficiencyMVACut(const xAOD::Muon& mu) const {
        SG::ReadHandle<xAOD::EventInfo> ei(m_eventInfo);
        return passedLowPtEfficiencyMVACut(columnar::MuonId(mu), columnar::EventInfoId(*ei));
    }

    bool MuonSelectionTool::passedLowPtEfficiencyMVACutRun3(const xAOD::Muon& mu) const {
        return passedLowPtEfficiencyMVACutRun3(columnar::MuonId(mu));
    }

    bool MuonSelectionTool::passedLowPtEfficiencyMVACut(columnar::MuonId mu, columnar::EventInfoId event) const {
        //LowPt Not supported in run3 for the time being
        if (isRun3() && !m_developMode) {
            ATH_MSG_VERBOSE("LowPt WP currently not supported for run3 if not in expert mode");
            return false;
        }
        if (!m_useMVALowPt) {
            ATH_MSG_DEBUG("Low pt MVA disabled. Return... ");
            return false;
        }
        // Read event number for LowPt MVA even/odd reader selection
        const unsigned long long eventNumber = (m_expertMode_EvtNumber.value() != 0)
            ? m_expertMode_EvtNumber.value()
            : static_cast<unsigned long long>(m_eventNumberAcc(event));
        std::lock_guard<std::mutex> guard(m_low_pt_mva_mutex);
        // set values for all BDT input variables from the muon in question
        float momentumBalanceSig{-1}, CurvatureSig{-1}, energyLoss{-1}, muonSegmentDeltaEta{-1}, scatteringNeigbour{-1};
        momentumBalanceSig  = m_momentumBalanceSigAcc(mu);
        CurvatureSig        = m_scatteringCurvatureSigAcc(mu);
        scatteringNeigbour  = m_scatteringNeighbourSigAcc(mu);
        energyLoss          = m_energyLossAcc(mu);
        muonSegmentDeltaEta = m_segmentDeltaEtaAcc(mu);

        uint8_t middleSmallHoles{0}, middleLargeHoles{0};
        middleSmallHoles = m_middleSmallHolesAcc(mu);
        middleLargeHoles = m_middleLargeHolesAcc(mu);
        const float middleHoles = middleSmallHoles + middleLargeHoles;

        const auto author = m_authorAcc(mu);

        // Segment-derived quantities. In xAOD mode, walk the segment links directly;
        // segment objects are thinned in PHYSLITE so getSegmentsSorted() returns empty
        // there, and the defaults below apply. In columnar mode (PHYSLITE only) segment
        // links are not available in the columnar framework, so defaults are always used.
        float seg1ChamberIdx{-9.f}, seg2ChamberIdx{-9.f};
        float seg1NPrecisionHits{-1.f}, seg1GlobalR{0.f}, seg1Chi2OverDoF{-1.f};

        if constexpr (columnar::ColumnarModeDefault::isXAOD) {
            const xAOD::Muon& xmu = mu.getXAODObject();
            const std::vector<const xAOD::MuonSegment*> muonSegments = getSegmentsSorted(xmu);
            using namespace Muon::MuonStationIndex;
            seg1ChamberIdx = !muonSegments.empty()   ? static_cast<float>(toInt(muonSegments[0]->chamberIndex())) : -9.f;
            seg2ChamberIdx = muonSegments.size() > 1 ? static_cast<float>(toInt(muonSegments[1]->chamberIndex())) : -9.f;
            if (author == xAOD::Muon::MuTagIMO) {
                seg1NPrecisionHits = !muonSegments.empty() ? static_cast<float>(muonSegments[0]->nPrecisionHits()) : -1.f;
                seg1GlobalR        = !muonSegments.empty()
                                     ? std::hypot(muonSegments[0]->x(), muonSegments[0]->y(), muonSegments[0]->z())
                                     : 0.f;
                seg1Chi2OverDoF    = !muonSegments.empty()
                                     ? muonSegments[0]->chiSquared() / muonSegments[0]->numberDoF()
                                     : -1.f;
            }
        }

        if (author == xAOD::Muon::MuTagIMO && seg1ChamberIdx == -9.f)
            ATH_MSG_WARNING("passedLowPtEfficiencyMVACut - found segment-tagged muon with no segments!");

        // variables for the BDT
        std::vector<float> var_vector;
        if (author == xAOD::Muon::MuidCo || author == xAOD::Muon::MuGirl) {
            var_vector = {momentumBalanceSig, CurvatureSig,        scatteringNeigbour, energyLoss,
                          middleHoles,        muonSegmentDeltaEta, seg1ChamberIdx,     seg2ChamberIdx};
        } else {
            if (std::abs(m_etaAcc(mu)) >= 1.3)
                var_vector = {seg2ChamberIdx, seg1ChamberIdx,  seg1NPrecisionHits,    muonSegmentDeltaEta,
                              seg1GlobalR,    seg1Chi2OverDoF, std::abs(CurvatureSig)};
            else
                var_vector = {seg1ChamberIdx, seg1NPrecisionHits, muonSegmentDeltaEta,
                              seg1GlobalR,    seg1Chi2OverDoF,    std::abs(CurvatureSig)};
        }

        // use different trainings for even/odd numbered events
        TMVA::Reader *reader_MUID, *reader_MUGIRL;
        if (eventNumber % 2 == 1) {
            reader_MUID = m_readerE_MUID.get();
            reader_MUGIRL = m_readerE_MUGIRL.get();
        } else {
            reader_MUID = m_readerO_MUID.get();
            reader_MUGIRL = m_readerO_MUGIRL.get();
        }

        // BDT for MuTagIMO is binned in |eta|
        TMVA::Reader* reader_MUTAGIMO;
        if (std::abs(m_etaAcc(mu)) < 0.7)
            reader_MUTAGIMO = m_reader_MUTAGIMO_etaBin1.get();
        else if (std::abs(m_etaAcc(mu)) < 1.3)
            reader_MUTAGIMO = m_reader_MUTAGIMO_etaBin2.get();
        else
            reader_MUTAGIMO = m_reader_MUTAGIMO_etaBin3.get();

        // get the BDT discriminant response
        float BDTdiscriminant;

        if (author == xAOD::Muon::MuidCo)
            BDTdiscriminant = reader_MUID->EvaluateMVA(var_vector, "BDTG");
        else if (author == xAOD::Muon::MuGirl)
            BDTdiscriminant = reader_MUGIRL->EvaluateMVA(var_vector, "BDTG");
        else if (author == xAOD::Muon::MuTagIMO && m_useSegmentTaggedLowPt)
            BDTdiscriminant = reader_MUTAGIMO->EvaluateMVA(var_vector, "BDT");
        else {
            ATH_MSG_WARNING("Invalid author for low-pT MVA, failing selection...");
            return false;
        }

        // cut on dicriminant
        float BDTcut = (author == xAOD::Muon::MuTagIMO) ? 0.12 : -0.6;

        if (BDTdiscriminant > BDTcut) {
            ATH_MSG_VERBOSE("Passed low-pT MVA cut");
            return true;
        } else {
            ATH_MSG_VERBOSE("Failed low-pT MVA cut");
            return false;
        }
    }

    bool MuonSelectionTool::passedLowPtEfficiencyMVACutRun3(columnar::MuonId mu) const {
        if (!m_useMVALowPt) {
            ATH_MSG_DEBUG("Low pt MVA disabled. Return... ");
            return false;
        }

        auto cbLink  = mu(m_cbTrackLinkAcc);
        auto cbtrack = columnar::OptObjectId<columnar::MuonTrackDef>(cbLink);
        auto idtrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_idTrackLinkAcc).opt_value());
        auto metrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_meTrackLinkAcc));

        if (!cbtrack || !idtrack || !metrack) {
            ATH_MSG_VERBOSE("Missing primary, ID, or extrapolated MS track for Run-3 low-pT MVA; failing selection");
            return false;
        }

        const auto author = m_authorAcc(mu);

        if (author == xAOD::Muon::MuidCo) {
            ATH_MSG_VERBOSE("passedLowPtEfficiencyMVACutRun3() for MuidCO");

            //-- Prepare BDT input feature variables
            float momentumBalanceSig{-1}, CurvatureSig{-1}, scatteringNeigbour{-1};
            int CaloMuonIDTag{-1};
            uint8_t nPixelHits{0}, nTRTOutliers{0};
            float reducedChi2{-1};
            float etaBalanceSig{-1}, phiBalanceSig{-1};
            float seg1ChamberIdx{-9.f};

            momentumBalanceSig = m_momentumBalanceSigAcc(mu);
            CurvatureSig       = m_scatteringCurvatureSigAcc(mu);
            scatteringNeigbour = m_scatteringNeighbourSigAcc(mu);

            CaloMuonIDTag = m_caloMuonIDTagAcc(mu);

            // reducedChi2, nPixelHits, nTRTOutliers: all from the combined track particle
            reducedChi2  = m_trkChi2Acc(*cbLink) / m_trkNDoFAcc(*cbLink);
            nPixelHits   = m_nPixelHitsCBTrackAcc(*cbLink);
            nTRTOutliers = m_nTRTOutliersCBTrackAcc(*cbLink);
            etaBalanceSig = std::abs(m_trkMomentumAcc.eta(*idtrack, 0.) - m_trkMomentumAcc.eta(*metrack, 0.));
            phiBalanceSig = std::abs(m_trkMomentumAcc.phi(*idtrack, 0.) - m_trkMomentumAcc.phi(*metrack, 0.));

            // Segment objects are thinned in PHYSLITE; getSegmentsSorted() returns empty there.
            // In columnar mode (PHYSLITE only) the default -9 is used.
            if constexpr (columnar::ColumnarModeDefault::isXAOD) {
                const xAOD::Muon& xmu = mu.getXAODObject();
                const std::vector<const xAOD::MuonSegment*> muonSegments = getSegmentsSorted(xmu);
                using namespace Muon::MuonStationIndex;
                seg1ChamberIdx = !muonSegments.empty() ? static_cast<float>(toInt(muonSegments[0]->chamberIndex())) : -9.f;
            }

            hitSummary summary{};
            fillSummary(mu, summary);

            //-- Apply clipping
            etaBalanceSig = std::min(etaBalanceSig, 1.0f);
            reducedChi2 = std::min(reducedChi2, 100.0f);
            CurvatureSig = std::clamp(CurvatureSig, -10.0f, 10.0f);
            scatteringNeigbour = std::clamp(scatteringNeigbour, -10.0f, 10.0f);

            int energyLossType = static_cast<int>(m_energyLossTypeAcc(mu));

            //-- Prepare BDT input feature variables vector
            std::vector<float> muidCO_feature_vector = {
                static_cast<float>(nPixelHits),
                static_cast<float>(nTRTOutliers),
                static_cast<float>(CaloMuonIDTag),
                static_cast<float>(energyLossType),
                etaBalanceSig,
                momentumBalanceSig,
                static_cast<float>(summary.nprecisionLayers),
                phiBalanceSig,
                reducedChi2,
                CurvatureSig,
                scatteringNeigbour,
                seg1ChamberIdx
            };

            //-- Apply scaling
            for (size_t j = 0; j < muidCO_feature_vector.size(); ++j)
                muidCO_feature_vector[j] = (muidCO_feature_vector[j] - m_lowPtMuidCO_means[j]) / m_lowPtMuidCO_scaler[j];

            //-- Get BDT response
            float bdt_score = m_MuidCO->GetClassification(muidCO_feature_vector);
            ATH_MSG_VERBOSE(" Low-pT MVA BDT score (MuidCo, Run-3) : " << bdt_score);

            //-- cut on discriminant (value motivated by study, slide 10):
            // https://indico.cern.ch/event/1639238/contributions/6896745/attachments/3208593/5714230/Run-3%20LowPt%20MVA%20WP%20Update.pdf
            float lowPtMVARun3_MuidCO_cut_value = 0.1300;
            return (bdt_score > lowPtMVARun3_MuidCO_cut_value);

        } else if (author == xAOD::Muon::MuGirl &&
                   ((m_allAuthorsAcc(mu) >> xAOD::Muon::MuTagIMO) & 1)) {
            ATH_MSG_VERBOSE("passedLowPtEfficiencyMVACutRun3() for MuGirl");

            //-- Prepare BDT input feature variables
            uint8_t nTRTOutliers{0}, middleClosePrecisionHits{0}, outerClosePrecisionHits{0};
            float momentumBalanceSig{-1}, seg1ChamberIdx{-9.f}, energyLoss{-1};
            int CaloMuonIDTag{-1};
            float reducedChi2{-1}, etaBalanceSig{-1}, phiBalanceSig{-1};
            float segmentDeltaEta{-1}, etaPrime{-1}, innerHits{-1};
            float middleHoles{-1}, nGoodPrecLayers{-1};
            float nprecisionHoleLayers{-1}, outerHoles{-1};

            momentumBalanceSig = m_momentumBalanceSigAcc(mu);
            segmentDeltaEta   = m_segmentDeltaEtaAcc(mu);
            energyLoss        = m_energyLossAcc(mu);

            uint8_t middleSmallHoles{0}, middleLargeHoles{0};
            middleSmallHoles = m_middleSmallHolesAcc(mu);
            middleLargeHoles = m_middleLargeHolesAcc(mu);
            uint8_t outerSmallHoles{0}, outerLargeHoles{0};
            outerSmallHoles  = m_outerSmallHolesAcc(mu);
            outerLargeHoles  = m_outerLargeHolesAcc(mu);
            nTRTOutliers = m_nTRTOutliersCBTrackAcc(*cbLink);
            middleClosePrecisionHits = m_middleClosePrecisionHitsAcc(mu);
            outerClosePrecisionHits  = m_outerClosePrecisionHitsAcc(mu);

            CaloMuonIDTag = m_caloMuonIDTagAcc(mu);

            middleHoles = middleSmallHoles + middleLargeHoles;
            outerHoles  = outerSmallHoles  + outerLargeHoles;
            reducedChi2   = m_trkChi2Acc(*cbLink) / m_trkNDoFAcc(*cbLink);
            etaBalanceSig = std::abs(m_trkMomentumAcc.eta(*idtrack, 0.) - m_trkMomentumAcc.eta(*metrack, 0.));
            etaPrime      = std::abs((m_trkMomentumAcc.eta(*idtrack, 0.) - m_trkMomentumAcc.eta(*metrack, 0.)) / m_etaAcc(mu));
            phiBalanceSig = std::abs(m_trkMomentumAcc.phi(*idtrack, 0.) - m_trkMomentumAcc.phi(*metrack, 0.));

            // Segment objects are thinned in PHYSLITE; getSegmentsSorted() returns empty there.
            // In columnar mode (PHYSLITE only) the default -9 is used.
            if constexpr (columnar::ColumnarModeDefault::isXAOD) {
                const xAOD::Muon& xmu = mu.getXAODObject();
                const std::vector<const xAOD::MuonSegment*> muonSegments = getSegmentsSorted(xmu);
                using namespace Muon::MuonStationIndex;
                seg1ChamberIdx = !muonSegments.empty() ? static_cast<float>(toInt(muonSegments[0]->chamberIndex())) : -9.f;
            }

            hitSummary summary{};
            fillSummary(mu, summary);

            innerHits              = summary.innerSmallHits + summary.innerLargeHits;
            nGoodPrecLayers        = summary.nGoodPrecLayers;
            nprecisionHoleLayers   = summary.nprecisionHoleLayers;

            //-- Apply clipping
            etaBalanceSig = std::min(etaBalanceSig, 1.0f);
            reducedChi2   = std::min(reducedChi2, 100.0f);
            etaPrime      = std::clamp(etaPrime, -10.0f, 10.0f);

            //-- Prepare BDT input feature variables vector
            std::vector<float> muGirl_feature_vector = {
                static_cast<float>(nTRTOutliers),
                static_cast<float>(CaloMuonIDTag),
                energyLoss,
                etaBalanceSig,
                etaPrime,
                innerHits,
                static_cast<float>(middleClosePrecisionHits),
                middleHoles,
                momentumBalanceSig,
                nGoodPrecLayers,
                nprecisionHoleLayers,
                static_cast<float>(summary.nprecisionLayers),
                static_cast<float>(outerClosePrecisionHits),
                outerHoles,
                phiBalanceSig,
                reducedChi2,
                seg1ChamberIdx,
                segmentDeltaEta
            };

            //-- Apply scaling
            for (size_t j = 0; j < muGirl_feature_vector.size(); ++j)
                muGirl_feature_vector[j] = (muGirl_feature_vector[j] - m_lowPtMuGirl_means[j]) / m_lowPtMuGirl_scaler[j];

            //-- Get BDT response
            float bdt_score = m_MuGirl->GetClassification(muGirl_feature_vector);
            ATH_MSG_VERBOSE(" Low-pT MVA BDT score (MuGirl, Run-3) : " << bdt_score);

            //-- cut on discriminant (value motivated by study, slide 11):
            // https://indico.cern.ch/event/1639238/contributions/6896745/attachments/3208593/5714230/Run-3%20LowPt%20MVA%20WP%20Update.pdf
            float lowPtMVARun3_MuGirl_cut_value = 0.1550;
            return (bdt_score > lowPtMVARun3_MuGirl_cut_value);
        } else {
            ATH_MSG_WARNING("Invalid author for low-pT MVA in Run-3, failing selection...");
            return false;
        }
    }

    bool MuonSelectionTool::passedHighPtCuts(const xAOD::Muon& mu) const {
        return passedHighPtCuts(columnar::MuonId(mu));
    }

    bool MuonSelectionTool::passedHighPtCuts(columnar::MuonId mu) const {
        ATH_MSG_VERBOSE("Checking whether muon passes high-pT selection...");

        // :: Request combined muons
        if (m_muonTypeAcc(mu) != xAOD::Muon::Combined) {
            ATH_MSG_VERBOSE("Muon is not combined - fail high-pT");
            return false;
        }
        if (m_authorAcc(mu) == xAOD::Muon::STACO) {
            ATH_MSG_VERBOSE("Muon is STACO - fail high-pT");
            return false;
        }

        // :: Reject muons with out-of-bounds hits
        if (m_combinedTrackOutBoundsHitsAcc(mu) > 0) {
            ATH_MSG_VERBOSE("Muon has out-of-bounds precision hits - fail high-pT");
            return false;
        }

        // :: Access MS hits information
        hitSummary summary{};
        fillSummary(mu, summary);

        ATH_MSG_VERBOSE("number of precision layers: " << (int)summary.nprecisionLayers);

        //::: Apply MS Chamber Vetoes
        // Given according to their eta-phi locations in the muon spectrometer
        // FORM: CHAMBERNAME[ array of four values ] = { eta 1, eta 2, phi 1, phi 2}
        // The vetoes are applied based on the MS track if available. If the MS track is not available,
        // the vetoes are applied according to the combined track, and runtime warning is printed to
        // the command line.
        // In columnar mode (PHYSLITE only) the MS track particle is thinned; use combined track.
        auto CB_track = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_cbTrackLinkAcc));
        if (!CB_track) {
            ATH_MSG_WARNING("passedHighPtCuts - CB track missing in muon! Failing High-pT selection...");
            return false;
        }
        float etaCB = m_trkMomentumAcc.eta(*CB_track, 0.);
        float etaMS = etaCB;
        float phiMS = m_trkMomentumAcc.phi(*CB_track, 0.);

        //::: no unspoiled clusters in CSC
        if (!isRun3() && (std::abs(etaMS) > 2.0 || std::abs(etaCB) > 2.0)) {
            if (summary.cscUnspoiledEtaHits == 0) {
                ATH_MSG_VERBOSE("Muon has only spoiled CSC clusters - fail high-pT");
                return false;
            }
        }

        // veto bad CSC giving troubles with scale factors
        if (!isRun3() && m_etaAcc(mu) < -1.899 && std::abs(m_phiAcc(mu)) < 0.211) {
            ATH_MSG_VERBOSE("Muon is in eta/phi region vetoed due to disabled chambers in MC - fail high-pT");
            return false;
        }

        //::: Barrel/Endcap overlap region
        if ((1.01 < std::abs(etaMS) && std::abs(etaMS) < 1.1) || (1.01 < std::abs(etaCB) && std::abs(etaCB) < 1.1)) {
            ATH_MSG_VERBOSE("Muon is in barrel/endcap overlap region - fail high-pT");
            return false;
        }

        //::: BIS78
        if (isBIS78(etaMS, phiMS)) {
            if (!isRun3() || !m_useBEEBISInHighPtRun3) {
                ATH_MSG_VERBOSE("Muon is in BIS7/8 eta/phi region - fail high-pT");
                return false;
            }
        }

        //// tentatively removed for r22, to be rechecked
        ////::: BMG - only veto in 2017+2018 data and corresponding MC
        //if (getRunNumber(true) >= 324320) {
            //if (isBMG(etaMS, phiMS)) {
                //ATH_MSG_VERBOSE("Muon is in BMG eta/phi region - fail high-pT");
                //return false;
            //}
        //}

        //::: BEE
        if (isBEE(etaMS, phiMS)) {
            // in Run3, large mis-alignment on the BEE chamber was found. temporarily mask the BEE region
            if (isRun3() && !m_useBEEBISInHighPtRun3) {
                ATH_MSG_VERBOSE("Muon is in BEE eta/phi region - fail high-pT");
                return false;
            }
            // Muon falls in the BEE eta-phi region: asking for 4 good precision layers
            // if( nGoodPrecLayers < 4 ) return false; // postponed (further studies needed)
            if (summary.nprecisionLayers < 4) {
                ATH_MSG_VERBOSE("Muon is in BEE eta/phi region and does not have 4 precision layers - fail high-pT");
                return false;
            }
        }
        if (std::abs(etaCB) > 1.4) {
            // Veto residual 3-station muons in BEE region due to MS eta/phi resolution effects
            // if( nGoodPrecLayers<4 && (extendedSmallHits>0||extendedSmallHoles>0) ) return false; // postponed (further studies
            // needed)
            if (summary.nprecisionLayers < 4 && (summary.extendedSmallHits > 0 || summary.extendedSmallHoles > 0)) {
                ATH_MSG_VERBOSE("Muon is in BEE eta/phi region and does not have 4 precision layers - fail high-pT");
                return false;
            }
        }

        //::: Apply 1/p significance cut
        auto idtrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_idTrackLinkAcc).opt_value());
        auto metrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_meTrackLinkAcc));
        if (idtrack && metrack && m_trkCovAcc(*metrack)(4, 4) > 0) {
            const float qOverPsignif = qOverPsignificance(mu);

            ATH_MSG_VERBOSE("qOverP significance: " << qOverPsignif);

            if (std::abs(qOverPsignif) > 7) {
                ATH_MSG_VERBOSE("Muon failed qOverP significance cut");
                return false;
            }
        } else {
            ATH_MSG_VERBOSE("Muon missing ID or ME tracks - fail high-pT");
            return false;
        }

        // Accept good 2-station muons if the user has opted to include these
        if (m_use2stationMuonsHighPt && summary.nprecisionLayers == 2) {
            // should not accept EM+EO muons due to ID/MS alignment issues
            if (std::abs(m_etaAcc(mu)) > 1.2 && summary.extendedSmallHits < 3 && summary.extendedLargeHits < 3) {
                ATH_MSG_VERBOSE("2-station muon with EM+EO - fail high-pT");
                return false;
            }

            // only select muons missing the inner precision layer
            // apply strict veto on overlap between small and large sectors

            if (summary.innerLargeHits == 0 && summary.middleLargeHits == 0 && summary.outerLargeHits == 0 &&
                summary.extendedLargeHits == 0 && summary.middleSmallHits > 2 &&
                (summary.outerSmallHits > 2 || summary.extendedSmallHits > 2)) {
                ATH_MSG_VERBOSE("Accepted 2-station muon in small sector");
                return true;
            }

            if (summary.innerSmallHits == 0 && summary.middleSmallHits == 0 && summary.outerSmallHits == 0 &&
                summary.extendedSmallHits == 0 && summary.middleLargeHits > 2 &&
                (summary.outerLargeHits > 2 || summary.extendedLargeHits > 2)) {
                ATH_MSG_VERBOSE("Accepted 2-station muon in large sector");
                return true;
            }
        }

        //::: Require 3 (good) station muons
        if (summary.nprecisionLayers < 3) {
            ATH_MSG_VERBOSE("Muon has less than 3 precision layers - fail high-pT");
            return false;
        }

        // Remove 3-station muons with small-large sectors overlap
        if (summary.isSmallGoodSectors) {
            if (!(summary.innerSmallHits > 2 && summary.middleSmallHits > 2 &&
                  (summary.outerSmallHits > 2 || summary.extendedSmallHits > 2))) {
                ATH_MSG_VERBOSE("Muon has small/large sectors overlap - fail high-pT");
                return false;
            }
        } else {
            if (!(summary.innerLargeHits > 2 && summary.middleLargeHits > 2 &&
                  (summary.outerLargeHits > 2 || summary.extendedLargeHits > 2))) {
                ATH_MSG_VERBOSE("Muon has small/large sectors overlap - fail high-pT");
                return false;
            }
        }

        ATH_MSG_VERBOSE("Muon passed high-pT selection");
        return true;
    }

    bool MuonSelectionTool::passedErrorCutCB(const xAOD::Muon& mu) const {
        return passedErrorCutCB(columnar::MuonId(mu));
    }

    bool MuonSelectionTool::passedErrorCutCB(columnar::MuonId mu) const {
        // ::
        if (m_muonTypeAcc(mu) != xAOD::Muon::Combined) return false;
        // ::
        const double abs_eta = std::abs(m_etaAcc(mu));
        double start_cut = 3.0, end_cut = 1.6;

        // parametrization of expected q/p error as function of pT
        double p0 = 8.0, p1 = 0., p2 = 0.;
        if (isRun3()) { // MC21 optimization
            if      (abs_eta <= 1.05)                         { p1 = 0.046; p2 = 0.00005; }
            else if (abs_eta > 1.05 && abs_eta <= 1.3)        { p1 = 0.052; p2 = 0.00008; }
            else if (abs_eta > 1.3  && abs_eta <= 1.7)        { p1 = 0.068; p2 = 0.00006; }
            else if (abs_eta > 1.7  && abs_eta <= 2.0)        { p1 = 0.048; p2 = 0.00006; }
            else if (abs_eta > 2.0)                           { p1 = 0.037; p2 = 0.00006; }
        } else {
            if      (abs_eta <= 1.05)                         { p1 = 0.039; p2 = 0.00006; }
            else if (abs_eta > 1.05 && abs_eta <= 1.3)        { p1 = 0.040; p2 = 0.00009; }
            else if (abs_eta > 1.3  && abs_eta <= 1.7)        { p1 = 0.056; p2 = 0.00008; }
            else if (abs_eta > 1.7  && abs_eta <= 2.0)        { p1 = 0.041; p2 = 0.00006; }
            else if (abs_eta > 2.0)                           { p1 = 0.031; p2 = 0.00006; }
        }
        // ::
        hitSummary summary{};
        fillSummary(mu, summary);

        // independent parametrization for 2-station muons
        if (m_use2stationMuonsHighPt && summary.nprecisionLayers == 2) {
            start_cut = 1.1; end_cut = 0.7;
            p1 = 0.0739568; p2 = 0.00012443;
            if      (abs_eta > 1.05 && abs_eta < 1.3)  { p1 = 0.0674484; p2 = 0.000119879; }
            else if (abs_eta >= 1.3 && abs_eta < 1.7)  { p1 = 0.041669;  p2 = 0.000178349; }
            else if (abs_eta >= 1.7 && abs_eta < 2.0)  { p1 = 0.0488664; p2 = 0.000137648; }
            else if (abs_eta >= 2.0)                    { p1 = 0.028077;  p2 = 0.000152707; }
        }
        // ::
        bool passErrorCutCB = false;
        const auto cbLink = mu(m_cbTrackLinkAcc);
        if (cbLink) {
            // ::
            const auto cbtrack = columnar::OptObjectId<columnar::MuonTrackDef>(cbLink);
            const double pt_CB = std::min(m_trkMomentumAcc.pt(*cbtrack, 0.) * MeVtoGeV, 5000.);
            const double qOverP_CB    = m_qOverPCBTrackAcc(*cbLink);
            const double qOverPerr_CB = std::sqrt(m_trkCovAcc(*cbtrack)(4, 4));
            // sigma represents the average expected error at the muon's pt/eta
            const double sigma       = std::sqrt(std::pow(p0 / pt_CB, 2) + std::pow(p1, 2) + std::pow(p2 * pt_CB, 2));
            // cutting at start_cut*sigma for pt <=1 TeV depending on eta region,
            // then linearly tightening until end_cut*sigma is reached at pt >= 5TeV.
            const double a           = (end_cut - start_cut) / 4000.0;
            const double b           = end_cut - a * 5000.0;
            const double coefficient = (pt_CB > 1000.) ? (a * pt_CB + b) : start_cut;
            if (std::abs(qOverPerr_CB / qOverP_CB) < coefficient * sigma) passErrorCutCB = true;
        }
        // ::
        if (m_use2stationMuonsHighPt && m_doBadMuonVetoMimic && summary.nprecisionLayers == 2) {
            bool isSim = false;
            if constexpr (columnar::ColumnarModeDefault::isXAOD) {
                ATH_MSG_DEBUG("passedErrorCutCB: reading IS_SIMULATION flag via xAOD bridge");
                SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo);
                isSim = eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION);
            } else {
                ATH_MSG_DEBUG("passedErrorCutCB: IS_SIMULATION unavailable in columnar mode; BMV mimic cut skipped");
            }
            if (isSim) {
                ATH_MSG_DEBUG("The current event is a MC event. Use bad muon veto mimic.");
                return passErrorCutCB && passedBMVmimicCut(mu);
            }
        }


        // ::
        return passErrorCutCB;
    }

    bool MuonSelectionTool::passedBMVmimicCut(const xAOD::Muon& mu) const {
        return passedBMVmimicCut(columnar::MuonId(mu));
    }

    bool MuonSelectionTool::passedBMVmimicCut(columnar::MuonId mu) const {
        const auto cbLink = mu(m_cbTrackLinkAcc);
        if (!cbLink) {
            ATH_MSG_VERBOSE("passedBMVmimicCut: no combined track, failing");
            return false;
        }

        const float eta = m_etaAcc(mu);
        TF1* cutFunction;
        double p1, p2;
        if (std::abs(eta) < 1.05) {
            cutFunction = m_BMVcutFunction_barrel.get();
            p1 = 0.066265;
            p2 = 0.000210047;
        } else {
            cutFunction = m_BMVcutFunction_endcap.get();
            p1 = 0.0629747;
            p2 = 0.000196466;
        }

        const auto cbtrack = columnar::OptObjectId<columnar::MuonTrackDef>(cbLink);
        const double cbPt = m_trkMomentumAcc.pt(*cbtrack, 0.) * MeVtoGeV;
        const double qOpRelResolution = std::hypot(p1, p2 * cbPt);

        const double qOverPabs_unsmeared = std::abs(m_qOverPCBTrackAcc(*cbLink));
        const double qOverPabs_smeared   = 1.0 / (m_ptAcc(mu) * std::cosh(eta));

        return (qOverPabs_smeared - qOverPabs_unsmeared) / (qOpRelResolution * qOverPabs_unsmeared) >=
               cutFunction->Eval(cbPt);
    }

    bool MuonSelectionTool::passedMuonCuts(const xAOD::Muon& mu) const {
        return passedMuonCuts(columnar::MuonId(mu));
    }

    bool MuonSelectionTool::passedMuonCuts(columnar::MuonId mu) const {
        const auto  muonType = m_muonTypeAcc(mu);
        const float eta      = m_etaAcc(mu);
        const auto  author   = m_authorAcc(mu);

        // ::
        if (muonType == xAOD::Muon::Combined) { return author != xAOD::Muon::STACO; }
        // ::
        if (muonType == xAOD::Muon::CaloTagged && std::abs(eta) < 0.105)
            return passedCaloTagQuality(mu);
        // ::
        if (muonType == xAOD::Muon::SegmentTagged && (std::abs(eta) < 0.105 || m_useSegmentTaggedLowPt))
            return true;
        // ::
        if (author == xAOD::Muon::MuidSA && std::abs(eta) > 2.4) return true;
        // ::
        if (muonType == xAOD::Muon::SiliconAssociatedForwardMuon) {
            auto cbtrack = columnar::OptObjectId<columnar::MuonTrackDef>(mu(m_cbTrackLinkAcc));
            return (cbtrack && std::abs(m_trkMomentumAcc.eta(*cbtrack, 0.)) > 2.4);
        }
        // ::
        return false;
    }

    bool MuonSelectionTool::passedIDCuts(const xAOD::TrackParticle& track) const {
        return passedIDCuts(columnar::ObjectId<columnar::MuonTrackDef>(track));
    }

    bool MuonSelectionTool::passedIDCuts(columnar::ObjectId<columnar::MuonTrackDef> track) const {
        if ((m_PixCutOff || m_SctCutOff || m_SiHolesCutOff) && !m_developMode)
            ATH_MSG_WARNING(
                " !! Tool configured with some of the ID hits requirements changed... FOR DEVELOPMENT ONLY: muon efficiency SF won't be "
                "valid !! ");

        const uint8_t nPixHits  = m_nPixelHitsIDTrackAcc(track);
        const uint8_t nPixDead  = m_nPixelDeadSensorsIDTrackAcc(track);
        if ((nPixHits + nPixDead == 0) && !m_PixCutOff) return false;

        const uint8_t nSCTHits  = m_nSCTHitsIDTrackAcc(track);
        const uint8_t nSCTDead  = m_nSCTDeadSensorsIDTrackAcc(track);
        if ((nSCTHits + nSCTDead <= 4) && !m_SctCutOff) return false;

        const uint8_t nPixHoles = m_nPixelHolesIDTrackAcc(track);
        const uint8_t nSCTHoles = m_nSCTHolesIDTrackAcc(track);
        if ((nPixHoles + nSCTHoles >= 3) && !m_SiHolesCutOff) return false;

        if (!m_TrtCutOff) {
            const float   abseta  = std::abs(m_trkMomentumAcc.eta(track, 0.));
            const uint8_t nTRT    = m_nTRTHitsIDTrackAcc(track);
            const uint8_t nTRTOut = m_nTRTOutliersIDTrackAcc(track);
            const uint8_t totTRT  = nTRT + nTRTOut;
            if (!((0.1 < abseta && abseta <= 1.9 && totTRT > 5 && nTRTOut < (0.9 * totTRT)) ||
                  (abseta <= 0.1 || abseta > 1.9)))
                return false;
        }
        // Reached end - all ID hit cuts are passed.
        return true;
    }  // passedIDCuts

    bool MuonSelectionTool::passedCaloTagQuality(const xAOD::Muon& mu) const {
        return passedCaloTagQuality(columnar::MuonId(mu));
    }

    bool MuonSelectionTool::passedCaloTagQuality(columnar::MuonId mu) const {
        // Use CaloScore variable based on Neural Network if enabled
        // The neural network is only trained until eta = 1
        // cf. https://cds.cern.ch/record/2802605/files/CERN-THESIS-2021-290.pdf
        constexpr float eta_range = 1.;
        if (std::abs(m_etaAcc(mu)) < eta_range && m_useCaloScore) return passedCaloScore(mu);

        // Otherwise we use CaloMuonIDTag

        // Extract CaloMuonIDTag variable
        if (!m_caloMuonIDTagAcc.isAvailable(mu)) {
            ATH_MSG_WARNING("Unable to read CaloMuonIDTag Quality information! Rejecting the CALO muon!");
            return false;
        }

        // Cut on CaloMuonIDTag variable
        return (m_caloMuonIDTagAcc(mu) > 10);
    }

    bool MuonSelectionTool::passedCaloScore(const xAOD::Muon& mu) const {
        return passedCaloScore(columnar::MuonId(mu));
    }

    bool MuonSelectionTool::passedCaloScore(columnar::MuonId mu) const {
        // We use a working point with a pT-dependent cut on the NN discriminant, designed to achieve a constant
        // fakes rejection as function of pT in Z->mumu MC

        // Extract the relevant score variable (NN discriminant)
        if (!m_caloMuonScoreAcc.isAvailable(mu)) {
            ATH_MSG_WARNING("CaloMuonScore not available! Rejecting the CALO muon!");
            return false;
        }
        const float CaloMuonScore = m_caloMuonScoreAcc(mu);

        if (m_caloScoreWP == 1) return (CaloMuonScore >= 0.92);
        if (m_caloScoreWP == 2) return (CaloMuonScore >= 0.56);
        if (m_caloScoreWP == 3 || m_caloScoreWP == 4) {
            // Cut on the score variable
            const float pT = m_ptAcc(mu) * MeVtoGeV;
            if (pT > 20.0) return (CaloMuonScore >= 0.77);  // constant cut above 20 GeV
            // pT-dependent cut below 20 GeV
            // The pT-dependent cut is based on a fit of a third-degree polynomial, with coefficients as given below
            if (m_caloScoreWP == 3) return (CaloMuonScore >= (-1.98e-4*std::pow(pT,3) + 6.04e-3*std::pow(pT,2) - 6.13e-2*pT + 1.16));
            if (m_caloScoreWP == 4) return (CaloMuonScore >= (-1.80e-4*std::pow(pT,3) + 5.02e-3*std::pow(pT,2) - 4.62e-2*pT + 1.12));
        }
        return false;
    }

    bool MuonSelectionTool::passTight(const xAOD::Muon& mu, float rho, float oneOverPSig) const {
        return passTight(columnar::MuonId(mu), rho, oneOverPSig);
    }

    bool MuonSelectionTool::passTight(columnar::MuonId mu, float rho, float oneOverPSig) const {
        if (isRun3() && !m_developMode && (m_excludeNSWFromPrecisionLayers || !m_recalcPrecisionLayerswNSW)) {
            ATH_MSG_VERBOSE("for run3, Tight WP is only supported when ExcludeNSWFromPrecisionLayers=False and RecalcPrecisionLayerswNSW=True");
            return false;
        }
        float symmetric_eta = std::abs(m_etaAcc(mu));
        float pt = m_ptAcc(mu) * MeVtoGeV;

        // Impose pT and eta cuts; the bounds of the cut maps
        if (pt < 4.0 || symmetric_eta >= 2.5) return false;
        ATH_MSG_VERBOSE("Muon is passing tight WP kinematic cuts with pT,eta " << pt << "  ,  " << m_etaAcc(mu));

        // ** Low pT specific cuts ** //
        if (pt < 20.0) {
            double rhoCut    = m_tightWP_lowPt_rhoCuts->Interpolate(pt, symmetric_eta);
            double qOverPCut = m_tightWP_lowPt_qOverPCuts->Interpolate(pt, symmetric_eta);

            ATH_MSG_VERBOSE("Applying tight WP cuts to a low pt muon with (pt,eta) ( " << pt << " , " << m_etaAcc(mu) << " ) ");
            ATH_MSG_VERBOSE("Rho value " << rho << ", required to be less than " << rhoCut);
            ATH_MSG_VERBOSE("Momentum significance value " << oneOverPSig << ", required to be less than " << qOverPCut);

            if (rho > rhoCut) return false;
            ATH_MSG_VERBOSE("Muon passed tight WP, low pT rho cut!");

            if (oneOverPSig > qOverPCut) return false;
            ATH_MSG_VERBOSE("Muon passed tight WP, low pT momentum significance cut");

            // Tight muon!
            return true;

        }

        // ** Medium pT specific cuts ** //
        else if (pt < 100.0) {
            double rhoCut = m_tightWP_mediumPt_rhoCuts->Interpolate(pt, symmetric_eta);
            //
            ATH_MSG_VERBOSE("Applying tight WP cuts to a medium pt muon with (pt,eta) (" << pt << "," << m_etaAcc(mu) << ")");
            ATH_MSG_VERBOSE("Rho value " << rho << " required to be less than " << rhoCut);

            // Apply cut
            if (rho > rhoCut) return false;
            ATH_MSG_VERBOSE("Muon passed tight WP, medium pT rho cut!");

            // Tight muon!
            return true;
        }

        // ** High pT specific cuts
        else if (pt < 500.0) {
            //
            ATH_MSG_VERBOSE("Applying tight WP cuts to a high pt muon with (pt,eta) (" << pt << "," << m_etaAcc(mu) << ")");
            // No interpolation, since bins with -1 mean we should cut really loose
            double rhoCut = m_tightWP_highPt_rhoCuts->GetBinContent(m_tightWP_highPt_rhoCuts->FindFixBin(pt, symmetric_eta));
            ATH_MSG_VERBOSE("Rho value " << rho << ", required to be less than " << rhoCut << " unless -1, in which no cut is applied");
            //
            if (rhoCut < 0.0) return true;
            if (rho > rhoCut) return false;
            ATH_MSG_VERBOSE("Muon passed tight WP, high pT rho cut!");

            return true;
        }
        // For muons with pT > 500 GeV, no extra cuts
        else {
            ATH_MSG_VERBOSE("Not applying any tight WP cuts to a very high pt muon with (pt,eta) (" << pt << "," << m_etaAcc(mu) << ")");
            return true;
        }

        // you should never reach this point
        return false;
    }

    //============================================================================
    void MuonSelectionTool::fillSummary(const xAOD::Muon& muon, hitSummary& summary) const {
        fillSummary(columnar::MuonId(muon), summary);
    }

    void MuonSelectionTool::fillSummary(columnar::MuonId muon, hitSummary& summary) const {

        checkSanity();

        summary.nprecisionLayers     = m_nprecisionLayersAcc(muon);
        summary.nprecisionHoleLayers = m_nprecisionHoleLayersAcc(muon);
        summary.nGoodPrecLayers      = m_nGoodPrecLayersAcc(muon);
        summary.innerSmallHits       = m_innerSmallHitsAcc(muon);
        summary.innerLargeHits       = m_innerLargeHitsAcc(muon);
        summary.middleSmallHits      = m_middleSmallHitsAcc(muon);
        summary.middleLargeHits      = m_middleLargeHitsAcc(muon);
        summary.outerSmallHits       = m_outerSmallHitsAcc(muon);
        summary.outerLargeHits       = m_outerLargeHitsAcc(muon);
        summary.extendedSmallHits    = m_extendedSmallHitsAcc(muon);
        summary.extendedLargeHits    = m_extendedLargeHitsAcc(muon);
        summary.extendedSmallHoles   = m_extendedSmallHolesAcc(muon);
        summary.isSmallGoodSectors   = m_isSmallGoodSectorsAcc(muon);

        if (!isRun3()) {
            // ignore missing of cscUnspoiledEtaHits in case we are running in expert developer mode
            // e.g. for when we want to apply Run2 WPs in Run3
            if (!m_developMode && !m_cscUnspoiledEtaHitsAcc.isAvailable(muon)) {
                ATH_MSG_FATAL(__FILE__ << ":" << __LINE__ << " cscUnspoiledEtaHits not available");
                throw std::runtime_error("cscUnspoiledEtaHits not available");
            }
            if (m_cscUnspoiledEtaHitsAcc.isAvailable(muon))
                summary.cscUnspoiledEtaHits = m_cscUnspoiledEtaHitsAcc(muon);

            if (std::abs(m_etaAcc(muon)) > 2.0) {
                ATH_MSG_VERBOSE("Recalculating number of precision layers for combined muon");
                summary.nprecisionLayers = (summary.innerSmallHits > 1 || summary.innerLargeHits > 1)
                                         + (summary.middleSmallHits > 2 || summary.middleLargeHits > 2)
                                         + (summary.outerSmallHits > 2 || summary.outerLargeHits > 2);
            }

        } else if (std::abs(m_etaAcc(muon)) > 1.3 && (m_excludeNSWFromPrecisionLayers || m_recalcPrecisionLayerswNSW)) {
            summary.nprecisionLayers = (summary.middleSmallHits > 2 || summary.middleLargeHits > 2)
                                     + (summary.outerSmallHits > 2 || summary.outerLargeHits > 2)
                                     + (summary.extendedSmallHits > 2 || summary.extendedLargeHits > 2);

            if (!m_excludeNSWFromPrecisionLayers && m_recalcPrecisionLayerswNSW) {

                if (!m_etaLayer1STGCHitsAcc.isAvailable(muon) || !m_etaLayer2STGCHitsAcc.isAvailable(muon) || !m_MMHitsAcc.isAvailable(muon)) {
                    ATH_MSG_FATAL(__FILE__ << ":" << __LINE__ << " Failed to retrieve NSW hits!"
                                           << " (Please use DxAODs with p-tags >= p5834 OR set ExcludeNSWFromPrecisionLayers to True (tests only)");
                    throw std::runtime_error("Failed to retrieve NSW hits");
                }

                summary.etaLayer1STGCHits = m_etaLayer1STGCHitsAcc(muon);
                summary.etaLayer2STGCHits = m_etaLayer2STGCHitsAcc(muon);
                summary.MMHits            = m_MMHitsAcc(muon);
                summary.nprecisionLayers += ((summary.etaLayer1STGCHits + summary.etaLayer2STGCHits) > 3 || summary.MMHits > 3);
            }
        }
    }



    void MuonSelectionTool::retrieveParam(const xAOD::Muon& muon, float& value, const xAOD::Muon::ParamDef param) const {
        if (!muon.parameter(value, param)) {
            ATH_MSG_FATAL(__FILE__ << ":" << __LINE__ << " Failed to retrieve parameter " << param
                                   << " for muon with pT:" << muon.pt() * MeVtoGeV << ", eta:" << muon.eta() << ", phi: " << muon.phi()
                                   << ", q:" << muon.charge() << ", author: " << muon.author());
            throw std::runtime_error("Failed to retrieve Parameter");
        }
    }

    // Returns an integer corresponding to categorization of muons with different resolutions
    int MuonSelectionTool::getResolutionCategory(const xAOD::Muon& mu) const {
        // Resolutions have only been evaluated for medium combined muons
        if (mu.muonType() != xAOD::Muon::Combined || getQuality(mu) > xAOD::Muon::Medium) return ResolutionCategory::unclassified;

        // :: Access MS hits information
        hitSummary summary{};
        fillSummary(mu, summary);

        // For muons passing the high-pT working point, distinguish between 2-station tracks and the rest
        if (passedHighPtCuts(mu)) {
            if (summary.nprecisionLayers == 2)
                return ResolutionCategory::highPt2station;
            else
                return ResolutionCategory::highPt;
        }

        const xAOD::TrackParticle* CB_track = mu.trackParticle(xAOD::Muon::CombinedTrackParticle);
        const xAOD::TrackParticle* MS_track = mu.trackParticle(xAOD::Muon::MuonSpectrometerTrackParticle);
        if (!MS_track) {
            ATH_MSG_VERBOSE("getResolutionCategory - No MS track available for muon. Using combined track.");
            MS_track = mu.trackParticle(xAOD::Muon::CombinedTrackParticle);
        }

        if (!MS_track || !CB_track) return ResolutionCategory::unclassified;
        const float etaMS = MS_track->eta();
        const float etaCB = CB_track->eta();
        const float phiMS = MS_track->phi();

        int category = ResolutionCategory::unclassified;

        if ((summary.isSmallGoodSectors && summary.innerSmallHits < 3) || (!summary.isSmallGoodSectors && summary.innerLargeHits < 3))
            category = ResolutionCategory::missingInner;  // missing-inner

        if ((summary.isSmallGoodSectors && summary.middleSmallHits < 3) || (!summary.isSmallGoodSectors && summary.middleLargeHits < 3))
            category = ResolutionCategory::missingMiddle;  // missing-middle

        if ((summary.isSmallGoodSectors && summary.outerSmallHits < 3 && summary.extendedSmallHits < 3) ||
            (!summary.isSmallGoodSectors && summary.outerLargeHits < 3 && summary.extendedLargeHits < 3))
            category = ResolutionCategory::missingOuter;  // missing-outer

        if (!isRun3() && (std::abs(etaMS) > 2.0 || std::abs(etaCB) > 2.0) && summary.cscUnspoiledEtaHits == 0)
            category = ResolutionCategory::spoiledCSC;  // spoiled CSC

        if ((1.01 < std::abs(etaMS) && std::abs(etaMS) < 1.1) || (1.01 < std::abs(etaCB) && std::abs(etaCB) < 1.1))
            category = ResolutionCategory::BEoverlap;  // barrel-end-cap overlap

        if (isBIS78(etaMS, phiMS)) category = ResolutionCategory::BIS78;  // BIS7/8

        //::: BEE
        if (isBEE(etaMS, phiMS) || (std::abs(etaCB) > 1.4 && (summary.extendedSmallHits > 0 || summary.extendedSmallHoles > 0))) {
            if (summary.extendedSmallHits < 3 && summary.middleSmallHits >= 3 && summary.outerSmallHits >= 3)
                category = ResolutionCategory::missingBEE;  // missing-BEE

            if (summary.extendedSmallHits >= 3 && summary.outerSmallHits < 3) category = ResolutionCategory::missingOuter;  // missing-outer

            if (!summary.isSmallGoodSectors)
                category = ResolutionCategory::unclassified;  // ambiguity due to eta/phi differences between MS and CB track
        }

        if (summary.nprecisionLayers == 1) category = ResolutionCategory::oneStation;  // one-station track

        return category;
    }

    //============================================================================
    // need run number (or random run number) to apply period-dependent selections
    unsigned int MuonSelectionTool::getRunNumber(bool needOnlyCorrectYear /*=false*/) const {

        static const SG::AuxElement::ConstAccessor<unsigned int> acc_rnd("RandomRunNumber");

        SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo);
	//overwrite run number
        unsigned int runNumber = 0;
        if(m_expertMode_RunNumber.value()!=0) runNumber=m_expertMode_RunNumber.value();
        else runNumber = eventInfo->runNumber();

        // Case of data
        if (!eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) {
            ATH_MSG_DEBUG("The current event is a data event. Return runNumber.");
            return runNumber;
        }
 
        // Case of MC 
        // attempt to get the run number assigned by the PRW tool
        static std::atomic<bool> issuedWarningPRW{false};
        if (acc_rnd.isAvailable(*eventInfo)) {
            unsigned int rn = acc_rnd(*eventInfo);
            if (rn != 0) return acc_rnd(*eventInfo);

            if (!issuedWarningPRW) {
                ATH_MSG_WARNING("Pile up tool has assigned runNumber = 0");
                issuedWarningPRW = true;
            }
        }

        // otherwise return a dummy run number
        if (needOnlyCorrectYear) {
            if (runNumber < 300000) {        // mc16a (2016): 284500
                ATH_MSG_DEBUG("Random run number not available and this is mc16a or mc20a, returning dummy 2016 run number.");
                return 311071;
                    
            } else if (runNumber < 310000) { // mc16d (2017): 300000
                ATH_MSG_DEBUG("Random run number not available and this is mc16d or mc20d, returning dummy 2017 run number.");
                return 340072;
                    
            } else if (runNumber < 320000) { // mc16e (2018): 310000
                ATH_MSG_DEBUG("Random run number not available and this is mc16e or mc20e, returning dummy 2018 run number.");
                return 351359;

            } else if (runNumber < 600000) { //mc21: 330000, mc23a: 410000, mc23c: 450000
                ATH_MSG_DEBUG("Random run number not available and this is mc21/mc23, for the time being we're returing a dummy run number.");
                return 399999;
            } else {
                ATH_MSG_DEBUG("Detected some run 4 / phase II runnumber "<<runNumber<<". ");
                return 666666;
            }

            ATH_MSG_FATAL("Random run number not available, fallback option of using runNumber failed since "<<runNumber<<" cannot be recognised");
            throw std::runtime_error("MuonSelectionTool() - need RandomRunNumber decoration by the PileupReweightingTool");
        }

        ATH_MSG_FATAL("Failed to find the RandomRunNumber decoration by the PileupReweightingTool");
        throw std::runtime_error("MuonSelectionTool() - need RandomRunNumber decoration from PileupReweightingTool");
    }


    // Check if eta/phi coordinates correspond to BIS7/8 chambers
    bool MuonSelectionTool::isBIS78(const float eta, const float phi) const {
        static constexpr std::array<float, 2> BIS78_eta{1.05, 1.3};
        static constexpr std::array<float, 8> BIS78_phi{0.21, 0.57, 1.00, 1.33, 1.78, 2.14, 2.57, 2.93};

        float abs_eta = std::abs(eta);
        float abs_phi = std::abs(phi);

        if (abs_eta >= BIS78_eta[0] && abs_eta <= BIS78_eta[1]) {
            if ((abs_phi >= BIS78_phi[0] && abs_phi <= BIS78_phi[1]) || (abs_phi >= BIS78_phi[2] && abs_phi <= BIS78_phi[3]) ||
                (abs_phi >= BIS78_phi[4] && abs_phi <= BIS78_phi[5]) || (abs_phi >= BIS78_phi[6] && abs_phi <= BIS78_phi[7])) {
                return true;
            }
        }

        return false;
    }

    // Check if eta/phi coordinates correspond to BEE chambers
    bool MuonSelectionTool::isBEE(const float eta, const float phi) const {
        static constexpr std::array<float, 2> BEE_eta{1.440, 1.692};
        static constexpr std::array<float, 8> BEE_phi{0.301, 0.478, 1.086, 1.263, 1.872, 2.049, 2.657, 2.834};

        float abs_eta = std::abs(eta);
        float abs_phi = std::abs(phi);

        if (abs_eta >= BEE_eta[0] && abs_eta <= BEE_eta[1]) {
            if ((abs_phi >= BEE_phi[0] && abs_phi <= BEE_phi[1]) || (abs_phi >= BEE_phi[2] && abs_phi <= BEE_phi[3]) ||
                (abs_phi >= BEE_phi[4] && abs_phi <= BEE_phi[5]) || (abs_phi >= BEE_phi[6] && abs_phi <= BEE_phi[7])) {
                return true;
            }
        }

        return false;
    }

    // Check if eta/phi coordinates correspond to BMG chambers
    bool MuonSelectionTool::isBMG(const float eta, const float phi) const {
        static constexpr std::array<float, 6> BMG_eta{0.35, 0.47, 0.68, 0.80, 0.925, 1.04};
        static constexpr std::array<float, 4> BMG_phi{-1.93, -1.765, -1.38, -1.21};

        float abs_eta = std::abs(eta);

        if ((abs_eta >= BMG_eta[0] && abs_eta <= BMG_eta[1]) || (abs_eta >= BMG_eta[2] && abs_eta <= BMG_eta[3]) ||
            (abs_eta >= BMG_eta[4] && abs_eta <= BMG_eta[5])) {
            if ((phi >= BMG_phi[0] && phi <= BMG_phi[1]) || (phi >= BMG_phi[2] && phi <= BMG_phi[3])) { return true; }
        }

        return false;
    }

    float MuonSelectionTool::getTightNNScore(const xAOD::Muon& mu) const {
        if (!m_calculateTightNNScore)
        {
            ATH_MSG_ERROR("TightNNScore calculation is disabled. Please set the property CalculateTightNNScore to true.");
            throw std::runtime_error("cannot calculate TightNNScore");  
        }
        //this score currently only can be calculated for combined muons
        if (mu.muonType() != xAOD::Muon::Combined) return -999;
        const xAOD::TrackParticle* idtrack = mu.trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
            const xAOD::TrackParticle* metrack = mu.trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);
        if(!idtrack || !metrack) return -999;
        //the score is only calculated for muons which pass the Medium WP
        if (getQuality(mu) > xAOD::Muon::Medium) return -999;
        //only muons with pt > 4 GeV and |eta|<2.5 are considered
        if (std::abs(mu.eta())>2.5) return -999;
        if(mu.pt()<4000.) return -999;

        std::vector<float> input_features;
        // 1. Fill input features
        int mu_author=mu.author();
        float mu_rhoPrime=rhoPrime(mu);
        float mu_scatteringCurvatureSignificance=0.;
        retrieveParam(mu, mu_scatteringCurvatureSignificance, xAOD::Muon::scatteringCurvatureSignificance);
        float mu_scatteringNeighbourSignificance=0.;
        retrieveParam(mu, mu_scatteringNeighbourSignificance, xAOD::Muon::scatteringNeighbourSignificance);
        float mu_momentumBalanceSignificance=0.;
        retrieveParam(mu, mu_momentumBalanceSignificance, xAOD::Muon::momentumBalanceSignificance);
        float mu_qOverPSignificance=qOverPsignificance(mu);
        float mu_reducedChi2=mu.primaryTrackParticle()->chiSquared() / mu.primaryTrackParticle()->numberDoF();
        float mu_reducedChi2_ID=idtrack->chiSquared() / idtrack->numberDoF();
        float mu_reducedChi2_ME=metrack->chiSquared() / metrack->numberDoF();
        float mu_spectrometerFieldIntegral=0.;
        retrieveParam(mu, mu_spectrometerFieldIntegral, xAOD::Muon::spectrometerFieldIntegral);
        float mu_segmentDeltaEta=0;
        retrieveParam(mu, mu_segmentDeltaEta, xAOD::Muon::segmentDeltaEta);
        uint8_t mu_numberOfPixelHits=0;
        retrieveSummaryValue(mu, mu_numberOfPixelHits, xAOD::SummaryType::numberOfPixelHits);
        uint8_t mu_numberOfPixelDeadSensors=0;
        retrieveSummaryValue(mu, mu_numberOfPixelDeadSensors, xAOD::SummaryType::numberOfPixelDeadSensors);
        uint8_t mu_innerLargeHits=0;
        retrieveSummaryValue(mu, mu_innerLargeHits, xAOD::MuonSummaryType::innerLargeHits);
        uint8_t mu_innerSmallHits=0;
        retrieveSummaryValue(mu, mu_innerSmallHits, xAOD::MuonSummaryType::innerSmallHits);
        uint8_t mu_middleLargeHits=0;
        retrieveSummaryValue(mu, mu_middleLargeHits, xAOD::MuonSummaryType::middleLargeHits);
        uint8_t mu_middleSmallHits=0;
        retrieveSummaryValue(mu, mu_middleSmallHits, xAOD::MuonSummaryType::middleSmallHits);
        uint8_t mu_outerLargeHits=0;
        retrieveSummaryValue(mu, mu_outerLargeHits, xAOD::MuonSummaryType::outerLargeHits);
        uint8_t mu_outerSmallHits=0;
        retrieveSummaryValue(mu, mu_outerSmallHits, xAOD::MuonSummaryType::outerSmallHits);

        if(!isRun3())
        {
            input_features =     {(float)mu_author,
                                mu_rhoPrime,
                                mu_scatteringCurvatureSignificance,
                                mu_scatteringNeighbourSignificance,
                                mu_momentumBalanceSignificance,
                                mu_qOverPSignificance,
                                mu_reducedChi2,
                                mu_reducedChi2_ID,
                                mu_reducedChi2_ME,
                                mu_spectrometerFieldIntegral,
                                mu_segmentDeltaEta,
                                (float)mu_numberOfPixelHits,
                                (float)mu_numberOfPixelDeadSensors,
                                (float)mu_innerLargeHits,
                                (float)mu_innerSmallHits,
                                (float)mu_middleLargeHits,
                                (float)mu_middleSmallHits,
                                (float)mu_outerLargeHits,
                                (float)mu_outerSmallHits};
        }
        else
        {
            uint8_t mu_phiLayer1STGCHits=0;
            retrieveSummaryValue(mu, mu_phiLayer1STGCHits, xAOD::MuonSummaryType::phiLayer1STGCHits);
            uint8_t mu_phiLayer2STGCHits=0;
            retrieveSummaryValue(mu, mu_phiLayer2STGCHits, xAOD::MuonSummaryType::phiLayer2STGCHits);
            uint8_t mu_etaLayer1STGCHits=0;
            retrieveSummaryValue(mu, mu_etaLayer1STGCHits, xAOD::MuonSummaryType::etaLayer1STGCHits);
            uint8_t mu_etaLayer2STGCHits=0;
            retrieveSummaryValue(mu, mu_etaLayer2STGCHits, xAOD::MuonSummaryType::etaLayer2STGCHits);
            uint8_t mu_MMHits=0;
            retrieveSummaryValue(mu, mu_MMHits, xAOD::MuonSummaryType::MMHits);
            input_features =     {(float)mu_author,
                                mu_rhoPrime,
                                mu_scatteringCurvatureSignificance,
                                mu_scatteringNeighbourSignificance,
                                mu_momentumBalanceSignificance,
                                mu_qOverPSignificance,
                                mu_reducedChi2,
                                mu_reducedChi2_ID,
                                mu_reducedChi2_ME,
                                mu_spectrometerFieldIntegral,
                                mu_segmentDeltaEta,
                                (float)mu_numberOfPixelHits,
                                (float)mu_numberOfPixelDeadSensors,
                                (float)mu_innerLargeHits,
                                (float)mu_innerSmallHits,
                                (float)mu_middleLargeHits,
                                (float)mu_middleSmallHits,
                                (float)mu_outerLargeHits,
                                (float)mu_outerSmallHits,
                                (float)mu_phiLayer1STGCHits,
                                (float)mu_phiLayer2STGCHits,
                                (float)mu_etaLayer1STGCHits,
                                (float)mu_etaLayer2STGCHits,
                                (float)mu_MMHits};
        }

        float score=-999.;
        std::vector<int64_t> inputShape = {1, static_cast<int64_t>(input_features.size())};

        AthInfer::InputDataMap inputData;
        inputData["flatten_input"] = std::make_pair(
            inputShape, std::move(input_features)
        );

        AthInfer::OutputDataMap outputData;
        outputData["sequential"] = std::make_pair(
            std::vector<int64_t>{1, 1}, std::vector<float>{}
        );

        if (!m_onnxTool->inference(inputData, outputData).isSuccess()) {
            ATH_MSG_WARNING("ONNX inference failed!");
            return -999.;
        }
        const auto& variant = outputData["sequential"].second;
        if (std::holds_alternative<std::vector<float>>(variant)) {
            const auto& vec = std::get<std::vector<float>>(variant);
            if (!vec.empty()) score = vec[0];
            else {
                ATH_MSG_WARNING("ONNX output vector is empty!");
                return -999.;
            }
        } else {
            ATH_MSG_WARNING("ONNX output is not a float vector!");
            return -999.;
        }
        
        ATH_MSG_DEBUG("TightNNScore for muon with pT " << mu.pt() << " GeV, eta " << mu.eta() << " is " << score);

        return score;
    }

}  // namespace CP
