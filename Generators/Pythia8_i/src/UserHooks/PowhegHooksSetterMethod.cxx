// PowhegHooks.h is a part of the PYTHIA event generator.
// Copyright (C) 2025 Richard Corke, Torbjorn Sjostrand.
// PYTHIA is licenced under the GNU GPL v2 or later, see COPYING for details.
// Please respect the MCnet Guidelines, see GUIDELINES for details.

// Author: Richard Corke, modified by Christian T Preuss.
// modified by Katharina Voss (only change: setpThard method)
// modified version from PowhegHooks.h method in Pythia8.314
// /cvmfs/atlas-nightlies.cern.ch/repo/sw/main_AthGeneration_x86_64-el9-gcc13-opt/sw/lcg/releases/LCG_107a_ATLAS_9/MCGenerators/pythia8/314/x86_64-el9-gcc13-opt/include/Pythia8Plugins/PowhegHooks.h
// This class is used to perform a vetoed shower, where emissions
// already covered in a POWHEG NLO generator should be omitted.
// To first approximation the handover should happen at the SCALE
// of the LHA, but since the POWHEG-BOX uses a different pT definition
// than PYTHIA, both for ISR and FSR, a more sophisticated treatment
// is needed. See the online manual on POWHEG matching for details.

#ifndef Pythia8_PowhegHooksSetterMethod_H
#define Pythia8_PowhegHooksSetterMethod_H

// Includes
#include "Pythia8/Pythia.h"
#include "Pythia8/Plugins.h"

namespace Pythia8 {

//==========================================================================

// Use userhooks to veto PYTHIA emissions above the POWHEG scale.

class PowhegHooksSetterMethod : public UserHooks {

public:

  // Constructors and destructor.
  // default member variables from PowhegHooks settings initialized as specified in 
  // /cvmfs/atlas-nightlies.cern.ch/repo/sw/main_AthGeneration_x86_64-el9-gcc13-opt/sw/lcg/releases/LCG_107a_ATLAS_9/MCGenerators/pythia8/314/x86_64-el9-gcc13-opt/share/Pythia8/xmldoc/POWHEGMatching.xml
  // and showerModel as specified in 
  // /cvmfs/atlas-nightlies.cern.ch/repo/sw/main_AthGeneration_x86_64-el9-gcc13-opt/sw/lcg/releases/LCG_107a_ATLAS_9/MCGenerators/pythia8/314/x86_64-el9-gcc13-opt/share/Pythia8/xmldoc/PartonShowers.xml
  PowhegHooksSetterMethod(): 
        m_showerModel(1),
        m_nFinal(-1),
        m_vetoMode(0),
        m_MPIvetoMode(0),
        m_QEDvetoMode(0),
        m_vetoCount(3),
        m_pThardMode(0),
        m_pTemtMode(0),
        m_emittedMode(0),
        m_pTdefMode(0),
        m_pThard(0.),
        m_pTMPI(0.),
        m_accepted(false),
        m_isEmt(false),
        m_nAcceptSeq(0),
        m_nISRveto(0),
        m_nFSRveto(0) {}
  PowhegHooksSetterMethod(Pythia*, Settings*, Logger*): 
        m_showerModel(1),
        m_nFinal(-1),
        m_vetoMode(0),
        m_MPIvetoMode(0),
        m_QEDvetoMode(0),
        m_vetoCount(3),
        m_pThardMode(0),
        m_pTemtMode(0),
        m_emittedMode(0),
        m_pTdefMode(0),
        m_pThard(0.),
        m_pTMPI(0.),
        m_accepted(false),
        m_isEmt(false),
        m_nAcceptSeq(0),
        m_nISRveto(0),
        m_nFSRveto(0) {}
  ~PowhegHooksSetterMethod() {}

  //--------------------------------------------------------------------------

