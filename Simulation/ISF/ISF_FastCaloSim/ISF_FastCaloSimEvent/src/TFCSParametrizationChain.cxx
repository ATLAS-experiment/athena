/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ISF_FastCaloSimEvent/TFCSParametrizationChain.h"
#include "ISF_FastCaloSimEvent/TFCSParametrizationPlaceholder.h"
#include "ISF_FastCaloSimEvent/TFCSInvisibleParametrization.h"
#include "ISF_FastCaloSimEvent/TFCSSimulationState.h"
#include "ISF_FastCaloSimEvent/TFCSTruthState.h"
#include "ISF_FastCaloSimEvent/TFCSExtrapolationState.h"
#include <algorithm>
#include <iterator>
#include "TBuffer.h"
#include "TDirectory.h"

//=============================================
//======= TFCSParametrizationChain =========
//=============================================

void TFCSParametrizationChain::recalc_pdgid_intersect() {
  set_pdgid(m_chain[0]->pdgid());

  for (const auto &param : m_chain) {
    std::set<int> tmp;

    std::set_intersection(pdgid().begin(), pdgid().end(),
                          param->pdgid().begin(), param->pdgid().end(),
                          std::inserter(tmp, tmp.begin()));
    set_pdgid(tmp);
  }
}

void TFCSParametrizationChain::recalc_pdgid_union() {
  set_pdgid(chain()[0]->pdgid());

  for (const auto &param : chain()) {
    std::set<int> tmp;

    std::set_union(pdgid().begin(), pdgid().end(), param->pdgid().begin(),
                   param->pdgid().end(), std::inserter(tmp, tmp.begin()));
    set_pdgid(tmp);
  }
}

void TFCSParametrizationChain::recalc_Ekin_intersect() {
  set_Ekin(*m_chain[0]);

  for (const auto &param : m_chain) {
    if (param->Ekin_min() > Ekin_min())
      set_Ekin_min(param->Ekin_min());
    if (param->Ekin_max() < Ekin_max())
      set_Ekin_max(param->Ekin_max());
    if (Ekin_nominal() < Ekin_min() || Ekin_nominal() > Ekin_max())
      set_Ekin_nominal(param->Ekin_nominal());
  }

  if (Ekin_nominal() < Ekin_min() || Ekin_nominal() > Ekin_max())
    set_Ekin_nominal(0.5 * (Ekin_min() + Ekin_max()));
}

void TFCSParametrizationChain::recalc_eta_intersect() {
  set_eta(*m_chain[0]);

  for (const auto &param : m_chain) {
    if (param->eta_min() > eta_min())
      set_eta_min(param->eta_min());
    if (param->eta_max() < eta_max())
      set_eta_max(param->eta_max());
    if (eta_nominal() < eta_min() || eta_nominal() > eta_max())
      set_eta_nominal(param->eta_nominal());
  }

  if (eta_nominal() < eta_min() || eta_nominal() > eta_max())
    set_eta_nominal(0.5 * (eta_min() + eta_max()));
}

void TFCSParametrizationChain::recalc_Ekin_eta_intersect() {
  recalc_Ekin_intersect();
  recalc_eta_intersect();
}

void TFCSParametrizationChain::recalc_Ekin_union() {
  set_Ekin(*m_chain[0]);

  for (const auto &param : m_chain) {
    if (param->Ekin_min() < Ekin_min())
      set_Ekin_min(param->Ekin_min());
    if (param->Ekin_max() > Ekin_max())
      set_Ekin_max(param->Ekin_max());
    if (Ekin_nominal() < Ekin_min() || Ekin_nominal() > Ekin_max())
      set_Ekin_nominal(param->Ekin_nominal());
  }

  if (Ekin_nominal() < Ekin_min() || Ekin_nominal() > Ekin_max())
    set_Ekin_nominal(0.5 * (Ekin_min() + Ekin_max()));
}

