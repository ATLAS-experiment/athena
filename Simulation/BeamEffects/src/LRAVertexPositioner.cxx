/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
*/

#include "LRAVertexPositioner.h"

//---

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Vector/LorentzVector.h"

#include "AthenaKernel/RNGWrapper.h"
#include "PathResolver/PathResolver.h"

//---

namespace Simulation
{
  LRAVertexPositioner::LRAVertexPositioner(const std::string &t, const std::string &n, const IInterface *p)
    : base_class(t,n,p)
  {
  };

  //---

  StatusCode LRAVertexPositioner::initialize()
  {
    // Resolve the LRA file name.
    const auto FilePath = PathResolverFindDataFile(m_FileName);
    if(FilePath.empty())
      {
	ATH_MSG_ERROR("Unable to resolve LRAFile: m_FileName = " << (std::string)m_FileName);

	return StatusCode::FAILURE;
      };

    // Open the file...
    m_LRAFile = std::unique_ptr<TFile>(TFile::Open(FilePath.c_str(), "READ"));
    if(!m_LRAFile)
      {
	ATH_MSG_ERROR("Unable to open LRAFile: FilePath = " << FilePath);

	return StatusCode::FAILURE;
      };

    // ... and get the histogram.
    // ToDo: We should re-evaluate holding open the TFile, compared to cloaning out the bits of interest.
    //       Requires access to a real LRA file.
    m_LRAHist = m_LRAFile->Get<TH3F>(((std::string)m_HistName).c_str());
    if(!m_LRAHist)
      {
	ATH_MSG_ERROR("Unable to get LRAHist: FilePath = " << FilePath << ", m_HistName = " << (std::string)m_HistName);

	return StatusCode::FAILURE;
      };

    // ToDo: In the medium term we should look at cleaning up the Lumi groups code, adding whatever the
    //       BS group needs it to do, and getting that into Athena(_externals). The code above could then
    //       be changed to generate the TH3 dynamicaly (maybe simplyfing the configuration of all the tools),
    //       and the future vdM BSFinder could use the unbinned PDF.

    //---

    // Retrieve the tool handle to the RNG service...
    ATH_CHECK(m_RNGService.retrieve());

    // ... and get the RNG engine.
    m_RNGEngine = m_RNGService->getEngine(this, m_RNGStream);
    if (!m_RNGEngine)
      {
	ATH_MSG_ERROR("Unable to get RNGEngine: m_RNGStream = " << (std::string) m_RNGStream);

	return StatusCode::FAILURE;
      };

    //---

    // Cache the bin counts.
    const auto nBinsX = m_LRAHist->GetNbinsX();
    const auto nBinsY = m_LRAHist->GetNbinsY();
    const auto nBinsZ = m_LRAHist->GetNbinsZ();

    // Pre-allocate the space in the vector.
    m_Integral.reserve(nBinsX * nBinsY * nBinsZ);

    /*
      Loop over the bins in the histogram, and store the bin numbers and the running integral into m_Integral.

      Assumes the LRA histograms are "well-formed" PDFs:
        Unit integral.
        No probability in under/over-flow bins
        No negative bins.

      Is ugly, better would be ranges, iota * 3 -> cartesian_product -> for_each, but needs C++23 (No Stinking Loops!!!)
    */
    Double_t Integral = 0.0;
    for(Int_t x = 1; x <= nBinsX; x++)
      {
	for(Int_t y = 1; y <= nBinsY; y++)
	  {
	    for(Int_t z = 1; z <= nBinsZ; z++)
	      {
		Integral += m_LRAHist->GetBinContent(x, y, z);

		m_Integral.emplace_back(std::forward_as_tuple(x, y, z, Integral));
	      };
	  };
      };

    // Cache the axes.
    m_xAxis = m_LRAHist->GetXaxis();
    m_yAxis = m_LRAHist->GetYaxis();
    m_zAxis = m_LRAHist->GetZaxis();

    //---

    // Happy path...
    return StatusCode::SUCCESS;
  };

  StatusCode LRAVertexPositioner::finalize()
  {
    return StatusCode::SUCCESS;
  };

  //---

  CLHEP::HepLorentzVector *LRAVertexPositioner::generate(const EventContext &ctx) const
  {
    // Set the RNG seed and get the engine for this event.
    m_RNGEngine->setSeed(name(), ctx);
    CLHEP::HepRandomEngine *LRAEngine(m_RNGEngine->getEngine(ctx));

    //---

    // Compare the integral of two IntegralTuples.
    auto Comparison = [](const auto &LHS, const auto &RHS)
    {
      // No language place-holders... Yet...
      auto [lx, ly, lz, i] = LHS;
      auto [rx, ry, rz, t] = RHS;

      return(i < t);
    };

    // Get the point a V'th between the bin edges.
    auto GetCoordinate = [](const auto Axis, const auto Bin, const auto V)
    {
      return(Axis->GetBinLowEdge(Bin) + (V * Axis->GetBinWidth(Bin)));
    };

    // Find the x/y/z bin numbers of the interesting bin.
    auto [xBin, yBin, zBin, Integral] = *(std::lower_bound(m_Integral.cbegin(),
							   m_Integral.cend(),
							   std::make_tuple(0, 0, 0, CLHEP::RandFlat::shoot(LRAEngine)),
							   Comparison));

    // Return a point in the bin, caller deletes.
    CLHEP::HepLorentzVector *vertexPosition = new CLHEP::HepLorentzVector(GetCoordinate(m_xAxis, xBin, CLHEP::RandFlat::shoot(LRAEngine)),
									  GetCoordinate(m_yAxis, yBin, CLHEP::RandFlat::shoot(LRAEngine)),
									  GetCoordinate(m_zAxis, zBin, CLHEP::RandFlat::shoot(LRAEngine)),
									  0.0);

    // ToDo: This uses only the TH3 PDF to position the vertex, this should be expanded to include:
    //         x/y/z translations.
    //         Slant angles.
    //         Euler angle/quaternion/something for arbiraty rotations (for non-factorisable PDFs)?

    //---

    return(vertexPosition);
  };
};