  // Initialize settings, detailing merging strategy to use.
  bool initAfterBeams() {
    m_nFinal      = settingsPtr->mode("POWHEG:nFinal");
    m_vetoMode    = settingsPtr->mode("POWHEG:veto");
    m_vetoCount   = settingsPtr->mode("POWHEG:vetoCount");
    m_pThardMode  = settingsPtr->mode("POWHEG:pThard");
    m_pTemtMode   = settingsPtr->mode("POWHEG:pTemt");
    m_emittedMode = settingsPtr->mode("POWHEG:emitted");
    m_pTdefMode   = settingsPtr->mode("POWHEG:pTdef");
    m_MPIvetoMode = settingsPtr->mode("POWHEG:MPIveto");
    m_QEDvetoMode = settingsPtr->mode("POWHEG:QEDveto");
    m_showerModel = settingsPtr->mode("PartonShowers:model");
    return true;
  }

  //--------------------------------------------------------------------------

  // Routines to calculate the pT (according to m_pTdefMode) in a branching:
  //   ISR: i (radiator after)  -> j (emitted after) k (radiator before)
  //   FSR: i (radiator before) -> j (emitted after) k (radiator after)
  // For the Pythia pT definitions, a recoiler (after) must be specified.

  // Top-level wrapper for shower pTs.
  inline double pT(const Event& e, int RadAfterBranch,
    int EmtAfterBranch, int RecAfterBranch, bool FSR) {
    // VINCIA pT definition.
    if (m_showerModel == 2)
      return pTvincia(e, RadAfterBranch, EmtAfterBranch, RecAfterBranch);
    // DIRE pT definition.
    if (m_showerModel == 3)
      return pTdire(e, RadAfterBranch, EmtAfterBranch, RecAfterBranch);
    return pTpythia(e, RadAfterBranch, EmtAfterBranch, RecAfterBranch, FSR);
  }

  //--------------------------------------------------------------------------

  // Compute the Pythia pT separation.
  // Based on pTLund function in History.cc and identical to pTevol.
  inline double pTpythia(const Event& e, int RadAfterBranch,
    int EmtAfterBranch, int RecAfterBranch, bool FSR) {

    // Convenient shorthands for later
    Vec4 radVec = e[RadAfterBranch].p();
    Vec4 emtVec = e[EmtAfterBranch].p();
    Vec4 recVec = e[RecAfterBranch].p();
    int  radID  = e[RadAfterBranch].id();

    // Calculate virtuality of splitting
    double sign = (FSR) ? 1. : -1.;
    Vec4 Q(radVec + sign * emtVec);
    double Qsq = sign * Q.m2Calc();

    // Mass term of radiator
    double m2Rad = (std::abs(radID) >= 4 && std::abs(radID) < 7) ?
                   pow2(particleDataPtr->m0(radID)) : 0.;

    // z values for FSR and ISR
    double z, pTnow;
    if (FSR) {
      // Construct 2 -> 3 variables
      Vec4 sum = radVec + recVec + emtVec;
      double m2Dip = sum.m2Calc();
      double x1 = 2. * (sum * radVec) / m2Dip;
      double x3 = 2. * (sum * emtVec) / m2Dip;
      z     = x1 / (x1 + x3);
      pTnow = z * (1. - z);

    } else {
      // Construct dipoles before/after splitting
      Vec4 qBR(radVec - emtVec + recVec);
      Vec4 qAR(radVec + recVec);
      z     = qBR.m2Calc() / qAR.m2Calc();
      pTnow = (1. - z);
    }

    // Virtuality with correct sign
    pTnow *= (Qsq - sign * m2Rad);

    // Can get negative pT for massive splittings.
    if (pTnow < 0.) {
      loggerPtr->WARNING_MSG("negative pT");
      return -1.;
    }

    // Return pT
    return sqrt(pTnow);
  }

  //--------------------------------------------------------------------------

