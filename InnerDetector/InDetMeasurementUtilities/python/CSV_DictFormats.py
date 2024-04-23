# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

CSV_DictFormats = {
                    #All obtained AuxElement content using the HitsToxAODCopier tool
                    "PixelHits": { "col": "int", "row": "int", "tot": "int", "eta_module": "int", "phi_module": "int", "layer_disk": "int", "barrel_ec": "int", "detid": "unsigned long"},
                    "StripHits": { "strip": "int", "side": "int", "eta_module": "int", "phi_module": "int", "layer_disk": "int", "barrel_ec": "int", "detid": "unsigned long"},
                    
                    # Variables from the container:
                    #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/PixelClusterAuxContainer_v1.h
                    "ITkPixelClusters": {"globalPosition":"std::array<float,3>", "channelsInPhi":"int", "channelsInEta":"int", "widthInEta":"float", "omegaX":"float", "omegaY":"float", "totalToT":"int", "totalCharge":"float",   "energyLoss":"float", "splitProbability1":"float", "splitProbability2":"float", "lvl1a":"int"},
                    # ITkPixelClusters.isSplit is char with all ''? Keeping out for the moment

                    # Variables from the container:
                    #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/StripClusterAuxContainer_v1.h
                    "ITkStripClusters": {"globalPosition":"std::array<float,3>", "channelsInPhi":"int"},

                    # Variables from the container:
                    #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/SpacePoint_v1.h
                    "ITkStripSpacePoints": {"globalPosition":"std::array<float,3>", "radius":"float", "varianceR":"float", "varianceZ":"float", "topHalfStripLength":"float", "bottomHalfStripLength":"float", "topStripDirection":"xAOD::ArrayFloat3", "bottomStripDirection":"xAOD::ArrayFloat3", "stripCenterDistance":"xAOD::ArrayFloat3", "topStripCenter":"xAOD::ArrayFloat3"},

                    # Variables from the container:
                    #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/SpacePoint_v1.h
                    "ITkPixelSpacePoints": {"globalPosition":"std::array<float,3>", "radius":"float", "varianceR":"float", "varianceZ":"float"},

                    # Variables from the container:
                    #https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODTracking/xAODTracking/versions/TrackSurfaceAuxContainer_v1.h
                    # This container is empty
                    "ActsTrackSurfaces":
                    {
                        "translation":"float",
                        "rotation":"float",
                        "boundValues":"float",
                        "surfaceType":"int",
                    },

                    "ActsTrackMeasurements":
                    {
                        "meas":"vector<double>",
                        "covMatrix":"vector<double>",
                    },
 
                    "ActsTrackStates":
                    {
                        "chi2":"float",
                        "pathLength":"float",
                        "typeFlags":"uint64_t",
                        "geometryId":"uint64_t",
                    },

                    "ActsTrackSummary":
                    {
                        "chi2f":"float",
                        "nMeasurements":"unsigned int",
                        "nHoles":"unsigned int",
                        "ndf":"unsigned int",
                        "nOutliers":"unsigned int",
                        "nSharedHits":"unsigned int",
                        "tipIndex":"unsigned int",
                        "stemIndex":"unsigned int",
                        "particleHypothesis":"uint8_t",
                        "params":"vector<double>",
                        "covParams":"vector<double>",
                    },
          
                    "ActsTrackJacobians":
                    {
                        "jac":"vector<double>",
                    },
                   
                    "ActsTrackParameters":
                    {
                        "params":"vector<double>",
                        "covMatrix":"vector<double>",   
                    },
                   
                    "TruthParticles":
                    {
                        "pdgId":"int",
                        "barcode":"int",
                        "status":"int",
                        "px":"float", "py":"float", "pz":"float",
                        "e":"float","m":"float"
                    },

                    "TruthVertices":
                    {
                        "id":"int",
                        "barcode":"int",
                        "x":"float", "y":"float", "z":"float",
                        "t":"float"
                    },
                    
                    # Variables from the container:
                    #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODTracking/xAODTracking/versions/TrackParticle_v1.h
                    "InDetTrackParticles": 
                    {
                        "eta":"float", "theta":"float", "phi":"float", "charge":"float", "d0":"float", "z0":"float", "qOverP":"float",
                        "definingParametersCovMatrixDiag":"vector<float>", "definingParametersCovMatrixOffDiag":"vector<float>",
                        "vx":"float", "vy":"float", "vz":"float",
                        "radiusOfFirstHit":"float", "identifierOfFirstHit":"uint64_t",                        

                        # Variables from the container:
                        #https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODTracking/xAODTracking/versions/TrackParticleAuxContainer_v5.h
                        # For these variables the getConstDataSpan is failing, read using getattr    
                        "pt":"float",  
                        "m":"float",
                        "e":"float",
                        "rapidity":"float",
                        "phi0":"float",
                        "numberOfParameters":"unsigned int",  
                    },
               
}                                            

