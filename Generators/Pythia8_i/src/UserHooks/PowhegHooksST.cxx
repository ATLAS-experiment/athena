/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// PowhegHooksST.h 
// Copyright (C) 2025 Silvia Ferrario Ravasio and Tomas Jezo 
// Authors: Silvia Ferrario Ravasio and Tomas Jezo based on 
// PowhegHooks.h by Richard Corke
// PowhegHooksBB4l.h by Tomas Jezo, Markus Seidel, Ben Nachman 

// Adapted to be Athena and Pythia8_i compliant by Lukas Kretschmann (lukas.kretschmann@cern.ch) 

#include "Pythia8/Pythia.h"
#include "UserHooksUtils.h"
#include "Pythia8_i/UserHooksFactory.h"

#if PYTHIA_VERSION_INTEGER >= 8310
// Disable the pythia 8.310 plugin mechanism.
// We use our own mechanism to create these classes, and we get link errors
// if these are used in more than one compilation unit.
# include "Pythia8/Plugins.h"
# undef PYTHIA8_PLUGIN_CLASS
# undef PYTHIA8_PLUGIN_VERSIONS
# define PYTHIA8_PLUGIN_CLASS(BASE, CLASS, PYTHIA, SETTINGS, LOGGER)
# define PYTHIA8_PLUGIN_VERSIONS(...)
#endif

#include "Pythia8Plugins/PowhegHooks.h"
#include "UserSetting.h"

#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <map>
#include <sstream>
#include <cstdio>

namespace Pythia8 { class PowhegHooksST; }
Pythia8_UserHooks::UserHooksFactory::Creator<Pythia8::PowhegHooksST> powhegHooksSTCreator("PowhegHooksST");
Pythia8_UserHooks::UserHooksFactory::Creator<Pythia8::PowhegHooksST> powhegSTCreator("PowhegST");

namespace Pythia8 {

/**
 * @class PowhegHooksST
 * @brief POWHEG resonance-veto hooks for single-top t- and s-channel events.
 *
 * Production radiation is delegated to PowhegHooks. This class handles QCD
 * radiation from decayed resonances and vetoes emissions harder than the
 * corresponding POWHEG resonance scale.
 *
 * Resonance vetoes are enabled only for eventtype == 1 (the btilde
 * contribution). Remnant and regular events (eventtype == 2 and 3) are not
 * subject to this resonance veto, even if they contain a top.
 *
 * emscales == 0 reconstructs scales from Pythia decay kinematics. emscales
 * == 1 reads POWHEG's recorded scales using the original LHA resonance index.
 * In mode 1, a resonance with no coloured daughters is unrestricted (1e30),
 * while an unrecorded coloured decay uses pTcutoff.
 *
 * Resonances are identified by signed-ID ancestry rather than Pythia's system
 * number, which supports nested s-channel histories and event reordering.
 *
 * In Athena, UserHooks are loaded dynamically via UserHooksFactory without
 * direct access to LHAupPtr, so eventtype and #emscales information are
 * retrieved directly from the LHE event comments via infoPtr->getEventComments().
 */
class PowhegHooksST : public PowhegHooks {

public:

  /**
   * @brief Constructor and destructor for Athena UserHooksFactory.
   *
   * Initializes Pythia8_UserHooks::UserSetting handles for configuration
   * via Athena JobOptions (e.g. POWHEG:single-top:emscales, POWHEG:pTcutoff).
   */
  PowhegHooksST()
    : m_pTcutoffSetting("POWHEG:pTcutoff", 0.8),
      m_emscalesSetting("POWHEG:single-top:emscales", 0),
      m_emscalesAltSetting("POWHEG:emscales", 0),
      m_debugSetting("POWHEG:single-top:DEBUG", false),
      m_eventtype(1),
      m_pTcutoff(0.8),
      m_emscalesMode(0),
      m_vetoMode(0),
      m_QEDvetoMode(0),
      m_vetoCount(3)
  {
    std::cout << "**********************************************************" << std::endl;
    std::cout << "*                                                        *" << std::endl;
    std::cout << "*       Applying PowhegHooksST UserHook (Single-Top)!    *" << std::endl;
    std::cout << "*   Resonance-aware matching for single-top-dec-pwhg     *" << std::endl;
    std::cout << "*                                                        *" << std::endl;
    std::cout << "**********************************************************" << std::endl;
  }

