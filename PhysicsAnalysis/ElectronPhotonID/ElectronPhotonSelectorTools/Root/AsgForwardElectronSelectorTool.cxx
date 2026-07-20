/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
    @class AsgForwardElectronSelectorTool
    @brief Tool to apply DNN-based ID to forward electrons for Run 4 using ITk and HGTD variables.
    Internal Note: https://cds.cern.ch/record/2922175

    Workflow:
      1. Call AsgForwardElectronCalibrationTool to get the calibrated pT in this forward region
      2. Apply Linear Regression to decorrelate the 7 shower shape moments from pT
      3. Run DNN via lwtnn (MinMax scaler already into JSON as a batch norm layer)
      4. Apply pT- and eta-dependent working point cut on the DNN score

    @author Mathis Dubau (LAPP)
    @date   Feb 2026

    Based on ElectronDNNCalculator and AsgForwardElectronLikelihoodTool

*/


#include "ElectronPhotonSelectorTools/AsgForwardElectronSelectorTool.h"

#include "PathResolver/PathResolver.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Egamma.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "AsgTools/CurrentContext.h"

#include "lwtnn/parse_json.hh"

#include <fstream>
#include <cmath>





// ============================================================================
// Model constants
// ============================================================================

// --- Linear regression ---
// One coefficient and one intercept per shower shape variable (7) per eta bin
// Order: ENG_FRAC_MAX, LONGITUDINAL, SECOND_LAMBDA, LATERAL,
//        SECOND_R, CENTER_LAMBDA, SECOND_ENG_DENS

static const double s_lrCoeff[3][7] = {
  // Bin 0: 2.5 < |eta| <= 2.7
  {  5.7626170e-07,  4.7577697e-07, -1.3823183e-03, -9.1804844e-07,
    -5.5160094e-03,  4.0462506e-05,  2.8797556e-06 },
  // Bin 1: 2.7 < |eta| <= 3.2
  {  4.9091057e-07,  4.2073955e-07,  2.5762843e-03, -7.4278881e-07,
    -3.6089115e-03, -6.4899518e-06,  7.7993927e-06 },
  // Bin 2: 3.2 < |eta| <= 4.0
  {  2.9304172e-07, -1.0814731e-06, -1.6405748e-02, -3.3167328e-07,
    -7.3823769e-04, -1.7326717e-04,  9.6527780e-05 }
};

static const double s_lrIntercept[3][7] = {
  // Bin 0: 2.5 < |eta| <= 2.7
  {  5.3479767e-01,  1.1612210e-01,  2.2856726e+03,  4.5776847e-01,
     1.8551936e+03,  2.1710736e+02, -8.0515645e-02 },
  // Bin 1: 2.7 < |eta| <= 3.2
  {  5.3335690e-01,  7.2528444e-02,  9.1885895e+02,  3.8494921e-01,
     1.2577064e+03,  1.9783354e+02, -1.9677971e-01 },
  // Bin 2: 3.2 < |eta| <= 4.0
  {  5.7937855e-01,  3.4406906e-01,  5.7875332e+03,  2.8120410e-01,
     6.1775574e+02,  2.4003990e+02, -2.6005685e+00 }
};

// --- pT bin edges [MeV] - 12 bins ---
static const double s_ptEdges[13] = {
    5000.,  15000.,  20000.,  25000.,  30000.,  35000.,
   40000.,  45000.,  50000.,  60000.,  80000., 150000., 500000.
};

// --- DNN score cuts [eta_bin][pt_bin][wp_index] ---
// wp_index: 
//          0 = Loose = 90% 
//          1 = Medium = 80% 
//          2 = Tight = 70%
// pt_bin order matches s_ptEdges:
//   [5,15],[15,20],[20,25],[25,30],[30,35],[35,40],
//   [40,45],[45,50],[50,60],[60,80],[80,150],[150,500] GeV

