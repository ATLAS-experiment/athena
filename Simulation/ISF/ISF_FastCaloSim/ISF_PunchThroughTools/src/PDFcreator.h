/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PDFcreator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_PUNCHTHROUGHTOOLS_SRC_PDFCREATOR_H
#define ISF_PUNCHTHROUGHTOOLS_SRC_PDFCREATOR_H

// Athena Base
#include "AthenaKernel/IAtRndmGenSvc.h"

// ROOT includes
#include "TH1F.h"
#include "TH2F.h"

//ISF includes
#include "ISF_FastCaloSimEvent/TFCS1DFunction.h"




namespace ISF
{
  /** @class PDFcreator

      Creates random numbers with a distribution given as ROOT TF1.
      The TF1 function parameters will be retrieved from a histogram given by addPar.

      @author  Elmar Ritsch <Elmar.Ritsch@cern.ch>
      @maintainer/updater Thomas Carter <thomas.michael.carter@cern.ch>
  */

  class PDFcreator
  {

  public:
    /** construct the class with a given TF1 and a random engine */
    PDFcreator() {};

    ~PDFcreator();

    /** all following is used to set up the class */
    void setName( const std::string & PDFname ){ m_name = PDFname; }; //get the pdf's name
    void addToEnergyEtaHist1DMap(int energy, int etaMin, TH1 *hist); //add entry to map linking energy, eta window and histogram

    /** get the random value with this method, by providing the input parameters */
    double getRand(CLHEP::HepRandomEngine* rndmEngine, const std::vector<int>& inputParameters) const;
    const std::string& getName() const {return m_name;};

  private:
    std::string                         m_name;               //!< Give pdf a name for debug purposes
    std::map< int , std::map< int, TH1*> > m_energy_eta_hists1D; //!< map of energies to map of eta ranges to 1D histograms

  };
}

#endif
