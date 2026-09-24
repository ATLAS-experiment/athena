/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


//
// includes
//

#include <regex>
#include <AsgAnalysisAlgorithms/SysListDumperAlg.h>
#include <TH1.h>

//
// method implementations
//

namespace CP
{

  StatusCode SysListDumperAlg ::
  initialize ()
  {
    if (m_histogramName.empty())
    {
      ANA_MSG_ERROR ("histogram name should not be empty");
      return StatusCode::FAILURE;
    }

    try
    {
      std::regex expr (m_regex.value());
    } catch (const std::regex_error& e)
    {
      ANA_MSG_ERROR ("invalid systematics regex '" << m_regex.value() << "': " << e.what());
      return StatusCode::FAILURE;
    }

    ANA_CHECK (m_systematicsService.retrieve());

    return StatusCode::SUCCESS;
  }



  StatusCode SysListDumperAlg ::
  execute (const EventContext& /*ctx*/)
  {
    if (!m_firstEvent)
    {
      return StatusCode::SUCCESS;
    }

    m_firstEvent = false;

    const std::vector<CP::SystematicSet> systematics = makeSystematicsVector (m_regex);
    if (systematics.empty()) {
      return StatusCode::SUCCESS;
    }

    ANA_CHECK (book (TH1F (m_histogramName.value().c_str(), "systematics", systematics.size(), 0, systematics.size())));
    TH1 *histogram = hist (m_histogramName);

    int i = 1;
    const std::string sysSignatureStr{"%SYS%"};
    for (const SystematicSet& sys : systematics)
    {
      std::string name;
      ANA_CHECK (m_systematicsService->makeSystematicsName (name, sysSignatureStr, sys));

      histogram->GetXaxis()->SetBinLabel(i, name.c_str());
      i++;
    }

    return StatusCode::SUCCESS;
  }



  std::vector<CP::SystematicSet> SysListDumperAlg ::
  makeSystematicsVector (const std::string &regex) const
  {
    std::vector<CP::SystematicSet> inputVector = m_systematicsService->makeSystematicsVector ();
    if (regex.empty())
    {
      return inputVector;
    }

    std::vector<CP::SystematicSet> systematicsVector;
    std::regex expr (regex);
    for (const CP::SystematicSet& sys : inputVector)
    {
      if (regex_match (sys.name(), expr))
      {
        systematicsVector.push_back (sys);
      }
    }
    return systematicsVector;
  }


  StatusCode SysListDumperAlg ::
  finalize ()
  {
    const std::vector<CP::SystematicSet> systematics = makeSystematicsVector (m_regex);
    if (systematics.empty()) {
      ANA_MSG_INFO ("systematics regex '" << m_regex.value() << "' did not match any systematics");
      return StatusCode::SUCCESS;
    }

    ANA_MSG_INFO("Systematics regex '" << m_regex.value() << "' matched:");
    for(const CP::SystematicSet& mysys : systematics) {
      ANA_MSG_INFO ("  '" << mysys.name() << "'");
    }
    return StatusCode::SUCCESS;
  }
}
