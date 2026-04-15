#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "MeasurementToTruthAssociationAlg.icc"

#include "ClusterToTruthAssociation.h"
namespace ActsTrk {
   // instantiate the templates
   template class MeasurementToTruthAssociationAlg<xAOD::PixelClusterContainer,
                                                   InDetSimDataCollection,
                                                   xAODTruthParticleLinkVector,
                                                   MeasurementToTruthAssociationDebugHistograms>;
   template class MeasurementToTruthAssociationAlg<xAOD::StripClusterContainer,
                                                   InDetSimDataCollection,
                                                   xAODTruthParticleLinkVector,
                                                   MeasurementToTruthAssociationDebugHistograms>;
   template class MeasurementToTruthAssociationAlg<xAOD::HGTDClusterContainer,
                                                   InDetSimDataCollection,
                                                   xAODTruthParticleLinkVector,
                                                   MeasurementToTruthAssociationDebugHistograms>;                 
}
