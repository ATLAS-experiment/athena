/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Segment.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include <memory>

#include "TrkEventPrimitives/FitQuality.h"
#include "TrkSegment/Segment.h"

// default constructor
Trk::Segment::Segment()
  : Trk::MeasurementBase()
  , Trk::ObjectCounter<Trk::Segment>()
  , m_fitQuality(nullptr)
  , m_containedMeasBases()
  , m_author(AuthorUnknown)
{
}

Trk::Segment::Segment(Trk::LocalParameters&& locpars,
                      Amg::MatrixX&& locerr,
                      DataVector<const MeasurementBase>&& measurements,
                      FitQuality* fitqual,
                      Author author)
  : Trk::MeasurementBase(std::move(locpars), std::move(locerr))
  , Trk::ObjectCounter<Trk::Segment>()
  , m_fitQuality(fitqual)
  , m_containedMeasBases(std::move(measurements))
  , m_author(author)
{
}

// copy constructor
Trk::Segment::Segment(const Trk::Segment& seg)
  : Trk::MeasurementBase(seg)
  , Trk::ObjectCounter<Trk::Segment>(seg)
  , m_fitQuality(seg.m_fitQuality ? seg.m_fitQuality->clone() : nullptr)
  , m_containedMeasBases()
  , m_author(seg.m_author)
{
  // DV deep copy
  m_containedMeasBases.reserve(seg.m_containedMeasBases.size());
  for (const Trk::MeasurementBase* const measurement :
       seg.m_containedMeasBases) {
    m_containedMeasBases.push_back(measurement->clone());
  }
}

// move constructor
Trk::Segment::Segment(Trk::Segment&& seg) noexcept
  : Trk::MeasurementBase(seg)
  , m_fitQuality(std::move(seg.m_fitQuality))
  , m_containedMeasBases(std::move(seg.m_containedMeasBases))
  , m_author(seg.m_author)
{
}

// destructor - child save
Trk::Segment::~Segment() = default;

// assignment operator
Trk::Segment&
Trk::Segment::operator=(const Trk::Segment& seg)
{
  if (this != &seg) {
    Trk::MeasurementBase::operator=(seg);
    m_fitQuality.reset(seg.m_fitQuality ? seg.m_fitQuality->clone() : nullptr);
    // Deep copy
    m_containedMeasBases.clear();
    m_containedMeasBases.reserve(seg.m_containedMeasBases.size());
    for (const Trk::MeasurementBase* const measurement :
         seg.m_containedMeasBases) {
      m_containedMeasBases.push_back(measurement->clone());
    }
    m_author = seg.m_author;
  }
  return (*this);
}

// move assignment operator
//coverity[exn_spec_violation]
Trk::Segment&
Trk::Segment::operator=(Trk::Segment&& seg) noexcept
{
  if (this != &seg) {
    Trk::MeasurementBase::operator=(seg);
    m_fitQuality = std::move(seg.m_fitQuality);
    //called function throws exception of type SG::ExcInsertionInBaseClass.
    m_containedMeasBases = std::move(seg.m_containedMeasBases);
    m_author = seg.m_author;
  }
  return (*this);
}

std::string Trk::Segment::dumpAuthor() const {
  std::string author;
  switch (m_author) {
    case AuthorUnknown:
      author = "AuthorUnknown";
      break;
    case MooMdtSegmentMakerTool:
      author = "MooMdtSegmentMakerTool";
      break;
    case MooCscSegmentMakerTool:
      author = "MooCscSegmentMakerTool";
      break;
    case Muonboy:
      author = "Muonboy";
      break;
    case DCMathSegmentMaker:
      author = "DCMathSegmentMaker";
      break;
    case MDT_DHoughSegmentMakerTool:
      author = "MDT_DHoughSegmentMakerTool";
      break;
    case CSC_DHoughSegmentMakerTool:
      author = "CSC_DHoughSegmentMakerTool";
      break;
    case Csc2dSegmentMaker:
      author = "Csc2dSegmentMaker";
      break;
    case Csc4dSegmentMaker:
      author = "Csc4dSegmentMaker";
      break;
    case TRT_SegmentMaker:
      author = "TRT_SegmentMaker";
      break;
    case NswStereoSeeded:
      author = "Nsw MM stereo seeded";
      break;
    case NswStgcSeeded:
      author = "Nsw sTgc seeded";
      break;
    case NswQuadAlign:
      author = "Nsw single quad";
      break;
    case NswPadSeeded:
      author = "Nsw single quad";
      break;
    default:
      author = "Unrecognised author, enum = " + std::to_string(m_author);
      break;
  }
  return author;
}
void Trk::Segment::setAuthor(Author a) {
   m_author = a;
}