static const double s_cuts[3][12][3] = {
  // Bin 0: 2.5 < |eta| <= 2.7
  {
    { 0.003434060638577299,  0.010929810583837518,  0.028987138649460953 }, // [5,15]
    { 0.009150723693892365,  0.10347994714975359,   0.28577716946601867  }, // [15,20]
    { 0.1186611831188202,    0.43142691254615784,   0.7045282602310181   }, // [20,25]
    { 0.3128826916217804,    0.6842968463897705,    0.8462172150611877   }, // [25,30]
    { 0.6381388902664185,    0.883872103691101,     0.9505972623825073   }, // [30,35]
    { 0.815248429775238,     0.9427814483642578,    0.9722608327865601   }, // [35,40]
    { 0.8642481029033661,    0.9566433548927307,    0.9769171953201294   }, // [40,45]
    { 0.859777855873108,     0.9566103696823121,    0.9771971702575684   }, // [45,50]
    { 0.8481978237628938,    0.9565577507019043,    0.9773394107818604   }, // [50,60]
    { 0.8359241724014282,    0.9572678208351135,    0.9781455874443055   }, // [60,80]
    { 0.8056963920593262,    0.9453475475311279,    0.9758875846862793   }, // [80,150]
    { 0.8081184050246257,    0.9615500409750506,    0.9705579706236049   }  // [150,500]
  },
  // Bin 1: 2.7 < |eta| <= 3.2
  {
    { 0.0024538897448333936, 0.006224703698926278,  0.01602673525486277  }, // [5,15]
    { 0.029491694644093513,  0.1092548817396164,    0.2902836352586746   }, // [15,20]
    { 0.20487691462039948,   0.5585798382759094,    0.789155101776123    }, // [20,25]
    { 0.5053084492683411,    0.8155142784118653,    0.9119235754013061   }, // [25,30]
    { 0.7210912346839905,    0.9057818174362183,    0.9574183464050293   }, // [30,35]
    { 0.8253453254699707,    0.940361762046814,     0.9731698989868164   }, // [35,40]
    { 0.8629356622695923,    0.9526293277740479,    0.9774575233459473   }, // [40,45]
    { 0.8681894183158875,    0.954381775856018,     0.9784879982471466   }, // [45,50]
    { 0.8598942279815674,    0.9535421252250672,    0.9780020833015441   }, // [50,60]
    { 0.8658008813858032,    0.9549649477005004,    0.9777071833610534   }, // [60,80]
    { 0.8357489705085754,    0.9435908794403076,    0.971304714679718    }, // [80,150]
    { 0.8023493699313187,    0.9416966809437475,    0.9669603430019074   }  // [150,500]
  },
  // Bin 2: 3.2 < |eta| <= 4.0
  {
    { 0.0007497479446309254, 0.010757287517960587,  0.004936974379270967 }, // [5,15]
    { 0.014802457392215732,  0.042617067694664,     0.0812974080443382   }, // [15,20]
    { 0.07443902641534805,   0.23044122755527496,   0.45949786901474     }, // [20,25]
    { 0.34181419014930725,   0.6247072815895081,    0.7751210331916809   }, // [25,30]
    { 0.5459890365600586,    0.7635000348091125,    0.8726731538772583   }, // [30,35]
    { 0.6520047783851624,    0.8353232741355896,    0.9097822308540344   }, // [35,40]
    { 0.7230071187019348,    0.8757749557495117,    0.9323666334152222   }, // [40,45]
    { 0.7161759436130524,    0.8738378882408142,    0.9322998225688934   }, // [45,50]
    { 0.6740142107009888,    0.8602323532104492,    0.9265623986721039   }, // [50,60]
    { 0.7804294466972351,    0.9046212434768677,    0.9510670781135558   }, // [60,80]
    { 0.8492123007774353,    0.9328805446624756,    0.9633602142333985   }, // [80,150]
    { 0.7497747273127531,    0.8820413108674948,    0.9336298814733452   }  // [150,500]
  }
};


//=============================================================================
// Standard constructor
//=============================================================================
AsgForwardElectronSelectorTool::AsgForwardElectronSelectorTool(
    const std::string& myname)
  : AsgTool(myname)
{
}

//=============================================================================
// Standard destructor
//=============================================================================
AsgForwardElectronSelectorTool::~AsgForwardElectronSelectorTool() = default;