  // Compute the Vincia pT as in Eq. (2.63)-(2.66) in arXiv:2003.00702.
  // Branching is assumed to be {13} {23} -> 1 3 2.
  inline double pTvincia(const Event& event, int i1, int i3, int i2) {

    // Shorthands.
    Vec4 p1 = event[i1].p();
    Vec4 p3 = event[i3].p();
    Vec4 p2 = event[i2].p();

    // Fetch mothers of 1 and 2.
    int iMoth1 = event[i1].mother1();
    int iMoth2 = event[i2].mother1();
    if (iMoth1 == 0 || iMoth2 == 0) {
      loggerPtr->ABORT_MSG("could not find mothers of particles");
      exit(1);
    }

    // Invariants defined as in Eq. (5) in arXiv:2008.09468.
    double mMoth1Sq = event[iMoth1].m2();
    double mMoth2Sq = event[iMoth2].m2();
    double sgn1 = event[i1].isFinal() ? 1. : -1.;
    double sgn2 = event[i2].isFinal() ? 1. : -1.;
    double qSq13 = sgn1*(m2(sgn1*p1+p3) - mMoth1Sq);
    double qSq23 = sgn2*(m2(sgn2*p2+p3) - mMoth2Sq);

    // Normalisation as in Eq. (6) in arXiv:2008.09468.
    double sMax = -1.;
    if (event[i1].isFinal() && event[i2].isFinal()) {
      // FF.
      sMax = m2(p1+p2+p3) - mMoth1Sq - mMoth2Sq;
    } else if ((event[i1].isResonance() && event[i2].isFinal())
      || (!event[i1].isFinal() && event[i2].isFinal())) {
      // RF or IF.
      sMax = 2.*p1*p3 + 2.*p1*p2;
    } else if ((event[i1].isFinal() && event[i2].isResonance())
      || (event[i1].isFinal() && !event[i2].isFinal())) {
      // FR or FI.
      sMax = 2.*p2*p3 + 2.*p1*p2;
    } else if (!event[i1].isFinal() || !event[i2].isFinal()) {
      // II.
      sMax = 2.*p1*p2;
    } else {
      loggerPtr->ABORT_MSG("could not determine branching type");
      exit(1);
    }

    // Calculate pT2 as in Eq. (5) in arXiv:2008.09468.
    double pT2now = qSq13*qSq23/sMax;

    // Sanity check.
    if (pT2now < 0.) {
      loggerPtr->WARNING_MSG("negative pT");
      return -1.;
    }

    // Return pT.
    return sqrt(pT2now);
  }

  //--------------------------------------------------------------------------

  // Compute the Dire pT as in
  // DireTimes::pT2_FF, DireTimes::pT2_FI,
  // DireSpace::pT2_IF, DireSpace::pT2_II.
  inline double pTdire(const Event& event, int iRad, int iEmt, int iRec) {

    // Shorthands.
    const Particle& rad = event[iRad];
    const Particle& emt = event[iEmt];
    const Particle& rec = event[iRec];

    // Calculate pT2 depending on dipole configuration.
    double pT2 = -1.;
    if (rad.isFinal() && rec.isFinal()) {
      // FF -- copied from DireTimes::pT2_FF.
      const double sij = 2.*rad.p()*emt.p();
      const double sik = 2.*rad.p()*rec.p();
      const double sjk = 2.*rec.p()*emt.p();
      pT2 = sij*sjk/(sij+sik+sjk);
    } else if (rad.isFinal() && !rec.isFinal()) {
      // FI.
      const double sij =  2.*rad.p()*emt.p();
      const double sai = -2.*rec.p()*rad.p();
      const double saj = -2.*rec.p()*emt.p();
      pT2 = sij*saj/(sai+saj)*(sij+saj+sai)/(sai+saj);
      if (sij+saj+sai < 1e-5 && std::abs(sij+saj+sai) < 1e-5) pT2 = sij;
    } else if (!rad.isFinal() && rec.isFinal()) {
      // IF.
      const double sai = -2.*rad.p()*emt.p();
      const double sik =  2.*rec.p()*emt.p();
      const double sak = -2.*rad.p()*rec.p();
      pT2 = sai*sik/(sai+sak)*(sai+sik+sak)/(sai+sak);
    } else if (!rad.isFinal() || !rec.isFinal()) {
      // II.
      const double sai = -2.*rad.p()*emt.p();
      const double sbi = -2.*rec.p()*emt.p();
      const double sab =  2.*rad.p()*rec.p();
      pT2 = sai*sbi/sab*(sai+sbi+sab)/sab;
    } else {
      loggerPtr->ABORT_MSG("could not determine branching type");
      exit(1);
    }

    // Sanity check.
    if (pT2 < 0.) {
      loggerPtr->WARNING_MSG("negative pT");
      return -1.;
    }

    // Return pT.
    return sqrt(pT2);
  }