  ~PowhegHooksST() override {}

  /**
   * @brief Initialize settings for the POWHEG resonance-veto strategy.
   *
   * Reads the Pythia veto settings and prepares the selected scale source.
   * In emscales mode 1, scales will be parsed from the LHE event comments.
   *
   * @return true if initialization is successful, false otherwise.
   */
  bool initAfterBeams() override {
    // Initialize base PowhegHooks settings
    PowhegHooks::initAfterBeams();

    if (settingsPtr->isParm("POWHEG:pTcutoff")) {
      m_pTcutoff = settingsPtr->parm("POWHEG:pTcutoff");
    } else {
      m_pTcutoff = m_pTcutoffSetting(settingsPtr);
    }

    m_vetoMode    = settingsPtr->mode("POWHEG:veto");
    m_vetoCount   = settingsPtr->mode("POWHEG:vetoCount");
    m_QEDvetoMode = settingsPtr->mode("POWHEG:QEDveto");

    if (settingsPtr->isMode("POWHEG:single-top:emscales")) {
      m_emscalesMode = settingsPtr->mode("POWHEG:single-top:emscales");
    } else if (settingsPtr->isMode("POWHEG:emscales")) {
      m_emscalesMode = settingsPtr->mode("POWHEG:emscales");
    } else {
      m_emscalesMode = m_emscalesSetting(settingsPtr);
    }

    if (m_emscalesMode == 1) {
      std::cout << "PowhegHooksST: resonance veto scales taken from the POWHEG emission scales [emscales 1]\n";
      m_readScales.clear();
    } else {
      std::cout << "PowhegHooksST: resonance veto scales reconstructed from the event record [emscales 0]\n";
      m_vetoResScale.clear();
    }
    return true;
  }

  /**
   * @brief Compute the POWHEG transverse scale of a resonance splitting.
   *
   * @param event Event record containing the branching and resonance.
   * @param iEmt Index of the post-branching emitter.
   * @param iRad Index of the radiated parton.
   * @param iRes Index of the parent resonance.
   * @return The POWHEG transverse scale of the branching.
   */
  double pTpowhegFromRes(const Event &event, int iEmt, int iRad, int iRes) {
    // Rad should be the radiated gluon for q->qg.
    // For g->gg or g->qq it does not matter.
    if (event[iEmt].id() == 21 || event[iRad].id() != 21)
      std::swap(iEmt, iRad);

    // Resonance, post branching emitter, and radiated parton
    Vec4 pres(event[iRes].p());
    Vec4 pemt(event[iEmt].p());
    Vec4 prad(event[iRad].p());

    // Energies in the resonance rest frame
    double mRes = std::sqrt(pres * pres);
    double Erad = prad * pres / mRes;
    double Eemt = pemt * pres / mRes;
    double scale;
    if ((event[iEmt].id() != 21 && std::abs(event[iEmt].id()) > 6)
     || (event[iRad].id() != 21 && std::abs(event[iRad].id()) > 6)) {
      if (loggerPtr) loggerPtr->ABORT_MSG("pTpowheg should be called only over QCD splittings");
      exit(1);
    }

    // Gluon (rad) emission from quark (emt) line: 2*(1 - cos theta)*E_g^2
    if (std::abs(event[iEmt].id()) != std::abs(event[iRad].id())) {
      scale = 2.0 * pemt * prad * Erad / Eemt;
    } else {
      // Gluon splitting: use a symmetrised scale
      scale = 2.0 * pemt * prad * Eemt * Erad / (std::pow(Eemt, 2) + std::pow(Erad, 2));
    }
    return std::sqrt(scale);
  }

  /**
   * @brief Disable Pythia's ordinary resonance-scale assignment.
   *
   * Scales are assigned in doVetoProcessLevel(), before resonance showers are
   * interleaved, so the result is independent of Pythia's system order.
   *
   * @return Always false.
   */
  bool canSetResonanceScale() override { return false; }