// ============================================================================
// Initialise
// ============================================================================
StatusCode AsgForwardElectronSelectorTool::initialize()
{
  // Resolve working point index
  if      (m_workingPoint == "Loose")  m_wpIndex = 0;
  else if (m_workingPoint == "Medium") m_wpIndex = 1;
  else if (m_workingPoint == "Tight")  m_wpIndex = 2;
  else {
    ATH_MSG_ERROR("Unknown Working Point '" << m_workingPoint
                  << "'. Choose Loose, Medium, or Tight.");
    return StatusCode::FAILURE;
  }

  // Retrieve the calibration tool
  ATH_CHECK(m_calibTool.retrieve());

  // Sanity checks
  if (m_modelFiles.size() != 3) {
    ATH_MSG_ERROR("Exactly 3 model files expected (one per eta bin): "
                  "[2.5,2.7], [2.7,3.2], [3.2,4.0]). Only got "
                  << m_modelFiles.size());
    return StatusCode::FAILURE;
  }

  // Fixed variable names - must match the lwtnn JSON input
  m_variables = {
    // Calo eta and Phi
    "x1_calo_eta",          
    "x2_calo_phi",
    // ITk eta and Phi
    "x3_track_eta", 
    "x4_track_phi",
    // HGTD time
    "x5_time",
    // ITk hit counts
    "x6_pixels",            
    "x7_strips",
    // Shower shape moments 
    "x8_ENG_FRAC_MAX", 
    "x9_LONGITUDINAL",
    "x10_SECOND_LAMBDA",    
    "x11_LATERAL",
    "x12_SECOND_R",         
    "x13_CENTER_LAMBDA",
    "x14_SECOND_ENG_DENS",
    // Track-cluster matching variables
    "x15_delta_eta2",       
    "x16_delta_phi2",
    "x17_delta_phi_rescaled2", 
    "x18_delta_phi_last",
    // Calo energy fractions in each layer
    "x19_calo_frac_EM_1",
    "x20_calo_frac_EM_2",  
    "x21_calo_frac_EM_3",
    "x22_calo_frac_HAD_0",  
    "x23_calo_frac_HAD_1",
    "x24_calo_frac_HAD_2",  
    "x25_calo_frac_HAD_3"
  };
  // More informations on Table 3 in Internal Note: https://cds.cern.ch/record/2922175

  // Load one JSON per eta bin
  m_graphs.reserve(3);
  for (const auto& model : m_modelFiles) {
    const std::string path = PathResolverFindCalibFile(model);
    if (path.empty()) {
      ATH_MSG_ERROR("Could not locate: " << model);
      return StatusCode::FAILURE;
    }
    std::ifstream dnn_json(path);
    auto parsed = lwt::parse_json_graph(dnn_json);
    m_graphs.emplace_back(
      std::make_unique<lwt::LightweightGraph>(parsed));
    ATH_MSG_INFO("Loaded ID model for bin "
                 << m_graphs.size() - 1 << ": " << path);
  }

  // Register the single cut in AcceptInfo
  m_acceptInfo.addCut("PassDNNScore",
                      "Electron passes the DNN score cut for the configured WP");

  ATH_MSG_INFO("AsgForwardElectronSelectorTool initialised. "
               << "WorkingPoint=" << m_workingPoint);
  return StatusCode::SUCCESS;
}

// ============================================================================
// getAcceptInfo
// ============================================================================
const asg::AcceptInfo&
AsgForwardElectronSelectorTool::getAcceptInfo() const
{
  return m_acceptInfo;
}

// ============================================================================
// accept - IParticle
// ============================================================================
asg::AcceptData
AsgForwardElectronSelectorTool::accept(const xAOD::IParticle* part) const
{
  // Backwards compatibility
  return accept(Gaudi::Hive::currentContext(), part);
}

asg::AcceptData
AsgForwardElectronSelectorTool::accept(const EventContext& ctx,
				         const xAOD::IParticle* part) const
{
  if (part->type() == xAOD::Type::Electron) {
    const xAOD::Electron* el = static_cast<const xAOD::Electron*>(part);
    return accept(ctx, el);
  }
  ATH_MSG_ERROR("Input is not an electron.");
  return asg::AcceptData(&m_acceptInfo);
}


