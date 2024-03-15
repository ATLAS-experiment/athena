/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/



#include "egammaMVACalib/egammaMVACalibTool.h"
#include "EgammaAnalysisInterfaces/IegammaMVACalibTool.h"
#include <string>
#include "xAODTrigRinger/TrigRingerRingsContainer.h"

#include "TFile.h"
#include "TMath.h"
#include "TObjString.h"
#include "TTree.h"
#include "TClass.h"

#include "PathResolver/PathResolver.h"
#include "CxxUtils/checker_macros.h"
#include "AsgTools/AsgTool.h"

#ifndef TRIGFASTCALIBWITHRINGS_H
#define TRIGFASTCALIBWITHRINGS_H

class TrigFastCalibWithRings: public asg::AsgTool

{

    public:
        TrigFastCalibWithRings(const std::string& type, const std::string& myname, const IInterface* parent);
        ~TrigFastCalibWithRings();
        StatusCode initialize() final;
        StatusCode execute() const;
        float makeCalibWRings(const EventContext& ctx) const;
        StatusCode setupBDTFastCalo(const std::string& fileName); 
        static const TString& getString(TObject* obj);
        StatusCode checkRings(const EventContext& ctx ) const;


    private:
        Gaudi::Property<std::string> m_CalibPath{this, "CalibPath", "",
                                       "Path to BDT File"};
        SG::ReadHandleKey<xAOD::TrigRingerRingsContainer> m_ringerKey{this, "RingerKey" , "HLT_FastCaloRinger", ""}; 
        /// A TH2Poly used to extract bin numbers. Note there is an offset of 1
        std::unique_ptr<TH2Poly> m_hPoly;

        /// Where the BDTs are stored
        std::vector<MVAUtils::BDT> m_BDTs;

        /// shifts formulas
        std::vector<TFormula> m_shifts;

   };

#endif // TRIGFASTCALIBWITHRINGS_H