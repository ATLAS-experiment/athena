# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.SystemOfUnits import GeV
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from AthenaConfiguration.ComponentFactory import CompFactory
#
# photon hypo alg
#
def createTrigEgammaPrecisionPhotonCaloIsoHypoAlg(name, sequenceOut, sequenceIn):

  thePrecisionPhotonCaloIsoHypo = CompFactory.TrigEgammaPrecisionPhotonCaloIsoHypoAlg(name)
  thePrecisionPhotonCaloIsoHypo.Photons = sequenceIn       # Key of the input photon container
  thePrecisionPhotonCaloIsoHypo.IsoPhotons = sequenceOut   # key of the output isolated photon container
  return thePrecisionPhotonCaloIsoHypo


#
# For photons
#
class TrigEgammaPrecisionPhotonCaloIsoHypoToolConfig:


  #Below are the configuration of the calorimeter isolation selections
  # The dictionary key is the working point (icaloloose, icalo medium and icalotight
  # the value is an array of three components where first component refers to a cut related to topoetcone20/pt, the second to topoetcone30/pt and third to topoetcone40/pt
  # __caloIsolationCut is the cut on the variable topoetcone[x]/pt
  # __caloEtconeCut is the cut on etcone[x]/pt Not used. But kept for backward compatibility
  # __caloIsolationOffset is the offset applied to that cut
  # so the selection is:
  # 
  
  __caloIsolationCut = {
                          None          : [None, None, None],
                          'icaloloose'  : [0.1  , 999., 999. ],
                          'icalomedium' : [0.075, 999., 999. ],
                          'icalotight'  : [999. , 999., 0.03 ]
                        }

  __caloEtconeCut = {
                          None          : [None, None, None],
                          'icaloloose'  : [999., 999., 999.],
                          'icalomedium' : [999., 999., 999.],
                          'icalotight'  : [999., 999., 999.]
                        }

  __caloIsolationOffset = {
                          None         : [None, None, None],
                          'icaloloose' : [0.,0.,0.],
                          'icalomedium': [0.,0.,0],
                          'icalotight' : [0.,0.,2.45*GeV]
                          }


  def __init__(self, name, monGroups, cpart, tool=None):

    from AthenaCommon.Logging import logging
    self.__log = logging.getLogger('TrigEgammaPrecisionPhotonCaloIsoHypoTool')
    self.__name       = name
    self.__isoinfo    = cpart['isoInfo']
    self.__monGroups = monGroups
    
    if not tool:
      tool = CompFactory.TrigEgammaPrecisionPhotonCaloIsoHypoTool( name )
     
    tool.EtaBins        = [0.0, 0.6, 0.8, 1.15, 1.37, 1.52, 1.81, 2.01, 2.37, 2.47]

    self.__tool = tool
    self.__log.debug( 'Chain     :%s', self.__name )
    self.__log.debug( 'isoinfo   :%s', self.__isoinfo )


  def isoInfo(self):
    return self.__isoinfo

  def tool(self):
    return self.__tool
  
  #
  # Isolation and nominal cut
  #
  def isoCut(self):

    if self.isoInfo() == 'noiso':
        self.tool().AcceptAll = True
        return
    self.tool().RelTopoEtConeCut = self.__caloIsolationCut[self.isoInfo()]
    self.tool().RelEtConeCut = self.__caloEtconeCut[self.isoInfo()]
    self.tool().Offset = self.__caloIsolationOffset[self.isoInfo()]
 



  #
  # Compile the chain
  #
  def compile(self, flags):

    if self.isoInfo() != 'noiso':
        if self.isoInfo() not in self.__caloIsolationCut.keys():
            self.__log.error('Isolation cut %s not defined!', self.isoInfo())
            
        self.__log.debug('Configuring Isolation cut %s for [topoetcone20/et, topoetcone30/et, topoetcone40/et]', self.isoInfo())
        self.__log.debug('         with values = %s and offsets = %s', 
                         str(self.__caloIsolationCut[self.isoInfo()]), 
                         str(self.__caloIsolationOffset[self.isoInfo()]))
    else:
        self.__log.debug('Configuring Isolation to AcceptAll (not applying any cut)')
    self.isoCut()


    if hasattr(self.tool(), "MonTool"):
      
      doValidationMonitoring = flags.Trigger.doValidationMonitoring # True to monitor all chains for validation purposes
      monGroups = self.__monGroups

      if (any('egammaMon:online' in group for group in monGroups) or doValidationMonitoring):
        self.addMonitoring(flags)


  #
  # Monitoring code
  #
  def addMonitoring(self, flags):

    monTool = GenericMonitoringTool(flags, "MonTool_"+self.__name,
                                    HistPath = 'PrecisionPhotonCaloIsoHypo/'+self.__name)
    monTool.defineHistogram('Et_em', type='TH1F', path='EXPERT', title="PrecisionPhotonCaloIso Hypo cluster E_{T}^{EM};E_{T}^{EM} [MeV]", xbins=50, xmin=-2000, xmax=100000)
    monTool.defineHistogram('Eta', type='TH1F', path='EXPERT', title="PrecisionPhotonCaloIso Hypo entries per Eta;Eta", xbins=100, xmin=-2.5, xmax=2.5)
    monTool.defineHistogram('Phi', type='TH1F', path='EXPERT', title="PrecisionPhotonCaloIso Hypo entries per Phi;Phi", xbins=128, xmin=-3.2, xmax=3.2)
    monTool.defineHistogram('EtaBin', type='TH1I', path='EXPERT', title="PrecisionPhotonCaloIso Hypo entries per Eta bin;Eta bin no.", xbins=11, xmin=-0.5, xmax=10.5)

    cuts=['Input','eta','Calo Iso']

    monTool.defineHistogram('CutCounter', type='TH1I', path='EXPERT', title="PrecisionPhotonCaloIso Hypo Passed Cuts;Cut",
                            xbins=13, xmin=-1.5, xmax=12.5,  opt="kCumulative", xlabels=cuts)

    if flags.Trigger.doValidationMonitoring:
      monTool.defineHistogram('etcone20',type='TH1F',path='EXPERT',title= "PrecisionPhotonCaloIso Hypo etcone20; etcone20;", xbins=50, xmin=0, xmax=5.0)
      monTool.defineHistogram('topoetcone20',type='TH1F',path='EXPERT',title= "PrecisionPhotonCaloIso Hypo; topoetcone20;", xbins=50, xmin=-10, xmax=10)
      monTool.defineHistogram('reletcone20',type='TH1F',path='EXPERT',title= "PrecisionPhotonCaloIso Hypo etcone20/et; etcone20/et;", xbins=50, xmin=-0.5, xmax=0.5)
      monTool.defineHistogram('reltopoetcone20',type='TH1F',path='EXPERT',title= "PrecisionPhotonCaloIso Hypo; topoetcone20/pt;", xbins=50, xmin=-0.5, xmax=0.5)

    self.tool().MonTool = monTool



def _IncTool( flags, name, monGroups, cpart, tool=None ):
    config = TrigEgammaPrecisionPhotonCaloIsoHypoToolConfig(name, monGroups, cpart, tool=tool)
    config.compile(flags)
    return config.tool()



def TrigEgammaPrecisionPhotonCaloIsoHypoToolFromDict(flags, d, tool=None):
    """ Use menu decoded chain dictionary to configure the tool """
    cparts = [i for i in d['chainParts'] if ((i['signature']=='Electron') or (i['signature']=='Photon'))] 
    return _IncTool( flags, d['chainName'], d['monGroups'], cparts[0], tool=tool )