// ============================================================================
// accept - Electron overloads
// ============================================================================
asg::AcceptData
AsgForwardElectronSelectorTool::accept(const EventContext& ctx,
				       const xAOD::Electron* eg) const
{
  return accept(ctx, eg, -99);
}

asg::AcceptData
AsgForwardElectronSelectorTool::accept(const EventContext& ctx,
                                        const xAOD::Electron* eg,
                                        double /*mu*/) const
{
  asg::AcceptData acceptData(&m_acceptInfo);

  double score{-999.}, calibPt{-999.};
  if (!calculateWithCalibPt(ctx, eg, score, calibPt)) return acceptData;

  const xAOD::CaloCluster* cluster = eg->caloCluster();
  const int etaBin = getEtaBin(std::abs(cluster->eta()));
  const int ptBin  = getPtBin(calibPt);
  if (etaBin < 0 || ptBin < 0) return acceptData;

  acceptData.setCutResult("PassDNNScore",
                          score >= s_cuts[etaBin][ptBin][m_wpIndex]);
  return acceptData;
}

// ============================================================================
// accept - Egamma overloads
// ============================================================================
asg::AcceptData
AsgForwardElectronSelectorTool::accept(const EventContext& ctx,
                                        const xAOD::Egamma* eg) const
{
  return accept(ctx, eg, -99);
}

asg::AcceptData
AsgForwardElectronSelectorTool::accept(const EventContext& ctx,
                                        const xAOD::Egamma* eg,
				       double mu) const
{
  const xAOD::Electron* el = dynamic_cast<const xAOD::Electron*>(eg);
  if (!el) {
    ATH_MSG_ERROR("Input is not an electron.");
    return asg::AcceptData(&m_acceptInfo);
  }
  return accept(ctx, el, mu);
}

// ============================================================================
// calculate - IParticle overload
// ============================================================================
double
AsgForwardElectronSelectorTool::calculate(const EventContext& ctx,
                                           const xAOD::IParticle* part) const
{
  if (part->type() == xAOD::Type::Electron) {
    const xAOD::Electron* el = static_cast<const xAOD::Electron*>(part);
    return calculate(ctx, el);
  }
  ATH_MSG_ERROR("Input is not an electron.");
  return -999.;
}

// ============================================================================
// calculate - Electron overloads
// ============================================================================
double
AsgForwardElectronSelectorTool::calculate(const EventContext& ctx,
                                           const xAOD::Electron* eg) const
{
  return calculate(ctx, eg, -99);
}

double
AsgForwardElectronSelectorTool::calculate(const EventContext& ctx,
                                           const xAOD::Electron* eg,
					  double /*mu*/) const
{
  double score{-999.}, calibPt{-999.};
  calculateWithCalibPt(ctx, eg, score, calibPt);

  return score;
}

// ============================================================================
// calculate - Egamma overloads
// ============================================================================
double
AsgForwardElectronSelectorTool::calculate(const EventContext& ctx,
                                           const xAOD::Egamma* eg) const
{
  return calculate(ctx, eg, -99);
}

double
AsgForwardElectronSelectorTool::calculate(const EventContext& ctx,
                                           const xAOD::Egamma* eg,
                                           double mu) const
{
  const xAOD::Electron* el = dynamic_cast<const xAOD::Electron*>(eg);
  if (!el) { ATH_MSG_ERROR("Input is not an electron."); return -999.; }
  return calculate(ctx, el, mu);
}

