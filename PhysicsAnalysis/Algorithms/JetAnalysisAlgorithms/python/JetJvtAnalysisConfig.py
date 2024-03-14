# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class JetJvtAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the JVT sequence"""

    def __init__ (self, containerName) :
        super (JetJvtAnalysisConfig, self).__init__ ()
        self.containerName = containerName
        self.addOption ('postfix', '', type=str)
        self.addOption ('enableFJvt', False, type=bool)


    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        # Set up the per-event jet efficiency scale factor calculation algorithm
        if config.dataType() is not DataType.Data:
            alg = config.createAlgorithm( 'CP::AsgEventScaleFactorAlg', 'JvtEventScaleFactorAlg' + postfix )
            preselection = config.getFullSelection (self.containerName, '')
            alg.preselection = preselection + '&&no_jvt' if preselection else 'no_jvt'
            alg.scaleFactorInputDecoration = 'jvt_effSF_%SYS%'
            alg.scaleFactorOutputDecoration = 'jvt_effSF_%SYS%'
            alg.particles = config.readName (self.containerName)

            config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration, 'weight_jvt_effSF')

            if self.enableFJvt:
                alg = config.createAlgorithm( 'CP::AsgEventScaleFactorAlg', 'ForwardJvtEventScaleFactorAlg' )
                preselection = config.getFullSelection (self.containerName, '')
                alg.preselection = preselection + '&&no_fjvt' if preselection else 'no_fjvt'
                alg.scaleFactorInputDecoration = 'fjvt_effSF_%SYS%'
                alg.scaleFactorOutputDecoration = 'fjvt_effSF_%SYS%'
                alg.particles = config.readName (self.containerName)

                config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration, 'weight_fjvt_effSF')


def makeJetJvtAnalysisConfig( seq, containerName,
                              postfix = None,
                              enableFJvt = None ):
    """Create a jet JVT analysis algorithm config

    Keyword arguments:
      enableFJvt -- Whether to enable forward JVT calculations
    """

    config = JetJvtAnalysisConfig (containerName)
    config.setOptionValue ('postfix', postfix)
    config.setOptionValue ('enableFJvt', enableFJvt)

    seq.append (config)
