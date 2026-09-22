/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/HGTD_AnalysisAlgBase.h
 *
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 *
 * @brief  Base class for HGTD performance-study algorithms, providing the
 *  common histogram booking/filling machinery on top of AthAlgorithm.
 *
 *  To use it, create a new algorithm inheriting from HGTD_AnalysisAlgBase
 *  instead of the usual AthAlgorithm.
 */

#ifndef HGTD_ANALYSIS_ANALYSISALGBASE_H
#define HGTD_ANALYSIS_ANALYSISALGBASE_H

#include "AthenaBaseComps/AthAlgorithm.h"

// Athena includes
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"

// ROOT includes
#include "TEfficiency.h"
#include "TH1.h"

// stl includes
#include <map>
#include <string>

class HGTD_AnalysisAlgBase : public AthAlgorithm {

public:
  HGTD_AnalysisAlgBase(const std::string& name, ISvcLocator* svc_locator);
  virtual ~HGTD_AnalysisAlgBase();

  virtual StatusCode initialize();

  Gaudi::Property<std::string> m_directory_name{
      this, "DirectoryName", "/HGTD_ANA/", "The output directory name"};

  /// Templated function for booking histograms
  ///   The type of the histogram is passed as a template parameter
  ///   The function call takes a path (with terminating "/")
  ///     as well as the name and title of the histogram
  ///   Any additional parameters are passed as additonal parameters to the
  ///   constructor
  ///     i.e. the binning
  ///
  template <typename T = TH1F, typename... Ts>
  void bookSubdir(const std::string& trk_sel_name, const std::string& time_wp,
                  const std::string& hist_name, const std::string& title,
                  Ts... args) {
    // check if hist exists already and warn for duplication
    // doesn't do anything then...
    std::string name = trk_sel_name + "/" + time_wp + "/" + hist_name;
    if (m_histos.find(name) != m_histos.end()) {
      ATH_MSG_WARNING("You are duplicating histogram: "
                      << name << ", this is not a good idea\n");
      return;
    }
    m_histos[name] = new T(hist_name.c_str(), title.c_str(), args...);
    auto ptrT = dynamic_cast<T*>(m_histos[name]);
    if (!ptrT){
      ATH_MSG_WARNING("Cast failed for " << name);
      return;
    }
    ptrT->Sumw2();
    if (not m_hist_svc->regHist(m_directory_name + name, ptrT).isSuccess()) {
      ATH_MSG_WARNING("Failed to book " << name);
    }
  }

  template <typename T, typename... Ts>
  void book(const std::string& name, const std::string& title, Ts... args) {
    // check if hist exists already and warn for duplication
    // doesn't do anything then...
    if (m_histos.find(name) != m_histos.end()) {
      ATH_MSG_WARNING("You are duplicating histogram: "
                      << name << ", this is not a good idea\n");
      return;
    }
    m_histos[name] = new T(name.c_str(), title.c_str(), args...);
    dynamic_cast<T*>(m_histos[name])->Sumw2();
    if (not m_hist_svc
                ->regHist(m_directory_name + name,
                          dynamic_cast<T*>(m_histos[name]))
                .isSuccess()) {
      ATH_MSG_WARNING("Failed to book " << name);
    }
  }

  template <typename... Ts>
  void bookEffSubdir(const std::string& trk_sel_name,
                     const std::string& time_wp, const std::string& hist_name,
                     const std::string& title, Ts... args) {
    // check if hist exists already and warn for duplication
    // doesn't do anything then...
    std::string name = trk_sel_name + "/" + time_wp + "/" + hist_name;
    if (m_histos.find(name) != m_histos.end()) {
      ATH_MSG_WARNING("You are duplicating histogram: "
                      << name << ", this is not a good idea\n");
      return;
    }
    m_histos[name] = new TEfficiency(hist_name.c_str(), title.c_str(), args...);
    if (not m_hist_svc
                ->regGraph(m_directory_name + name,
                           reinterpret_cast<TGraph*>(m_histos[name]))
                .isSuccess()) {
      ATH_MSG_WARNING("Failed to book " << name);
    }
  }

  template <typename... Ts>
  void bookEff(const std::string& name, const std::string& title, Ts... args) {
    // check if hist exists already and warn for duplication
    // doesn't do anything then...
    if (m_histos.find(name) != m_histos.end()) {
      ATH_MSG_WARNING("You are duplicating histogram: "
                      << name << ", this is not a good idea\n");
      return;
    }
    m_histos[name] = new TEfficiency(name.c_str(), title.c_str(), args...);
    if (not m_hist_svc
                ->regGraph(m_directory_name + name,
                           reinterpret_cast<TGraph*>(m_histos[name]))
                .isSuccess()) {
      ATH_MSG_WARNING("Failed to book " << name);
    }
  }

  /// Templated function for filling histograms
  ///   The type of the histogram is passed as a template parameter
  ///   The function takes the name (including the path) as well as
  ///     any arguments to be passed to the histogram Fill method
  template <typename T, typename... Ts>
  void fill(const std::string& name, Ts... args) {
    if (m_histos[name] == nullptr or m_histos.find(name) == m_histos.end()) {
      ATH_MSG_WARNING(
          "[HistogramHandler::fill] ERROR: you are attempting to fill "
          "a histogram with name "
          << name << " which doesn't exist!\n");
      return;
    }
    dynamic_cast<T*>(m_histos[name])->Fill(args...);
  }

  template <typename T, typename... Ts>
  void fillSubdir(const std::string& trk_sel_name, const std::string& time_wp,
                  const std::string& hist_name, Ts... args) {
    std::string name = trk_sel_name + "/" + time_wp + "/" + hist_name;
    if (m_histos[name] == nullptr or m_histos.find(name) == m_histos.end()) {
      ATH_MSG_WARNING(
          "[HistogramHandler::fill] ERROR: you are attempting to fill "
          "a histogram with name "
          << name << " which doesn't exist!\n");
      return;
    }
    dynamic_cast<T*>(m_histos[name])->Fill(args...);
  }

  template <typename... Ts> void fillEff(const std::string& name, Ts... args) {
    if (m_histos[name] == nullptr or m_histos.find(name) == m_histos.end()) {
      ATH_MSG_WARNING(
          "[HistogramHandler::fill] ERROR: you are attempting to fill "
          "a histogram with name "
          << name << " which doesn't exist!\n");
      return;
    }
    dynamic_cast<TEfficiency*>(m_histos[name])->Fill(args...);
  }

  template <typename... Ts>
  void fillEffSubDir(const std::string& trk_sel_name,
                     const std::string& wp_name, const std::string& hist_name,
                     Ts... args) {
    std::string name = trk_sel_name + "/" + wp_name + "/" + hist_name;
    if (m_histos[name] == nullptr or m_histos.find(name) == m_histos.end()) {
      ATH_MSG_WARNING(
          "[HistogramHandler::fill] ERROR: you are attempting to fill "
          "a histogram with name "
          << name << " which doesn't exist!\n");
      return;
    }
    dynamic_cast<TEfficiency*>(m_histos[name])->Fill(args...);
  }

protected:
  ServiceHandle<ITHistSvc> m_hist_svc{this, "THistSvc", "THistSvc"};

private:
  std::map<std::string, TObject*> m_histos;
};

#endif // HGTD_ANALYSIS_ANALYSISALGBASE_H