  //--------------------------------------------------------------------------

  // Compute the POWHEG pT separation between i and j.
  inline double pTpowheg(const Event &e, int i, int j, bool FSR) {

    // pT value for FSR and ISR
    double pTnow = 0.;
    if (FSR) {
      // POWHEG d_ij (in CM frame). Note that the incoming beams have not
      // been updated in the parton systems pointer yet (i.e. prior to any
      // potential recoil).
      int iInA = partonSystemsPtr->getInA(0);
      int iInB = partonSystemsPtr->getInB(0);
      double betaZ = - ( e[iInA].pz() + e[iInB].pz() ) /
                       ( e[iInA].e()  + e[iInB].e()  );
      Vec4 iVecBst(e[i].p()), jVecBst(e[j].p());
      iVecBst.bst(0., 0., betaZ);
      jVecBst.bst(0., 0., betaZ);
      pTnow = sqrt( (iVecBst + jVecBst).m2Calc() *
                    iVecBst.e() * jVecBst.e() /
                    pow2(iVecBst.e() + jVecBst.e()) );

    } else {
      // POWHEG pT_ISR is just kinematic pT
      pTnow = e[j].pT();
    }

    // Check result.
    if (pTnow < 0.) {
      loggerPtr->WARNING_MSG("negative pT");
      return -1.;
    }

    return pTnow;
  }

  //--------------------------------------------------------------------------

