include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthGen",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

if not hasattr(filtSeq, "xAODParticleDecayFilter"):
    from GeneratorFilters.GeneratorFiltersConf import xAODParticleDecayFilter
    filtSeq += xAODParticleDecayFilter("xAODParticleDecayFilter")

# Example usage of this filter
#xAODParticleDecayFilter = filtSeq.xAODParticleDecayFilter
#xAODParticleDecayFilter.Ptcut = 0.0
#xAODParticleDecayFilter.Etacut = 10.0
#xAODParticleDecayFilter.PDG = 1000022
#xAODParticleDecayFilter.MinParts = 2
#xAODParticleDecayFilter.StatusReq = 11
