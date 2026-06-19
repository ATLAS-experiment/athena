/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CalibrationDataInterfaceTester.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////


#include "CalibrationDataInterface/CalibrationDataInterfaceTester.h"
#include "JetEvent/JetCollection.h"
#include "JetEvent/Jet.h"
#include "JetTagInfo/TruthInfo.h"
#include "AsgTools/AnaToolHandle.h"
#include <algorithm>
#include <utility>
#include <cctype>

using std::string;
using Analysis::CalibrationDataInterfaceTool;

//================ Constructor =================================================

Analysis::CalibrationDataInterfaceTester::CalibrationDataInterfaceTester(const std::string& name, ISvcLocator* pSvcLocator)
  :   AthAlgorithm(name, pSvcLocator)
{
  //  template for property declaration
  declareProperty("JetCollection",          m_jetCollection,          "name of the jet collection to be used");
  declareProperty("Tagger",                 m_tagger,                 "name of the tagging algorithm to be used");
  declareProperty("OperatingPoint",         m_operatingPoint,         "name of the tagging algorithm operating point to be used");
  declareProperty("CalibrationUncertainty", m_calibrationUncertainty, "calibration uncertainty result");
  declareProperty("CalibrationInterface",   m_calibrationInterface,   "interface tool instance name");
}

//================ Initialisation =================================================

StatusCode Analysis::CalibrationDataInterfaceTester::initialize()
{


  ATH_CHECK(m_JetCollectionKey.initialize());

  StatusCode sc = m_calibrationInterface.retrieve();
  if (sc.isFailure()) {
      ATH_MSG_ERROR("initialize() in " << name() << ": unable to retrieve "
		    << "calibration interface tool!");
    return sc;
  }

  std::transform(m_calibrationUncertainty.begin(), m_calibrationUncertainty.end(),
		 m_calibrationUncertainty.begin(), tolower);
  if (m_calibrationUncertainty.find("total") != string::npos) {
    m_uncertaintyType = CalibrationDataInterfaceTool::Total;
  } else if (m_calibrationUncertainty.find("syst") != string::npos) {
    m_uncertaintyType = CalibrationDataInterfaceTool::Systematic;
  } else if (m_calibrationUncertainty.find("stat") != string::npos) {
    m_uncertaintyType = CalibrationDataInterfaceTool::Statistical;
  } else {
    m_uncertaintyType = CalibrationDataInterfaceTool::None;
  }
  
  ATH_MSG_INFO("initialize() successful in " << name());
  return StatusCode::SUCCESS;
}

//================ Execution ====================================================

StatusCode Analysis::CalibrationDataInterfaceTester::execute(const EventContext& ctx)
{
  // Code entered here will be executed once per event

  // retrieve the desired jet collection
  EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<JetCollection> jets(m_JetCollectionKey, ctx);
  ATH_CHECK(jets.isValid());

  int njtag = (*jets).size();
  ATH_MSG_INFO("JetCollection " << m_jetCollection
	       << " found with " << njtag << " jets.");

  for (auto jetItr : jets) {
    // --- get btagging weight for the tagger under consideration
    double weight = (*jetItr)->getFlavourTagWeight(m_tagger);

    // --- get the true label of the jet from MC Truth:
    const Analysis::TruthInfo* mcinfo = (*jetItr)->tagInfo<Analysis::TruthInfo>("TruthInfo");
    if (! mcinfo) {
      ATH_MSG_DEBUG("could not find TruthInfo for matching jet");
      continue;
    }
    string label = mcinfo->jetTruthLabel();

    std::pair<double, double> sfResult =
      m_calibrationInterface->getScaleFactor(**jetItr, label, m_operatingPoint, m_uncertaintyType);
    std::pair<double, double> effResult =
      m_calibrationInterface->getEfficiency(**jetItr, label, m_operatingPoint, m_uncertaintyType);

    ATH_MSG_DEBUG("Jet with " << m_tagger << " weight: " << weight);
    ATH_MSG_DEBUG(" label: " << label);
    ATH_MSG_DEBUG(" SF (unc.): " << sfResult.first
		 << "(" << sfResult.second << ")");
    ATH_MSG_DEBUG(" eff (unc.): " << effResult.first << "(" << effResult.second << ")");

  }

  return StatusCode::SUCCESS;
}

//============================================================================================