  // Calculate pT for a splitting based on m_pTdefMode.
  // If j is -1, all final-state partons are tried.
  // If i, k, r and xSR are -1, then all incoming and outgoing
  // partons are tried.
  // xSR set to 0 means ISR, while xSR set to 1 means FSR.
  inline double pTcalc(const Event &e, int i, int j, int k, int r, int xSRin) {

    // Loop over ISR and FSR if necessary
    double pTemt = -1., pTnow;
    int xSR1 = (xSRin == -1) ? 0 : xSRin;
    int xSR2 = (xSRin == -1) ? 2 : xSRin + 1;
    for (int xSR = xSR1; xSR < xSR2; xSR++) {
      // FSR flag
      bool FSR = (xSR == 0) ? false : true;

      // If all necessary arguments have been given, then directly calculate.
      // POWHEG ISR and FSR, need i and j.
      if ((m_pTdefMode == 0 || m_pTdefMode == 1) && i > 0 && j > 0) {
        pTemt = pTpowheg(e, i, j, (m_pTdefMode == 0) ? false : FSR);

      // Pythia ISR, need i, j and r.
      } else if (!FSR && m_pTdefMode == 2 && i > 0 && j > 0 && r > 0) {
        pTemt = pT(e, i, j, r, FSR);

      // Pythia FSR, need k, j and r.
      } else if (FSR && m_pTdefMode == 2 && j > 0 && k > 0 && r > 0) {
        pTemt = pT(e, k, j, r, FSR);

      // Otherwise need to try all possible combinations.
      } else {
        // Start by finding incoming legs to the hard system after
        // branching (radiator after branching, i for ISR).
        // Use partonSystemsPtr to find incoming just prior to the
        // branching and track mothers.
        int iInA = partonSystemsPtr->getInA(0);
        int iInB = partonSystemsPtr->getInB(0);
        while (e[iInA].mother1() != 1) { iInA = e[iInA].mother1(); }
        while (e[iInB].mother1() != 2) { iInB = e[iInB].mother1(); }

        // If we do not have j, then try all final-state partons.
        int jNow = (j > 0) ? j : 0;
        int jMax = (j > 0) ? j + 1 : e.size();
        for (; jNow < jMax; jNow++) {

          // Final-state only.
          if (!e[jNow].isFinal()) continue;
          // Exclude photons (and W/Z!)
          if (m_QEDvetoMode==0 && e[jNow].colType() == 0) continue;

          // POWHEG.
          if (m_pTdefMode == 0 || m_pTdefMode == 1) {

            // ISR - only done once as just kinematical pT.
            if (!FSR) {
              pTnow = pTpowheg(e, iInA, jNow, (m_pTdefMode == 0) ? false : FSR);
              if (pTnow > 0.) pTemt = (pTemt < 0) ? pTnow : min(pTemt, pTnow);

            // FSR - try all outgoing partons from system before branching
            // as i. Note that for the hard system, there is no
            // "before branching" information.
            } else {

              int outSize = partonSystemsPtr->sizeOut(0);
              for (int iMem = 0; iMem < outSize; iMem++) {
                int iNow = partonSystemsPtr->getOut(0, iMem);

                // i != jNow and no carbon copies
                if (iNow == jNow ) continue;
                // Exclude photons (and W/Z!)
                if (m_QEDvetoMode==0 && e[iNow].colType() == 0) continue;
                if (jNow == e[iNow].daughter1()
                  && jNow == e[iNow].daughter2()) continue;

                pTnow = pTpowheg(e, iNow, jNow, (m_pTdefMode == 0)
                  ? false : FSR);
                if (pTnow > 0.) pTemt = (pTemt < 0)
                  ? pTnow : min(pTemt, pTnow);
              }
             // for (iMem)
            }
            // if (!FSR)
          // Pythia.
          } else if (m_pTdefMode == 2) {

            // ISR - other incoming as recoiler.
            if (!FSR) {
              pTnow = pT(e, iInA, jNow, iInB, FSR);
              if (pTnow > 0.) pTemt = (pTemt < 0) ? pTnow : min(pTemt, pTnow);
              pTnow = pT(e, iInB, jNow, iInA, FSR);
              if (pTnow > 0.) pTemt = (pTemt < 0) ? pTnow : min(pTemt, pTnow);

            // FSR - try all final-state coloured partons as radiator
            //       after emission (k).
            } else {
              for (int kNow = 0; kNow < e.size(); kNow++) {
                if (kNow == jNow || !e[kNow].isFinal()) continue;
                if (m_QEDvetoMode==0 && e[kNow].colType() == 0) continue;

                // For this kNow, need to have a recoiler.
                // Try two incoming.
                pTnow = pT(e, kNow, jNow, iInA, FSR);
                if (pTnow > 0.) pTemt = (pTemt < 0)
                  ? pTnow : min(pTemt, pTnow);
                pTnow = pT(e, kNow, jNow, iInB, FSR);
                if (pTnow > 0.) pTemt = (pTemt < 0)
                  ? pTnow : min(pTemt, pTnow);

                // Try all other outgoing.
                for (int rNow = 0; rNow < e.size(); rNow++) {
                  if (rNow == kNow || rNow == jNow ||
                      !e[rNow].isFinal()) continue;
                  if(m_QEDvetoMode==0 && e[rNow].colType() == 0) continue;
                  pTnow = pT(e, kNow, jNow, rNow, FSR);
                  if (pTnow > 0.) pTemt = (pTemt < 0)
                    ? pTnow : min(pTemt, pTnow);
                }
              // for (rNow)
              }
            // for (kNow)
            }
          // if (!FSR)
          }
        // if (m_pTdefMode)
        }
      // for (j)
      }
    }
    // for (xSR)

    return pTemt;
  }

  //--------------------------------------------------------------------------

  // Extraction of m_pThard based on the incoming event.
  // Assume that all the final-state particles are in a continuous block
  // at the end of the event and the final entry is the POWHEG emission.
  // If there is no POWHEG emission, then m_pThard is set to SCALUP.

