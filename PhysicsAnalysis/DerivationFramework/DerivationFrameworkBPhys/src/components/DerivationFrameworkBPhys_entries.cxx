#include "../Reco_Vertex.h"
#include "../Reco_4mu.h"
#include "../Select_onia2mumu.h"
#include "../Thin_vtxTrk.h"
#include "../Thin_vtxDuplicates.h"
#include "../AugOriginalCounts.h"
#include "../BPhysPVThinningTool.h"
#include "../VertexCaloIsolation.h"
#include "../VertexTrackIsolation.h"
#include "../BPhysMetadataBase.h"
#include "../Bmumu_metadata.h"
#include "../BdKstarMuMu_metadata.h"
#include "../MuPlusDpstCascade.h"
#include "../MuPlusDsCascade.h"
#include "../Reco_mumu.h"
#include "../FourMuonTool.h"
#include "../AnyVertexSkimmingTool.h"
#include "../BTrackVertexMapLogger.h"
#include "../VertexPlus1TrackCascade.h"
#include "../TriggerCountToMetadata.h"
#include "../MuonExtrapolationTool.h"
#include "DerivationFrameworkBPhys/CascadeTools.h"
#include "../Reco_V0Finder.h"
#include "../JpsiPlusV0Cascade.h"
#include "../JpsiPlusDsCascade.h"
#include "../JpsiPlusDpstCascade.h"
#include "../JpsiPlusDs1Cascade.h"
#include "../JpsiPlusPsiCascade.h"
#include "../PsiPlusPsiCascade.h"
#include "../ReVertex.h"
#include "../BPhysConversionFinder.h"
#include "../Cascade3Plus1.h"
#include "../BPhysBGammaFinder.h"
#include "../PsiPlusPsiSingleVertex.h"
#include "../JpsiXPlusDisplaced.h"
#include "../JpsiXPlus2V0.h"

using namespace DerivationFramework;

DECLARE_COMPONENT( Reco_4mu )
DECLARE_COMPONENT( Reco_mumu )
DECLARE_COMPONENT( Reco_Vertex )
DECLARE_COMPONENT( Select_onia2mumu )
DECLARE_COMPONENT( Thin_vtxTrk )
DECLARE_COMPONENT( Thin_vtxDuplicates )
DECLARE_COMPONENT( AugOriginalCounts )
DECLARE_COMPONENT( BPhysPVThinningTool )
DECLARE_COMPONENT( VertexCaloIsolation )
DECLARE_COMPONENT( VertexTrackIsolation )
DECLARE_COMPONENT( BPhysMetadataBase )
DECLARE_COMPONENT( Bmumu_metadata )
DECLARE_COMPONENT( BdKstarMuMu_metadata )
DECLARE_COMPONENT( MuPlusDpstCascade )
DECLARE_COMPONENT( MuPlusDsCascade )
DECLARE_COMPONENT( AnyVertexSkimmingTool )
DECLARE_COMPONENT( FourMuonTool )
DECLARE_COMPONENT( BTrackVertexMapLogger )
DECLARE_COMPONENT( PsiPlusPsiCascade )
DECLARE_COMPONENT( VertexPlus1TrackCascade )
DECLARE_COMPONENT( TriggerCountToMetadata )
DECLARE_COMPONENT( MuonExtrapolationTool )
DECLARE_COMPONENT( CascadeTools )
DECLARE_COMPONENT( Reco_V0Finder )
DECLARE_COMPONENT( JpsiPlusV0Cascade )
DECLARE_COMPONENT( JpsiPlusDsCascade )
DECLARE_COMPONENT( JpsiPlusDpstCascade )
DECLARE_COMPONENT( JpsiPlusDs1Cascade )
DECLARE_COMPONENT( JpsiPlusPsiCascade )
DECLARE_COMPONENT( ReVertex )
DECLARE_COMPONENT( BPhysConversionFinder )
DECLARE_COMPONENT( Cascade3Plus1 )
DECLARE_COMPONENT( BPhysBGammaFinder )
DECLARE_COMPONENT( PsiPlusPsiSingleVertex )
DECLARE_COMPONENT( JpsiXPlusDisplaced )
DECLARE_COMPONENT( JpsiXPlus2V0 )