void TFCSParametrizationChain::recalc_eta_union() {
  set_eta(*m_chain[0]);

  for (const auto &param : m_chain) {
    if (param->eta_min() < eta_min())
      set_eta_min(param->eta_min());
    if (param->eta_max() > eta_max())
      set_eta_max(param->eta_max());
    if (eta_nominal() < eta_min() || eta_nominal() > eta_max())
      set_eta_nominal(param->eta_nominal());
  }

  if (eta_nominal() < eta_min() || eta_nominal() > eta_max())
    set_eta_nominal(0.5 * (eta_min() + eta_max()));
}

void TFCSParametrizationChain::recalc_Ekin_eta_union() {
  recalc_Ekin_union();
  recalc_eta_union();
}

void TFCSParametrizationChain::recalc() {
  clear();
  if (m_chain.empty())
    return;

  recalc_pdgid_intersect();
  recalc_Ekin_eta_intersect();

  m_chain.shrink_to_fit();
}

bool TFCSParametrizationChain::is_match_Ekin_bin(int Ekin_bin) const {
  for (const auto &param : m_chain)
    if (!param->is_match_Ekin_bin(Ekin_bin))
      return false;
  return true;
}

bool TFCSParametrizationChain::is_match_calosample(int calosample) const {
  for (const auto &param : m_chain)
    if (!param->is_match_calosample(calosample))
      return false;
  return true;
}

FCSReturnCode TFCSParametrizationChain::simulate(
    TFCSSimulationState &simulstate, const TFCSTruthState *truth,
    const TFCSExtrapolationState *extrapol) const {
  Int_t retry = 0;
  Int_t retry_warning = 1;

  FCSReturnCode status = FCSSuccess;
  for (int i = 0; i <= retry; i++) {
    if (i >= retry_warning)
      ATH_MSG_WARNING(
          "TFCSParametrizationChain::simulate(): Retry simulate call "
          << i << "/" << retry);
    for (const auto &param : m_chain) {
      status = simulate_and_retry(param, simulstate, truth, extrapol);

      if (status >= FCSRetry) {
        retry = status - FCSRetry;
        retry_warning = retry >> 1;
        if (retry_warning < 1)
          retry_warning = 1;
        break;
      }
      if (status == FCSFatal)
        return FCSFatal;
    }

    if (status == FCSSuccess)
      break;
  }

  if (status != FCSSuccess) {
    ATH_MSG_FATAL(
        "TFCSParametrizationChain::simulate(): Simulate call failed after "
        << retry << " retries");
    return FCSFatal;
  }

  return FCSSuccess;
}

void TFCSParametrizationChain::Print(Option_t *option) const {
  TFCSParametrization::Print(option);
  TString opt(option);
  // bool shortprint=opt.Index("short")>=0;
  // bool longprint=msgLvl(MSG::DEBUG) || (msgLvl(MSG::INFO) && !shortprint);

  char count = 'A';
  for (const auto &param : m_chain) {
    param->Print(opt + count + ' ');
    count++;
  }
}