  inline bool canVetoMPIStep()    { return true; }
  inline int  numberVetoMPIStep() { return 1; }
  inline bool doVetoMPIStep(int nMPI, const Event &e) {
    // Extra check on nMPI
    if (nMPI > 1) return false;
    int iEmt = -1;
    double pT1(0.), pTsum(0.);

    // When m_nFinal is set, be strict about comparing the number of final-state
    // particles with expectation from Born and single-real emission states.
    // (Note: the default from 8.309 onwards is m_nFinal = -1).
    if (m_nFinal > 0) {
      // Find if there is a POWHEG emission. Go backwards through the
      // event record until there is a non-final particle. Also sum pT and
      // find pT_1 for possible MPI vetoing
      int count = 0;
      for (int i = e.size() - 1; i > 0; i--) {
        if (e[i].isFinal()) {
          count++;
          pT1    = e[i].pT();
          pTsum += e[i].pT();
        } else break;
      }
      // Extra check that we have the correct final state
      if (count != m_nFinal && count != m_nFinal + 1) {
        loggerPtr->ABORT_MSG("wrong number of final state particles in event");
        exit(1);
      }
      // Flag if POWHEG radiation present and index
      m_isEmt = (count == m_nFinal) ? false : true;
      iEmt  = (m_isEmt) ? e.size() - 1 : -1;

    // If m_nFinal == -1, then go through the event and extract only the
    // information on the emission and its pT, but do not enforce strict
    // comparisons of final state multiplicity.
    } else {

      // Flag whether POWHEG radiation is present, and save index of emission.
      m_isEmt = false;
      for (int i = e.size() - 1; i > 0; i--) {
        if (e[i].isFinal()) {
          if ( e[i].isParton() && iEmt < 0
            && e[e[i].mother1()].isParton() ) {
            m_isEmt = true;
            iEmt = i;
          }
          pT1    = e[i].pT();
          pTsum += e[i].pT();
        } else break;
      }
    }

    // If there is no radiation or if m_pThardMode is 0 then set m_pThard = SCALUP.
    if (!m_isEmt || m_pThardMode == 0) {
      m_pThard = infoPtr->scalup();

    // If m_pThardMode is 1 then the pT of the POWHEG emission is checked against
    // all other incoming and outgoing partons, with the minimal value taken.
    } else if (m_pThardMode == 1) {
      if (m_nFinal < 0) {
        loggerPtr->WARNING_MSG(
          "pThard == 1 not available for m_nFinal == -1, reset pThard = 0. (But might be reset later by child class)");
        m_pThardMode = 0;
        m_pThard = infoPtr->scalup();
      } else {
        m_pThard = pTcalc(e, -1, iEmt, -1, -1, -1);
      }

    // If m_pThardMode is 2, then the pT of all final-state partons is checked
    // against all other incoming and outgoing partons, with the minimal value
    // taken.
    } else if (m_pThardMode == 2) {
      if (m_nFinal < 0) {
        loggerPtr->WARNING_MSG(
          "pThard == 2 not available for m_nFinal == -1, reset pThard = 0. (But might be reset later by child class)");
        m_pThardMode = 0;
        m_pThard = infoPtr->scalup();
      } else {
        m_pThard = pTcalc(e, -1, -1, -1, -1, -1);
      }
    }

    // Find MPI veto pT if necessary.
    if (m_MPIvetoMode == 1) {
      m_pTMPI = (m_isEmt) ? pTsum / 2. : pT1;
    }

    // Initialise other variables.
    m_accepted   = false;
    m_nAcceptSeq = m_nISRveto = m_nFSRveto = 0;

    // Do not veto the event
    return false;
  }

  //--------------------------------------------------------------------------

  // ISR veto.