  /**
   * @brief Reconstruct a resonance scale from the Pythia event record.
   *
   * Used for emscales == 0. Top scales are computed from the b/gluon decay
   * system; leptonic W/Z decays are unrestricted; coloured W/Z decays without
   * a resolved gluon use pTcutoff. Other resonance types are unrestricted.
   *
   * @param iRes Index of the resonance particle.
   * @param event Event record containing the resonance decay.
   * @return The resonance veto scale, or 1e30 for an unrestricted resonance.
   */
  double scaleResonance(int iRes, const Event &event) override {
    double veto_res_scale;
    if (event[iRes].daughterList().size() > 3) {
      if (loggerPtr) loggerPtr->ABORT_MSG("A resonance can have at most 3 children");
      exit(1);
    }

    // For the time being, only from btilde events with tops
    if (m_eventtype != 1) {
      veto_res_scale = 1e30;
    } else {
      // Determine if W or Z boson is decaying leptonically or hadronically
      if (std::abs(event[iRes].id()) == 23 || std::abs(event[iRes].id()) == 24) {
        int iq = -1, iqb = -1, ig = -1;
        for (int i = 0; i < int(event[iRes].daughterList().size()); i++) {
          int iDau = event[iRes].daughterList()[i];
          if (event[iDau].id() == 21) ig = iDau;
          if (event[iDau].id() > 0 && event[iDau].id() < 6) iq = iDau;
          if (event[iDau].id() > -6 && event[iDau].id() < 0) iqb = iDau;
        }

        // Leptonic decay
        if (iq == -1 && iqb == -1) {
          veto_res_scale = 1e30;
        } else {
          // Hadronic decay without gluon emission
          if (ig == -1) {
            veto_res_scale = m_pTcutoff;
          } else {
            double pTqg  = (iq > 0) ? pTpowhegFromRes(event, iq, ig, iRes) : 1e30;
            double pTqbg = (iqb > 0) ? pTpowhegFromRes(event, iqb, ig, iRes) : 1e30;
            veto_res_scale = std::min(pTqg, pTqbg);
          }
        }
      }
      // Top decay
      else if (std::abs(event[iRes].id()) == 6) {
        // Now handle tops
        if (event[iRes].daughterList().size() < 3) {
          veto_res_scale = m_pTcutoff;
        } else {
          // Find top daughters
          int iw = -1, ib = -1, ig = -1;
          for (int i = 0; i < int(event[iRes].daughterList().size()); i++) {
            int iDau = event[iRes].daughterList()[i];
            if (std::abs(event[iDau].id()) == 24) iw = iDau;
            if (std::abs(event[iDau].id()) == 5)  ib = iDau;
            if (std::abs(event[iDau].id()) == 21) ig = iDau;
          }
          if (iw < 0 || ib < 0 || ig < 0) {
            if (loggerPtr) loggerPtr->ABORT_MSG("Top decay products not found");
            exit(1);
          }
          veto_res_scale = pTpowhegFromRes(event, ib, ig, iRes);
        }
      } else {
        // Other resonance: no veto
        veto_res_scale = 1e30;
      }
    }
    return veto_res_scale;
  }

  /**
   * @brief Advertise support for process-level processing.
   *
   * @return Always true.
   */
  bool canVetoProcessLevel() override { return true; }

