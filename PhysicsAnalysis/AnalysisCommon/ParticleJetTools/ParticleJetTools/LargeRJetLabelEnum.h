// Dear emacs, this is -*- c++ -*-
/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PARTICLEJETTOOLS_LARGERJETLABELENUM_H
#define PARTICLEJETTOOLS_LARGERJETLABELENUM_H

// ROOT include(s).
#include <TString.h>

namespace LargeRJetTruthLabel
{
  enum TypeEnum
  {
    UNKNOWN=0,    // Not tagged yet
    tqqb,         // fully-contained top->qqb
    Wqq,          // fully-contained W->qq
    Zbb,          // fully-contained Z->bb
    Zcc,          // fully-contained Z->cc
    Zqq,          // fully-contained Z->qq
    Wqq_From_t,   // fully-contained W->qq (also matched to top)
    other_From_t, // matched to top
    other_From_V, // matched to W/Z
    notruth,      // failed to truth-jet matching (pileup)
    qcd,          // not matched to top or W/Z (background jet)
    Hbb,          // fully-contained H->bb
    Hcc,          // fully-contained H->cc
    other_From_H, // matched to H
    HtautauEl,    // fully-contained H->tautau,  el
    HtautauMu,    // fully-contained H->tautau,  mu
    HtautauHad,   // fully-contained H->tautau, had
    ZtautauEl,    // fully-contained Z->tautau,  el
    ZtautauMu,    // fully-contained Z->tautau,  mu
    ZtautauHad,   // fully-contained Z->tautau, had
  };

  inline int enumToInt(const TypeEnum type)
  {
    switch (type) {
      case tqqb:         return 1;
      case Wqq:          return 2;
      case Zbb:          return 3;
      case Zcc:          return 4;
      case Zqq:          return 5;
      case Wqq_From_t:   return 6;
      case other_From_t: return 7;
      case other_From_V: return 8;
      case notruth:      return 9;
      case qcd:          return 10;
      case Hbb:          return 11;
      case Hcc:          return 12;
      case other_From_H: return 13;
      case HtautauEl:    return 14;
      case HtautauMu:    return 15;
      case HtautauHad:   return 16;
      case ZtautauEl:    return 17;
      case ZtautauMu:    return 18;
      case ZtautauHad:   return 19;
      default:           return 0;
      }
  }

  inline TypeEnum intToEnum(const int type)
  {
    switch (type) {
      case 1:  return tqqb;
      case 2:  return Wqq;
      case 3:  return Zbb;
      case 4:  return Zcc;
      case 5:  return Zqq;
      case 6:  return Wqq_From_t;
      case 7:  return other_From_t;
      case 8:  return other_From_V;
      case 9:  return notruth;
      case 10: return qcd;
      case 11: return Hbb;
      case 12: return Hcc;
      case 13: return other_From_H;
      case 14: return HtautauEl;
      case 15: return HtautauMu;
      case 16: return HtautauHad;
      case 17: return ZtautauEl;
      case 18: return ZtautauMu;
      case 19: return ZtautauHad;
      default: return UNKNOWN;
    }

  }

  inline TypeEnum stringToEnum(const TString& name)
  {
#define TRY(STRING) if (name.EqualTo(#STRING, TString::kIgnoreCase)) return STRING
    TRY(tqqb);
    TRY(Wqq);
    TRY(Zbb);
    TRY(Zcc);
    TRY(Zqq);
    TRY(Wqq_From_t);
    TRY(other_From_t);
    TRY(other_From_V);
    TRY(notruth);
    TRY(qcd);
    TRY(Hbb);
    TRY(Hcc);
    TRY(other_From_H);
    TRY(HtautauEl);
    TRY(HtautauMu);
    TRY(HtautauHad);
    TRY(ZtautauEl);
    TRY(ZtautauMu);
    TRY(ZtautauHad);
#undef TRY
    return UNKNOWN;
  }
}

#endif
