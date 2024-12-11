// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


#include "FPGATrackSimBankSvc.h"


///////////////////////////////////////////////////////////////////////////////
// AthService Functions
///////////////////////////////////////////////////////////////////////////////


FPGATrackSimBankSvc::FPGATrackSimBankSvc(const std::string& name, ISvcLocator*svc) :
    base_class(name,svc),
    m_FPGATrackSimMapping("FPGATrackSimMappingSvc", name)
{
}



StatusCode FPGATrackSimBankSvc::initialize()
{
    ATH_MSG_DEBUG("-------------- FPGATrackSimBankSvc -------------------");
    ATH_MSG_DEBUG("BankType Map    = " << m_bankTypes);
    ATH_MSG_DEBUG("Number of 1st stage coordinates = " << m_ncoords_1st);
    ATH_MSG_DEBUG("Number of 2nd stage coordinates = " << m_ncoords_2nd);
    ATH_MSG_DEBUG("-------------------------------------------------");

    // resize this to the appropriate one and to be safe, set to nullptr to start
    m_FitConstantBankNoGuess_1st.resize(m_FPGATrackSimMapping->PlaneMap_1st(0)->getNLogiLayers());
    m_FitConstantBankNoGuess_2nd.resize(m_FPGATrackSimMapping->PlaneMap_2nd(0)->getNLogiLayers());
    for (unsigned i = 0; i < m_FitConstantBankNoGuess_1st.size(); i++) m_FitConstantBankNoGuess_1st[i] = nullptr;
    for (unsigned i = 0; i < m_FitConstantBankNoGuess_2nd.size(); i++) m_FitConstantBankNoGuess_2nd[i] = nullptr;

    return StatusCode::SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
// Load Functions
///////////////////////////////////////////////////////////////////////////////

bool FPGATrackSimBankSvc::LoadFitConstantBank_1st(int missedPlane)
{
    if (missedPlane == -1)
      m_FitConstantBank_1st = std::unique_ptr<FPGATrackSimFitConstantBank>(new FPGATrackSimFitConstantBank(m_FPGATrackSimMapping->PlaneMap_1st(0), m_ncoords_1st, m_path_constants_1st, true));
    else
      m_FitConstantBankNoGuess_1st[missedPlane] = std::unique_ptr<FPGATrackSimFitConstantBank> (new FPGATrackSimFitConstantBank(m_FPGATrackSimMapping->PlaneMap_1st(0), m_ncoords_1st, m_path_constants_1st_noguess[missedPlane], true, missedPlane)); // no check on the plane number

    return true;
}


bool FPGATrackSimBankSvc::LoadFitConstantBank_2nd(int missedPlane)
{
    if (missedPlane == -1)
      m_FitConstantBank_2nd = std::make_unique<FPGATrackSimFitConstantBank> (m_FPGATrackSimMapping->PlaneMap_2nd(0), m_ncoords_2nd, m_path_constants_2nd, false);
    else
      m_FitConstantBankNoGuess_2nd[missedPlane] = std::make_unique<FPGATrackSimFitConstantBank> (m_FPGATrackSimMapping->PlaneMap_2nd(0), m_ncoords_2nd, m_path_constants_2nd_noguess[missedPlane], false, missedPlane); // no check on the plane number

    return true;
}


bool FPGATrackSimBankSvc::LoadSectorBank_1st()
{
  m_SectorBank_1st = std::unique_ptr<FPGATrackSimSectorBank> (new FPGATrackSimSectorBank(m_path_sectorbank_1st));
  m_SectorBank_1st->storeQOverPtBinning(m_qOverPtBins, m_absQOverPtBinning);
  return true;
}


bool FPGATrackSimBankSvc::LoadSectorBank_2nd()
{
  m_SectorBank_2nd = std::unique_ptr<FPGATrackSimSectorBank> (new FPGATrackSimSectorBank(m_path_sectorbank_2nd));
  m_SectorBank_2nd->storeQOverPtBinning(m_qOverPtBins, m_absQOverPtBinning);
  return true;
}


bool FPGATrackSimBankSvc::LoadSectorSlice()
{
  m_SectorSlices = std::unique_ptr<FPGATrackSimSectorSlice> (new FPGATrackSimSectorSlice(m_path_sectorslices));
  return true;
}





///////////////////////////////////////////////////////////////////////////////
// IFPGATrackSimBankSvc Functions
///////////////////////////////////////////////////////////////////////////////


const FPGATrackSimFitConstantBank* FPGATrackSimBankSvc::FitConstantBank_1st(int missedPlane)
{
    if (missedPlane == -1) {
        // Don't need to check the return actually, will return nullptr anyways
        if (!m_FitConstantBank_1st) LoadFitConstantBank_1st();

        return m_FitConstantBank_1st.get();
    }
    else {
        // Don't need to check the return actually, will return nullptr anyways
        if (!m_FitConstantBankNoGuess_1st[missedPlane]) LoadFitConstantBank_1st(missedPlane); 

        return m_FitConstantBankNoGuess_1st[missedPlane].get();
    }
}


const FPGATrackSimFitConstantBank* FPGATrackSimBankSvc::FitConstantBank_2nd(int missedPlane)
{
    if (missedPlane == -1) {
        // Don't need to check the return actually, will return nullptr anyways
        if (!m_FitConstantBank_2nd) LoadFitConstantBank_2nd();

        return m_FitConstantBank_2nd.get();
    }
    else {
        // Don't need to check the return actually, will return nullptr anyways
        if (!m_FitConstantBankNoGuess_2nd[missedPlane]) LoadFitConstantBank_2nd(missedPlane);

        return m_FitConstantBankNoGuess_2nd[missedPlane].get();
    }
}


const FPGATrackSimSectorBank* FPGATrackSimBankSvc::SectorBank_1st()
{
    // Don't need to check the return actually, will return nullptr anyways
    if (!m_SectorBank_1st) LoadSectorBank_1st();
    return m_SectorBank_1st.get();
}

const FPGATrackSimSectorBank* FPGATrackSimBankSvc::SectorBank_2nd()
{
    // Don't need to check the return actually, will return nullptr anyways
    if (!m_SectorBank_2nd) LoadSectorBank_2nd();

    return m_SectorBank_2nd.get();
}


const FPGATrackSimSectorSlice* FPGATrackSimBankSvc::SectorSlice()
{
    // Don't need to check the return actually, will return nullptr anyways
    if (!m_SectorSlices) LoadSectorSlice();

    return m_SectorSlices.get();
}