  /**
   * @brief Build resonance veto scales before showering starts.
   *
   * No resonance scales are built for eventtype != 1. Otherwise scales are
   * reconstructed from the event record or read using original LHA ires values.
   *
   * @param event Pythia event record for the current LHE event.
   * @return Always false; this prepares scales but does not reject the event.
   */
  bool doVetoProcessLevel(Event &event) override {
    m_vetoResScale.clear();
    m_eventtype = getEventType();

    // Regular or remnant events have no POWHEG radiation in their decays. In particular,
    // an empty emission-scale list must not turn into a cutoff veto for them.
    if (m_eventtype != 1) return false;

    if (m_emscalesMode == 1) {
      fillReadScales();
      // ires refers to the ORIGINAL LHA record, before Pythia reorders it.
      // In Pythia's event record, LHE particles are offset by 2 beam entries (indices 1 and 2),
      // so the original LHE index is i - 2.
      // Identify a resonance by its signed IDs from the resonance to the root.
      // This distinguishes e.g. W* -> t bbar -> W b bbar and W* -> W Z.
      for (int i = 0; i < event.size(); ++i) {
        if (event[i].statusAbs() == 22 && !event[i].daughterList().empty()) {
          bool hasQCDDecay = false;
          for (int d : event[i].daughterList()) {
            if (particleDataPtr->colType(event[d].id()) != 0) hasQCDDecay = true;
          }
          double scale = readScale(i - 2, hasQCDDecay ? m_pTcutoff : 1e30);
          storeResonanceScale(resonanceKey(i, event), scale);
        }
      }
    } else {
      for (int i = 0; i < event.size(); ++i) {
        if (event[i].status() == -22 && !event[i].daughterList().empty()) {
          storeResonanceScale(resonanceKey(i, event), scaleResonance(i, event));
        }
      }
    }
    return false;
  }

  /**
   * @brief Veto too-hard QCD radiation from a decayed resonance.
   *
   * Production radiation and radiation outside a resonance are delegated to
   * PowhegHooks. For eventtype == 1, resonance radiation is compared with the
   * scale prepared by doVetoProcessLevel(). A scale of 1e30 is unrestricted.
   * The colour test uses colType(), so top radiation such as t -> t g is kept.
   *
   * @param sizeOld Event-record size before the emission.
   * @param e Event record after the emission.
   * @param iSys Pythia parton-system index.
   * @param inResonance Whether the emission is inside a resonance.
   * @return true if the emission is vetoed, false otherwise.
   */
  bool doVetoFSREmission(int sizeOld, const Event &e, int iSys, bool inResonance) override {
    // Radiation from the hard system: delegate to PowhegHooks
    if (iSys == 0 || !inResonance) return PowhegHooks::doVetoFSREmission(sizeOld, e, iSys, inResonance);

    if (m_eventtype != 1) return false;

    // Get the participants of the splitting: the radiation and the emitter
    const int iRad = e.size() - 2;
    const int iEmtAft = e.size() - 3;
    const int iEmtBef = e[iRad].mother1();

    // Test colour rather than isParton(), which excludes the top in t -> t g
    // radiation inside the s-channel W* decay.
    // Do not veto QED radiation (i.e. if (pre or post) emitter or emission are not coloured;
    // can be f -> f gamma, gamma -> f fb, with f quark or lepton)
    if (m_QEDvetoMode == 0 && !(
        e[iEmtAft].colType() != 0 && e[iEmtBef].colType() != 0
        && e[iRad].colType() != 0)) {
      return false;
    }

    // Get the resonance ID from the partonSystem pointer
    const int iRes = partonSystemsPtr->getInRes(iSys);
    // Systems are created as resonances decay, possibly interleaved with the
    // shower. Resolve the actual incoming resonance instead of guessing iSys.
    auto scale = m_vetoResScale.find(resonanceKey(iRes, e));
    if (scale == m_vetoResScale.end()) {
      if (loggerPtr) loggerPtr->ABORT_MSG("Cannot find the resonance the showering is coming from");
      exit(1);
    }
    const double pTmax = scale->second;
    if (pTmax == 1e30) return false;

    const double pTcurr = pTpowhegFromRes(e, iEmtAft, iRad, iRes);
    // Veto emissions harder than the powheg one
    return (pTcurr > pTmax);
  }

private:

  Pythia8_UserHooks::UserSetting<double> m_pTcutoffSetting;
  Pythia8_UserHooks::UserSetting<int>    m_emscalesSetting;
  Pythia8_UserHooks::UserSetting<int>    m_emscalesAltSetting;
  Pythia8_UserHooks::UserSetting<bool>   m_debugSetting;

  int    m_eventtype;
  double m_pTcutoff;
  int    m_emscalesMode;
  int    m_vetoMode;
  int    m_QEDvetoMode;
  int    m_vetoCount;

  // Scales keyed by the resonance ancestry, independent of event/system ordering.
  std::map<std::vector<int>, double> m_vetoResScale;

