#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "MeasurementToTruthAssociationAlg.icc"

#include "ClusterToTruthAssociation.h"
namespace ActsTrk {
   // instantiate the templates
   template class MeasurementToTruthAssociationAlg<xAOD::PixelClusterContainerAlt,
                                                   InDetSimDataCollection,
                                                   xAODTruthParticleLinkVector,
                                                   MeasurementToTruthAssociationDebugHistograms>;
   template class MeasurementToTruthAssociationAlg<xAOD::StripClusterContainerAlt,
                                                   InDetSimDataCollection,
                                                   xAODTruthParticleLinkVector,
                                                   MeasurementToTruthAssociationDebugHistograms>;
   template class MeasurementToTruthAssociationAlg<xAOD::HGTDClusterContainer,
                                                   InDetSimDataCollection,
                                                   xAODTruthParticleLinkVector,
                                                   MeasurementToTruthAssociationDebugHistograms>;                 
}