  inline bool canVetoISREmission() { return (m_vetoMode == 0) ? false : true; }
  inline bool doVetoISREmission(int, const Event &e, int iSys) {
    // Must be radiation from the hard system
    if (iSys != 0) return false;

    // If we already have m_accepted 'm_vetoCount' emissions in a row, do nothing
    if (m_vetoMode == 1 && m_nAcceptSeq >= m_vetoCount) return false;

    // Pythia radiator after, emitted and recoiler after.
    int iRadAft = -1, iEmt = -1, iRecAft = -1;
    for (int i = e.size() - 1; i > 0; i--) {
      if (m_showerModel == 1) {
        // Pythia.
        if      (iRadAft == -1 && e[i].status() == -41) iRadAft = i;
        else if (iEmt    == -1 && e[i].status() ==  43) iEmt    = i;
        else if (iRecAft == -1 && e[i].status() == -42) iRecAft = i;
      } else if (m_showerModel == 2) {
        // Vincia.
        if      (iRadAft == -1 && e[i].status() == -41) iRadAft = i;
        else if (iEmt    == -1 && e[i].status() ==  43) iEmt    = i;
        else if (iRecAft == -1
          && (e[i].status() == -41 || e[i].status() == 44)) iRecAft = i;
      } else if (m_showerModel == 3) {
        // Dire.
        if      (iRadAft == -1 && e[i].status() == -41) iRadAft = i;
        else if (iEmt    == -1 && e[i].status() ==  43) iEmt    = i;
        else if (iRecAft == -1
          && (e[i].status() == -41
            || e[i].status() == 44 || e[i].status() == 48)) iRecAft = i;
      }
      if (iRadAft != -1 && iEmt != -1 && iRecAft != -1) break;
    }
    if (iRadAft == -1 || iEmt == -1 || iRecAft == -1) {
      loggerPtr->ABORT_MSG("could not find ISR emission");
      exit(1);
    }

    // m_pTemtMode == 0: pT of emitted w.r.t. radiator
    // m_pTemtMode == 1: min(pT of emitted w.r.t. all incoming/outgoing)
    // m_pTemtMode == 2: min(pT of all outgoing w.r.t. all incoming/outgoing)
    int xSR      = (m_pTemtMode == 0) ? 0       : -1;
    int i        = (m_pTemtMode == 0) ? iRadAft : -1;
    int j        = (m_pTemtMode != 2) ? iEmt    : -1;
    int k        = -1;
    int r        = (m_pTemtMode == 0) ? iRecAft : -1;
    double pTemt = pTcalc(e, i, j, k, r, xSR);

    // If a Born configuration, and a photon, and m_QEDvetoMode=2,
    //  then don't veto photons, W, or Z harder than m_pThard.
    bool vetoParton = (!m_isEmt && e[iEmt].colType()==0 && m_QEDvetoMode==2)
      ? false: true;

    // Veto if pTemt > m_pThard.
    if (pTemt > m_pThard) {
      if(!vetoParton) {
        // Don't veto ANY emissions afterwards.
        m_nAcceptSeq = m_vetoCount-1;
      } else {
        m_nAcceptSeq = 0;
        m_nISRveto++;
        return true;
      }
    }

    // Else mark that an emission has been m_accepted and continue.
    m_nAcceptSeq++;
    m_accepted = true;
    return false;
  }

  //--------------------------------------------------------------------------

  // FSR veto.

