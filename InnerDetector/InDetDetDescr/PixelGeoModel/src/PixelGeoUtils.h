/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#ifndef PIXEL_GEO_UTILS_H
#define PIXEL_GEO_UTILS_H

#include "ReadoutGeometryBase/PixelDiodeTree.h"
#include "PixelReadoutDefinitions/PixelReadoutDefinitions.h"
#include <array>
#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"

#define MSG_HELPER(LEVEL,body) if (this->msgLvl(LEVEL)) { this->msg(LEVEL) << body << endmsg;} do {} while (0)
#define GEO_MSG_DEBUG(body) MSG_HELPER(MSG::DEBUG,body)
#define GEO_MSG_INFO(body) MSG_HELPER(MSG::INFO,body)
#define GEO_MSG_WARNING(body) MSG_HELPER(MSG::WARNING,body)
#define GEO_MSG_ERROR(body) MSG_HELPER(MSG::ERROR,body)

namespace InDetDD {
   namespace detail {
      enum EPixelLocation {kCentral,kOuterEdge,kInnerEdge, kNPixelLocations};
      enum EDirection {kPhi, kEta,kNDirections};

      enum class FENumbering {
         kRegular,
         kMirror
      };

      struct MessagingAdapter {
         virtual ~MessagingAdapter() = default;
         virtual MsgStream& msg (const MSG::Level lvl) const = 0;
         virtual bool msgLvl (const MSG::Level lvl) const = 0;
      };

      // helper to allow usage of MSG macros
      struct PixelDiodeTreeMakerBase : public MessagingAdapter {
         PixelDiodeTree make(InDetDD::PixelReadoutTechnology readoutTechnology,
                             const std::array<int,kNDirections> &circuits,
                             const std::array<int,kNDirections> &dimPerCircuit,
                             const std::array<std::array<double,kNDirections>,kNPixelLocations> &pitch,
                             FENumbering fe_numbering);
      };

      template <class T_MsgParent>
      struct PixelDiodeTreeMaker final : public PixelDiodeTreeMakerBase {
         T_MsgParent* m_msgParent;
         PixelDiodeTreeMaker(T_MsgParent* msgParent) : m_msgParent(msgParent) {}
         virtual MsgStream& msg (const MSG::Level lvl) const override { return m_msgParent->msg(lvl); }
         virtual bool msgLvl (const MSG::Level lvl) const override { return m_msgParent ? m_msgParent->msgLvl(lvl) : false; }
      };

      // create a pixel diode tree for run1-3 pixel, DBM
      template <class T_MsgParent>
      inline PixelDiodeTree makePixelDiodeTree(T_MsgParent* gmt_mgr,
                                               InDetDD::PixelReadoutTechnology readoutTechnology,
                                               const std::array<int,kNDirections> &circuits,
                                               const std::array<int,kNDirections> &dimPerCircuit,
                                               const std::array<std::array<double,kNDirections>,kNPixelLocations> &pitch,
                                               FENumbering fe_numbering) {

         PixelDiodeTreeMaker<T_MsgParent> maker(gmt_mgr);
         auto ret=maker.make(readoutTechnology, circuits, dimPerCircuit, pitch, fe_numbering);
         return ret;
      }
   }
}

#endif
