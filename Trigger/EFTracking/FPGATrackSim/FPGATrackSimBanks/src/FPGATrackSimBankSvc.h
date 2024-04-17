// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef TRIGFPGATrackSimBANKSVC_H
#define TRIGFPGATrackSimBANKSVC_H

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/Service.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimBanks/FPGATrackSimFitConstantBank.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorSlice.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"


class FPGATrackSimBankSvc : public extends< AthService, IFPGATrackSimBankSvc >
{
    public:

        ///////////////////////////////////////////////////////////////////////
        // AthService

        FPGATrackSimBankSvc(const std::string& name, ISvcLocator* svc);
        virtual ~FPGATrackSimBankSvc() = default;

        virtual StatusCode initialize() override;

        ///////////////////////////////////////////////////////////////////////
        // IFPGATrackSimBankSvc

        virtual const FPGATrackSimFitConstantBank* FitConstantBank_1st(int missedPlane = -1) override;
        virtual const FPGATrackSimFitConstantBank* FitConstantBank_2nd(int missedPlane = -1) override;
        virtual const FPGATrackSimSectorBank* SectorBank_1st() override;
        virtual const FPGATrackSimSectorBank* SectorBank_2nd() override;
        virtual const FPGATrackSimSectorSlice* SectorSlice() override;

    private:
	// Gaudi properties

	Gaudi::Property<std::string> m_path_constants_1st {this, "constants_1st", "", "Path of the 1st stage constants"};
	Gaudi::Property<std::string> m_path_constants_2nd {this, "constants_2nd", "", "Path of the 2nd stage constants"};
	Gaudi::Property<std::string> m_path_sectorbank_1st {this, "sectorBank_1st", "", "Path of the 1st stage sector bank"};
	Gaudi::Property<std::string> m_path_sectorbank_2nd {this, "sectorBank_2nd", "", "Path of the 2nd stage sectorbank"};
	Gaudi::Property<std::string> m_path_sectorslices {this, "sectorSlices", "", "Path of the sector slices"};
	Gaudi::Property<std::string> m_bankTypes {this, "BankType", "FILE", "FILE or COOL (COOL/DB to be implemented)"};
	Gaudi::Property<int> m_ncoords_1st {this, "NCoords_1st", 10, "Number of 1st stage coordinates"};
	Gaudi::Property<int> m_ncoords_2nd {this, "NCoords_2nd", 18, "Number of 2nd stage coordinates"};
	Gaudi::Property<std::string> m_path_NNconstants {this, "m_path_NNconstants", "", "Path for NN constants"};
	Gaudi::Property<std::vector<std::string> > m_path_constants_1st_noguess {this, "constantsNoGuess_1st", {}, "Path of the 1st stage constants without guessing"};
	Gaudi::Property<std::vector<std::string> > m_path_constants_2nd_noguess {this, "constantsNoGuess_2nd", {}, "Path of the 2nd stage constants without guessing"};
	Gaudi::Property<bool> m_absQOverPtBinning { this, "qptAbsBinning", false, "Controls whether or not to interpret the bins as q/pt or |q/pt|"};
	Gaudi::Property<std::vector<double> > m_qOverPtBins { this, "sectorQPtBins", {}, "q/pt bins for sector definition"};

        ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping;


        ///////////////////////////////////////////////////////////////////////
        // Storage pointers
	std::unique_ptr <FPGATrackSimFitConstantBank> m_FitConstantBank_1st = nullptr;
	std::unique_ptr <FPGATrackSimFitConstantBank> m_FitConstantBank_2nd = nullptr;
        std::vector<std::unique_ptr <FPGATrackSimFitConstantBank> > m_FitConstantBankNoGuess_1st;
        std::vector<std::unique_ptr <FPGATrackSimFitConstantBank> > m_FitConstantBankNoGuess_2nd;
        std::unique_ptr<FPGATrackSimSectorBank> m_SectorBank_1st = nullptr;
        std::unique_ptr<FPGATrackSimSectorBank> m_SectorBank_2nd = nullptr;
        std::unique_ptr<FPGATrackSimSectorSlice> m_SectorSlices = nullptr;

        ///////////////////////////////////////////////////////////////////////
        // Methods for lazy loading.
        // missedPlane = -1 means banks where we guess any missing hits
        bool LoadFitConstantBank_1st(int missedPlane = -1);
        bool LoadFitConstantBank_2nd(int missedPlane = -1);
        bool LoadSectorBank_1st();
        bool LoadSectorBank_2nd();
        bool LoadSectorSlice();

};

#endif   // TRIGFPGATrackSimBANKSVC_H