  inline bool canVetoFSREmission() { return (m_vetoMode == 0) ? false : true; }
  inline bool doVetoFSREmission(int, const Event &e, int iSys, bool) {
    // Must be radiation from the hard system.
    if (iSys != 0) return false;

    // If we already have m_accepted 'm_vetoCount' emissions in a row, do nothing.
    if (m_vetoMode == 1 && m_nAcceptSeq >= m_vetoCount) return false;

    // Pythia radiator (before and after), emitted and recoiler (after).
    int iRecAft = e.size() - 1;
    int iEmt    = e.size() - 2;
    int iRadAft = e.size() - 3;
    int iRadBef = e[iEmt].mother1();
    bool stop = false;
    if (m_showerModel == 1 || m_showerModel == 3) {
      // Pythia or Dire.
      if ( (e[iRecAft].status() != 52 && e[iRecAft].status() != -53) ||
        e[iEmt].status() != 51 || e[iRadAft].status() != 51) stop = true;
    } else if (m_showerModel == 2) {
      // Vincia.
      if ( (e[iRecAft].status() != 51 && e[iRecAft].status() != 52) ||
        e[iEmt].status() != 51 || e[iRadAft].status() != 51) stop = true;
    }
    if (stop) {
      loggerPtr->ABORT_MSG("could not find FSR emission");
      exit(1);
    }

    // Behaviour based on m_pTemtMode:
    //  0 - pT of emitted w.r.t. radiator before
    //  1 - min(pT of emitted w.r.t. all incoming/outgoing)
    //  2 - min(pT of all outgoing w.r.t. all incoming/outgoing)
    int xSR = (m_pTemtMode == 0) ? 1       : -1;
    int i   = (m_pTemtMode == 0) ? iRadBef : -1;
    int k   = (m_pTemtMode == 0) ? iRadAft : -1;
    int r   = (m_pTemtMode == 0) ? iRecAft : -1;

    // When m_pTemtMode is 0 or 1, iEmt has been selected.
    double pTemt = 0.;
    if (m_pTemtMode == 0 || m_pTemtMode == 1) {
      // Which parton is emitted, based on m_emittedMode:
      //  0 - Pythia definition of emitted
      //  1 - Pythia definition of radiated after emission
      //  2 - Random selection of emitted or radiated after emission
      //  3 - Try both emitted and radiated after emission
      int j = iRadAft;
      if (m_emittedMode == 0 || (m_emittedMode == 2 && rndmPtr->flat() < 0.5)) j++;

      for (int jLoop = 0; jLoop < 2; jLoop++) {
        if      (jLoop == 0) pTemt = pTcalc(e, i, j, k, r, xSR);
        else if (jLoop == 1) pTemt = min(pTemt, pTcalc(e, i, j, k, r, xSR));

        // For m_emittedMode == 3, have tried iRadAft, now try iEmt
        if (m_emittedMode != 3) break;
        if (k != -1) swap(j, k); else j = iEmt;
      }

    // If m_pTemtMode is 2, then try all final-state partons as emitted
    } else if (m_pTemtMode == 2) {
      pTemt = pTcalc(e, i, -1, k, r, xSR);

    }

    // If a Born configuration, and a photon, and m_QEDvetoMode=2,
    //  then don't veto photons, W's or Z's harder than m_pThard
    bool vetoParton = (!m_isEmt && e[iEmt].colType()==0 && m_QEDvetoMode==2)
      ? false: true;

    // Veto if pTemt > m_pThard
    if (pTemt > m_pThard) {
      if(!vetoParton) {
        // Don't veto ANY emissions afterwards
        m_nAcceptSeq = m_vetoCount-1;
      } else {
        m_nAcceptSeq = 0;
        m_nFSRveto++;
        return true;
      }
    }

    // Else mark that an emission has been m_accepted and continue
    m_nAcceptSeq++;
    m_accepted = true;
    return false;
  }

  //--------------------------------------------------------------------------

  // MPI veto.

  inline bool canVetoMPIEmission() {return (m_MPIvetoMode == 0) ? false : true;}
  inline bool doVetoMPIEmission(int, const Event &e) {
    if (m_MPIvetoMode == 1) {
      if (e[e.size() - 1].pT() > m_pTMPI) return true;
    }
    return false;
  }

  //--------------------------------------------------------------------------

  // Functions to return information.

  inline int getNISRveto() { return m_nISRveto; }
  inline int getNFSRveto() { return m_nFSRveto; }

  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // Setter function of m_pThard, if this is calculated outside of base class UserHook
  inline void setpThard(double newPThard) {m_pThard = newPThard;}

  //--------------------------------------------------------------------------

 private:
  int    m_showerModel, m_nFinal, m_vetoMode, m_MPIvetoMode, m_QEDvetoMode, m_vetoCount;
  int    m_pThardMode, m_pTemtMode, m_emittedMode, m_pTdefMode;
  double m_pThard, m_pTMPI;
  bool   m_accepted, m_isEmt;
  // The number of m_accepted emissions (in a row)
  // Flag for PowHeg Born or Radiation
  int m_nAcceptSeq;
  // Statistics on vetos
  unsigned long int m_nISRveto, m_nFSRveto;

};

//==========================================================================

} // end namespace Pythia8

#endif // end Pythia8_PowhegHooksSetterMethod_H