// ============================================================================
// calculateWithCalibPt
// ============================================================================
bool
AsgForwardElectronSelectorTool::calculateWithCalibPt(const EventContext& /*ctx*/,
						     const xAOD::Electron* eg,
                                                      double& score,
						     double& calibPt) const
{
  score = -999.; calibPt = -999.;

  if (!eg) {
    ATH_MSG_ERROR("Failed, no Electron object.");
    return false; 
  }

  const xAOD::CaloCluster* cluster = eg->caloCluster();
  if (!cluster) {
    ATH_MSG_WARNING("Failed, no cluster.");
    return false; 
  }

  // ITk electron must have a track
  const xAOD::TrackParticle* track = eg->trackParticle();
  if (!track) {
    ATH_MSG_WARNING("Failed, no track.");
    return false;
  }

  const double absEta = std::abs(cluster->eta());
  const int etaBin = getEtaBin(absEta);
  if (etaBin < 0) {
    ATH_MSG_WARNING("Electron |eta|=" << absEta
                    << " is outside allowed range.");
    return false;
  }

  // Step 1 - Usage of allready calibrated pT
  calibPt=eg->pt();

  // Step 2 - extract inputs with LR decorrelation
  std::vector<double> inputs;
  if (!getInputs(eg, calibPt, etaBin, inputs)) return false;

  // Step 3 - run DNN
  std::map<std::string, std::map<std::string, double>> inputMap;
  for (size_t i = 0; i < m_variables.size(); ++i)
    inputMap["node_0"][m_variables[i]] = inputs[i];

  score = m_graphs[etaBin]->compute(inputMap).begin()->second;
  // Round 6 digits due to lwtnn mismatch
  score = std::round(score * 1e6) / 1e6;
  return true;
}

// ============================================================================
// calculateMultipleOutputs
// ============================================================================
std::vector<float>
AsgForwardElectronSelectorTool::calculateMultipleOutputs(
    const EventContext& ctx,
    const xAOD::Electron* eg,
    double mu) const
{
  return { static_cast<float>(calculate(ctx, eg, mu)) };
}

// ============================================================================
// getOperatingPointName
// ============================================================================
std::string
AsgForwardElectronSelectorTool::getOperatingPointName() const
{
  return m_workingPoint;
}

// ============================================================================
// getEtaBin
// ============================================================================
int AsgForwardElectronSelectorTool::getEtaBin(double absEta) 
{
  // Convention: x1 < |eta| <= x2
  if (absEta > 2.5 && absEta <= 2.7) return 0;
  if (absEta > 2.7 && absEta <= 3.2) return 1;
  if (absEta > 3.2 && absEta <= 4.0) return 2;
  return -1;
}

// ============================================================================
// getPtBin
// ============================================================================
int AsgForwardElectronSelectorTool::getPtBin(double calibPt) 
{
  constexpr int nBins = 12;
  for (int i = 0; i < nBins; ++i) {
    if (calibPt >= s_ptEdges[i] && calibPt < s_ptEdges[i + 1])
      return i;
  }
  return -1;
}