void TFCSParametrizationChain::Streamer(TBuffer &R_b) {
  // Stream an object of class TFCSParametrizationChain.

  UInt_t R_s, R_c;
  TDirectory *dir = nullptr;

  if (R_b.IsReading()) {
    Version_t R_v = R_b.ReadVersion(&R_s, &R_c);
    if (R_v == 1) {
      R_b.SetBufferOffset(R_s);
      R_b.ReadClassBuffer(TFCSParametrizationChain::Class(), this);
    } else {
      TFCSParametrization::Streamer(R_b);

      TObject *parent = R_b.GetParent();
      if (R_b.GetParent()) {
        if (parent->InheritsFrom(TDirectory::Class())) {
          dir = static_cast<TDirectory *>(parent);
        }
      }

      TFCSParametrizationChain::Chain_t &R_stl = m_chain;
      R_stl.clear();
      TClass *R_tcl1 = TFCSParametrizationBase::Class();
      if (R_tcl1 == nullptr) {
        Error("m_chain streamer",
              "Missing the TClass object for class TFCSParametrizationBase *!");
        return;
      }
      int R_i, R_n;
      R_b >> R_n;
      R_stl.reserve(R_n);
      for (R_i = 0; R_i < R_n; R_i++) {
        std::unique_ptr<TFCSParametrizationBase> R_t;
        R_t.reset((TFCSParametrizationBase *)R_b.ReadObjectAny(R_tcl1));
        if (R_t != nullptr) {
          if (R_t->InheritsFrom(TFCSParametrizationPlaceholder::Class())) {
            std::unique_ptr<TFCSParametrizationBase> new_R_t = nullptr;

            if (dir) {
              new_R_t.reset(
                  (TFCSParametrizationBase *)dir->Get(R_t->GetName()));
            }

            if (new_R_t) {
              R_t = std::move(new_R_t);
            } else {
              Error("TFCSParametrizationChain::Streamer",
                    "Found placeholder object in the parametrization chain, "
                    "but could not read the real object from the file!");
            }
          }
        }
        if(R_t){
          R_t->checkHists();
          R_stl.emplace_back(R_t.release());
        }
      }

      R_b.CheckByteCount(R_s, R_c, TFCSParametrizationChain::IsA());
    }
  } else {
    R_c = R_b.WriteVersion(TFCSParametrizationChain::IsA(), kTRUE);
    TFCSParametrization::Streamer(R_b);

    if (SplitChainObjects()) {
      TObject *parent = R_b.GetParent();
      if (R_b.GetParent()) {
        if (parent->InheritsFrom(TDirectory::Class())) {
          dir = static_cast<TDirectory *>(parent);
        }
      }
    }

    TFCSParametrizationChain::Chain_t &R_stl = m_chain;
    int R_n = int(R_stl.size());
    R_b << R_n;
    if (R_n) {
      TFCSParametrizationChain::Chain_t::iterator R_k;
      for (R_k = R_stl.begin(); R_k != R_stl.end(); ++R_k) {
        TFCSParametrizationBase *R_t = *R_k;  // Ownership stays with m_chain
        if (dir && R_t != nullptr) {
          dir->WriteTObject(R_t);
          TFCSParametrizationPlaceholder tmp( R_t->GetName(), TString("Placeholder for: ") + R_t->GetTitle());
          R_b.WriteObject( &tmp, false ); // tell R_b object with same address are actually different
        } else {
          R_b.WriteObject( R_t );
        }
      }
    }
    R_b.SetByteCount(R_c, kTRUE);
  }
}

void TFCSParametrizationChain::unit_test(
    TFCSSimulationState *simulstate, const TFCSTruthState *truth,
    const TFCSExtrapolationState *extrapol) {
  ISF_FCS::MLogging logger;
  if (!simulstate)
    simulstate = new TFCSSimulationState();
  if (!truth)
    truth = new TFCSTruthState();
  if (!extrapol)
    extrapol = new TFCSExtrapolationState();

  TFCSParametrizationChain chain("chain", "chain");
  chain.setLevel(MSG::DEBUG);

  ATH_MSG_NOCLASS(logger, "====         Chain setup       ====");
  chain.Print();
  ATH_MSG_NOCLASS(logger, "==== Simulate with empty chain ====");
  chain.simulate(*simulstate, truth, extrapol);
  ATH_MSG_NOCLASS(logger, "===================================" << std::endl);

  TFCSParametrizationBase *param;
  param = new TFCSInvisibleParametrization("A begin all", "A begin all");
  param->setLevel(MSG::VERBOSE);
  chain.push_back(param);
  param = new TFCSParametrization("A end all", "A end all");
  param->setLevel(MSG::DEBUG);
  chain.push_back(param);

  ATH_MSG_NOCLASS(logger, "====         Chain setup       ====");
  chain.Print();
  ATH_MSG_NOCLASS(logger, "==== Simulate only begin/end all ====");
  chain.simulate(*simulstate, truth, extrapol);
  ATH_MSG_NOCLASS(logger,
                  "==== Simulate only begin/end all with chain retry====");
  chain.set_RetryChainFromStart();
  chain.simulate(*simulstate, truth, extrapol);
  chain.reset_RetryChainFromStart();
  ATH_MSG_NOCLASS(logger, "===================================" << std::endl);
}