  //--------------------------------------------------------------------------
  // Everything below is only used when emscalesMode == 1
  //--------------------------------------------------------------------------

  struct EmissionScale {
    int    idx;
    double scalup;
    int    id_emitter;
    int    id_radiated;
    int    ires;
  };
  std::vector<EmissionScale> m_readScales;

  /**
   * @brief Construct the stable signed-ID ancestry key for a resonance.
   *
   * Only status-22 entries are included, so shower/recoil copies do not create
   * a second identity for the same physical resonance history.
   *
   * @param iRes Index of the resonance or its shower copy.
   * @param event Pythia event record.
   * @return Signed-ID path from the resonance towards the hard-process root.
   */
  std::vector<int> resonanceKey(int iRes, const Event &event) const {
    std::vector<int> key;
    // Original resonance entries retain |status| = 22. Shower/recoil copies
    // have other statuses; follow their mothers without counting them again.
    for (int i = iRes; i > 0; i = event[i].mother1()) {
      if (event[i].statusAbs() == 22) key.push_back(event[i].id());
    }
    return key;
  }

  /**
   * @brief Store a scale for a resonance ancestry and reject ambiguity.
   *
   * @param key Stable resonance ancestry key.
   * @param scale Resonance veto scale.
   */
  void storeResonanceScale(const std::vector<int> &key, double scale) {
    // The s- and t-channel histories have unique resonance paths. Do not
    // silently assign one scale to two indistinguishable sibling resonances.
    if (!m_vetoResScale.emplace(key, scale).second) {
      if (loggerPtr) loggerPtr->ABORT_MSG("Ambiguous resonance ancestry in POWHEG event");
      exit(1);
    }
  }

  /**
   * @brief Extract rad_type / eventtype from LHE event comments.
   *
   * In Athena, UserHooks run without the Fortran common block powhegeventssettings_.
   * The event type (1 = btilde, 2 = remnant, 3 = regular) is parsed from the
   * per-event comments (#rwgt <type> ... or # <type> rad_type).
   *
   * @return The event type (1, 2, or 3). Defaults to 1 if comments cannot be found.
   */
  int getEventType() const {
    int eventtype = 1;
    if (!infoPtr) return eventtype;
    std::string comments = infoPtr->getEventComments();
    if (comments.empty()) return eventtype;

    std::istringstream iss(comments);
    std::string line;
    while (std::getline(iss, line)) {
      if (line.compare(0, 5, "#rwgt") == 0) {
        std::istringstream lss(line.substr(5));
        int type = 0;
        if (lss >> type) return type;
      }
      if (line.find("rad_type") != std::string::npos) {
        int type = 0;
        if (std::sscanf(line.c_str(), "# %d rad_type", &type) == 1) return type;
      }
    }
    return eventtype;
  }

  /**
   * @brief Populate POWHEG emission scales for the current event.
   *
   * In Athena, event comments recorded in the LHE event record are parsed
   * for #emscales lines without needing the Fortran common block c_emscales_.
   */
  void fillReadScales() {
    m_readScales.clear();
    if (!infoPtr) return;
    std::string comments = infoPtr->getEventComments();
    if (comments.empty()) return;

    std::istringstream iss(comments);
    std::string line;
    while (std::getline(iss, line)) {
      if (line.compare(0, 9, "#emscales") != 0) continue;
      if (line.find("nemissions=") != std::string::npos) continue;

      EmissionScale es{};
      int idx = 0;
      if (std::sscanf(line.c_str(),
            "#emscales i=%d scalup=%lf id_emitter=%d id_radiated=%d ires=%d",
            &idx, &es.scalup, &es.id_emitter, &es.id_radiated, &es.ires) == 5) {
        es.idx = idx;
        m_readScales.push_back(es);
      }
    }
  }

  /**
   * @brief Look up a POWHEG scale by original LHA resonance index.
   *
   * @param ires Original LHA resonance index.
   * @param fallback Value returned when no scale was recorded for ires.
   * @return The recorded scale or fallback.
   */
  double readScale(int ires, double fallback) const {
    for (const auto &es : m_readScales) {
      if (es.ires == ires) return es.scalup;
    }
    return fallback;
  }

};

} // namespace Pythia8