// ============================================================================
// getInputs
// ============================================================================
bool AsgForwardElectronSelectorTool::getInputs(const xAOD::Electron* eg,
                                                double calibPt,
                                                int etaBin,
                                                std::vector<double>& inputs) const
{
  inputs.clear();
  inputs.reserve(25);

  const xAOD::CaloCluster*   cluster = eg->caloCluster();
  const xAOD::TrackParticle* track   = eg->trackParticle();

  if (!cluster) { ATH_MSG_ERROR("No CaloCluster.");   return false; }
  if (!track)   { ATH_MSG_ERROR("No TrackParticle."); return false; }

  // x1, x2 = calo eta and phi
  inputs.push_back(static_cast<float>(cluster->eta()));
  inputs.push_back(static_cast<float>(cluster->phi()));

  // x3, x4 = track eta and phi
  inputs.push_back(static_cast<float>(track->eta()));
  inputs.push_back(static_cast<float>(track->phi()));

  // x5 = HGTD time
  static const SG::AuxElement::Accessor<uint8_t> accValid("hasValidTime");
  if (accValid.isAvailable(*track))
    {
      if (accValid(*track))
	{
	  inputs.push_back(static_cast<float>(track->time()));
	}
      else
	{
	  ATH_MSG_DEBUG("No valid time for the track while doing track->time()" );
	  inputs.push_back(-99);
	}                                                                                                      
    }
  else
    {
      ATH_MSG_ERROR("No available time for the track" );
    }
  
  // x6, x7 = ITk hit counts
  inputs.push_back(static_cast<float>(eg->trackParticleSummaryIntValue(xAOD::numberOfPixelHits)));
  inputs.push_back(static_cast<float>(eg->trackParticleSummaryIntValue(xAOD::numberOfSCTHits)));

  // x8 to x14 = 7 calorimeter shower-shape moments
  auto getMoment = [&](xAOD::CaloCluster::MomentType type,
                       const char* name) -> float {
    double val{0.};
    if (!cluster->retrieveMoment(type, val))
      ATH_MSG_WARNING("Could not retrieve calo moment: " << name);
    return val;
  };

  // Retrieve raw moments
  const double rawMoments[7] = {
    getMoment(xAOD::CaloCluster::ENG_FRAC_MAX,    "ENG_FRAC_MAX"),
    getMoment(xAOD::CaloCluster::LONGITUDINAL,    "LONGITUDINAL"),
    getMoment(xAOD::CaloCluster::SECOND_LAMBDA,   "SECOND_LAMBDA"),
    getMoment(xAOD::CaloCluster::LATERAL,         "LATERAL"),
    getMoment(xAOD::CaloCluster::SECOND_R,        "SECOND_R"),
    getMoment(xAOD::CaloCluster::CENTER_LAMBDA,   "CENTER_LAMBDA"),
    getMoment(xAOD::CaloCluster::SECOND_ENG_DENS, "SECOND_ENG_DENS")
  };

  // Apply LR decorrelation: moment_decorr = moment_raw - (coeff * calibPt + intercept)
  for (int i = 0; i < 7; ++i)
    inputs.push_back(rawMoments[i]
                    - (s_lrCoeff[etaBin][i] * calibPt + s_lrIntercept[etaBin][i]));


  // x15 to x18 = track-calo matching
  auto getMatch = [&](xAOD::EgammaParameters::TrackCaloMatchType type,
                      const char* name) -> float {
    float val{0.f};
    if (!eg->trackCaloMatchValue(val, type))
      ATH_MSG_WARNING("Could not retrieve track-calo match: " << name);
    return val;
  };

  inputs.push_back(getMatch(xAOD::EgammaParameters::deltaEta2,
                            "delta_eta2"));
  inputs.push_back(getMatch(xAOD::EgammaParameters::deltaPhi2,
                            "delta_phi2"));
  inputs.push_back(getMatch(xAOD::EgammaParameters::deltaPhiRescaled2,
                            "delta_phi_rescaled2"));
  inputs.push_back(getMatch(xAOD::EgammaParameters::deltaPhiFromLastMeasurement,
                            "delta_phi_last"));

  // x19 to x25 = Derived calorimeter energy fractions
  // Prevent division by zero
  const double caloE   = cluster->e();
  const double inv_E   = (caloE != 0.) ? 1. / caloE : 0.;

  using CS = CaloSampling::CaloSample;

  // EM fractions: 
  //   f_i^EM = energyBE(i) / caloE  for i in {1,2,3}
  inputs.push_back(static_cast<float>(cluster->energyBE(1) * inv_E));
  inputs.push_back(static_cast<float>(cluster->energyBE(2) * inv_E));
  inputs.push_back(static_cast<float>(cluster->energyBE(3) * inv_E));

  // HAD fractions:
  //   f_0^HAD = HEC0 / caloE
  //   f_i^HAD = (HEC_i + FCAL_(i-1)) / caloE  for i in {1,2,3}
  inputs.push_back(static_cast<float>(
    cluster->eSample(static_cast<CS>(CaloSampling::HEC0)) * inv_E));
  inputs.push_back(static_cast<float>(
    (cluster->eSample(static_cast<CS>(CaloSampling::HEC1)) +
     cluster->eSample(static_cast<CS>(CaloSampling::FCAL0))) * inv_E));
  inputs.push_back(static_cast<float>(
    (cluster->eSample(static_cast<CS>(CaloSampling::HEC2)) +
     cluster->eSample(static_cast<CS>(CaloSampling::FCAL1))) * inv_E));
  inputs.push_back(static_cast<float>(
    (cluster->eSample(static_cast<CS>(CaloSampling::HEC3)) +
     cluster->eSample(static_cast<CS>(CaloSampling::FCAL2))) * inv_E));

  return true;
}
