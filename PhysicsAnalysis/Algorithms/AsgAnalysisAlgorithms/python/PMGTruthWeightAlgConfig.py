# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


# ATTENTION: This is a ComponentAccumulator-based configuration file
# (not to be confused with ConfigAccumulator). If you are an analysis
# user you are likely looking for GeneratorAnalysisBlock in
# AsgAnalysisConfig.py. That uses the block-configuration generally
# used for algorithms under PhysicsAnalysis/Algorithms. If you are
# using the block configuration do not use this configuration. We
# generally do not provide ComponentAccumulator-based configurations
# for the CP algorithms, and a special exception was made for this
# algorithm. Do not cite this as example why any other
# ComponentAccumulator-based configurations should be added. You can
# use this as an example for how ComponentAccumulator-based
# configurations can look like, but do not try to extend this to cover
# more CP algorithms. That is not supported, and you are likely to
# encounter problems if you scale this up in the wrong way. This
# configuration is covered by a unit test. Should it fail use your
# best judgement whether to fix this configuration or to change it to
# wrap the block configuration instead.

def PMGTruthWeightAlgCfg(flags, systematicsRegex='.*'):
    """Decorate systematically varied generator weights to
    'EventInfo', with a human-readable name.

    With systematicsRegex set to 'None', do not add the
    SystematicsSvc.

    """
    ca = ComponentAccumulator()
    # we need both the systematics service and the metadata service to
    # make this tool work.
    if systematicsRegex is not None:
        ca.addService(
            CompFactory.CP.SystematicsSvc(
                name="SystematicsSvc",
                sigmaRecommended=1,
                systematicsRegex=systematicsRegex,
            )
        )
    ca.merge(MetaDataSvcCfg(flags))
    ca.addEventAlgo(
        CompFactory.CP.PMGTruthWeightAlg(
            name="PMGTruthWeightAlg",
            truthWeightTool=CompFactory.PMGTools.PMGTruthWeightTool(
                name="PMGTruthWeightTool"
            ),
            decoration = 'generatorWeight_%SYS%',
        )
    )
    return ca
