/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PunchThroughParticle.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// class header
#include "PunchThroughParticle.h"

// ROOT
#include "TH2F.h"

PunchThroughParticle::PunchThroughParticle(int pdg, bool doAnti):
  m_pdgId(pdg),
  m_doAnti(doAnti),
  m_minEnergy(0.),
  m_maxNum(-1),
  m_numParticlesFactor(1.),
  m_energyFactor(1.),
  m_posAngleFactor(1.),
  m_momAngleFactor(1.),
  m_corrPdg(0),
  m_corrMinEnergy(0),
  m_corrFullEnergy(0),
  m_histCorrLowE(nullptr),
  m_histCorrHighE(nullptr),
  m_corrHistDomains(nullptr),
  m_pdfNumParticles(nullptr),	//does this number ever change?
  m_pdf_pca0(nullptr),
  m_pdf_pca1(nullptr),
  m_pdf_pca2(nullptr),
  m_pdf_pca3(nullptr),
  m_pdf_pca4(nullptr)
{ }

PunchThroughParticle::~PunchThroughParticle()
{
  delete[] m_corrHistDomains;
}

void PunchThroughParticle::setMinEnergy(double minEnergy)
{
  m_minEnergy = minEnergy;
}

void PunchThroughParticle::setMaxNumParticles(int maxNum)
{
  m_maxNum = maxNum;
}

void PunchThroughParticle::setNumParticlesFactor(double numFactor)
{
  m_numParticlesFactor = numFactor;
}

void PunchThroughParticle::setEnergyFactor(double energyFactor)
{
  m_energyFactor = energyFactor;
}

void PunchThroughParticle::setPosAngleFactor(double posAngleFactor)
{
  m_posAngleFactor = posAngleFactor;
}

void PunchThroughParticle::setMomAngleFactor(double momAngleFactor)
{
  m_momAngleFactor = momAngleFactor;
}

void PunchThroughParticle::setNumParticlesPDF(std::unique_ptr<PunchThroughPDFCreator> pdf)
{
  m_pdfNumParticles = std::move(pdf);
}

void PunchThroughParticle::setCorrelation(int corrPdg,
                                               TH2F *histLowE, TH2F *histHighE,
                                               double minCorrelationEnergy,
                                               double fullCorrelationEnergy,
                                               double lowE,
                                               double midE,
                                               double upperE)
{
  m_corrPdg = corrPdg;
  m_histCorrLowE = histLowE;
  m_histCorrHighE = histHighE;
  m_corrMinEnergy = minCorrelationEnergy;
  m_corrFullEnergy = fullCorrelationEnergy;

  m_corrHistDomains = new double [3];
  m_corrHistDomains[0] = lowE;
  m_corrHistDomains[1] = midE;
  m_corrHistDomains[2] = upperE;
}

void PunchThroughParticle::setPCA0PDF(std::unique_ptr<PunchThroughPDFCreator> pdf)
{
  m_pdf_pca0 = std::move(pdf);
}

void PunchThroughParticle::setPCA1PDF(std::unique_ptr<PunchThroughPDFCreator> pdf)
{
  m_pdf_pca1 = std::move(pdf);
}

void PunchThroughParticle::setPCA2PDF(std::unique_ptr<PunchThroughPDFCreator> pdf)
{
  m_pdf_pca2 = std::move(pdf);
}

void PunchThroughParticle::setPCA3PDF(std::unique_ptr<PunchThroughPDFCreator> pdf)
{
  m_pdf_pca3 = std::move(pdf);
}

void PunchThroughParticle::setPCA4PDF(std::unique_ptr<PunchThroughPDFCreator> pdf)
{
  m_pdf_pca4 = std::move(pdf);
}
