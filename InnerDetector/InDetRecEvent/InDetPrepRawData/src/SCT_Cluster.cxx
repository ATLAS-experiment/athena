/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SCT_Cluster.cxx
//   Implementation file for class SCT_Cluster
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Version 1.0 15/07/2003 Veronique Boisvert
///////////////////////////////////////////////////////////////////

#include "InDetPrepRawData/SCT_Cluster.h"
// forward includes
#include "GaudiKernel/MsgStream.h"
#include "InDetPrepRawData/SiWidth.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include <ostream>
#include <sstream>


namespace InDet{

SCT_Cluster::SCT_Cluster(const Identifier& RDOId,
                         const Amg::Vector2D& locpos,
                         std::vector<Identifier>&& rdoList,
                         const InDet::SiWidth& width,
                         const InDetDD::SiDetectorElement* detEl,
                         Amg::MatrixX&& locErrMat)
  : SiCluster(RDOId,
              locpos,
              std::move(rdoList),
              width,
              detEl,
              std::move(locErrMat))
  , m_hitsInThirdTimeBin(0)
{}

SCT_Cluster::~SCT_Cluster() = default;

MsgStream&    operator << (MsgStream& stream,    const SCT_Cluster& prd)
{
    return prd.dump(stream);
}

std::ostream& operator << (std::ostream& stream, const SCT_Cluster& prd)
{
    return prd.dump(stream);
}

bool
SCT_Cluster::type(Trk::PrepRawDataType type) const{
  return (type == Trk::PrepRawDataType::SCT_Cluster or type == Trk::PrepRawDataType::SiCluster);
}
Trk::PrepRawDataType SCT_Cluster::prdType() const {
  return Trk::PrepRawDataType::SCT_Cluster;
}


MsgStream& SCT_Cluster::dump( MsgStream&    stream) const
{
    std::ostringstream out;
    dump(out);
    stream<<out.str();
    return stream;
}

std::ostream& SCT_Cluster::dump( std::ostream&    stream) const
{
    stream << "SCT_Cluster object"<<std::endl;
    stream <<  "Base class (SiCluster):" << std::endl;
    this->SiCluster::dump(stream);

    return stream;
}


}//end of ns
