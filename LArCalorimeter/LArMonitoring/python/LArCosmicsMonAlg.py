
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def LArCosmicsMonConfig(flags):
    '''Function to configures some algorithms in the monitoring system.'''

    # The following class will make a sequence, configure algorithms, and link                                                                   
    # them to GenericMonitoringTools                                                                                                                                 
    
    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'LArCosmicsMonAlgCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    LArCosmicsMonConfigCore(helper, CompFactory.LArCosmicsMonAlg,flags)

    return helper.result()


def LArCosmicsMonConfigCore(helper, algoinstance,flags):


    from LArMonitoring.GlobalVariables import lArDQGlobals

    larCosmicsMonAlg = helper.addAlgorithm(algoinstance,'larCosmicsMonAlg')

    larCosmicsMonAlg.CosmicsMonGroupName = 'LarCosmicsMonGroup'
    larCosmicsMonAlg.MuonADCthreshold_EM_barrel = 30
    larCosmicsMonAlg.MuonADCthreshold_EM_endcap = 40
    larCosmicsMonAlg.MuonADCthreshold_HEC = 40
    larCosmicsMonAlg.MuonADCthreshold_FCAL = 40
    larCosmicsMonAlg.ProblemsToMask=["deadReadout","deadPhys","short","almostDead","highNoiseHG","highNoiseMG","highNoiseLG","sporadicBurstNoise"]
    

    #mon group 
    cosmicMonGroup = helper.addGroup(
        larCosmicsMonAlg,
        larCosmicsMonAlg.CosmicsMonGroupName,
        '/LAr/',
        'run'
    )

    cosmic_path="Cosmics/"

    EM_bins=sorted(list(set(lArDQGlobals.Cell_Variables["etaRange"]["EMEC"]["C"]["2"]+lArDQGlobals.Cell_Variables["etaRange"]["EMB"]["C"]["2"]+lArDQGlobals.Cell_Variables["etaRange"]["EMB"]["A"]["2"]+lArDQGlobals.Cell_Variables["etaRange"]["EMEC"]["A"]["2"])))
    cosmicMonGroup.defineHistogram('mon_eta_EM,mon_phi;Muon2DHitsECAL',
                                   type='TH2F', 
                                   path=cosmic_path,
                                   title='Cosmics Seeds - Digit Max > '+str(int(larCosmicsMonAlg.MuonADCthreshold_EM_barrel))+'/'+str(int(larCosmicsMonAlg.MuonADCthreshold_EM_endcap))+' [ADC] in S2 Barrel/Endcap - EM;#eta cell;#phi cell;Number of Hits',
                                   xbins=EM_bins,
                                   ybins=lArDQGlobals.Cell_Variables["phiRange"]["EMB"]["A"]["2"])

    HEC_bins=sorted(list(set(lArDQGlobals.Cell_Variables["etaRange"]["HEC"]["C"]["1"]+lArDQGlobals.Cell_Variables["etaRange"]["HEC"]["A"]["1"])))
    cosmicMonGroup.defineHistogram('mon_eta_HEC,mon_phi;Muon2DHitsHCAL',
                                   type='TH2F', 
                                   path=cosmic_path,
                                   title='Cosmics Seeds - Digit Max > '+str(int(larCosmicsMonAlg.MuonADCthreshold_HEC))+' [ADC] in S1 HEC;#eta cell;#phi cell;Number of Hits',
                                   xbins=HEC_bins,
                                   ybins=lArDQGlobals.Cell_Variables["phiRange"]["HEC"]["A"]["1"])

    FCal_bins=sorted(list(set(lArDQGlobals.Cell_Variables["etaRange"]["FCal"]["C"]["2"]+lArDQGlobals.Cell_Variables["etaRange"]["FCal"]["A"]["2"])))
    cosmicMonGroup.defineHistogram('mon_eta_FCal,mon_phi;Muon2DHitsFCAL',
                                   type='TH2F', 
                                   path=cosmic_path,
                                   title='Cosmics Seeds - Digit Max > '+str(int(larCosmicsMonAlg.MuonADCthreshold_FCAL))+' [ADC] in S2 FCal;#eta cell;#phi cell;Number of Hits',
                                   xbins=FCal_bins,
                                   ybins=lArDQGlobals.Cell_Variables["phiRange"]["FCal"]["A"]["2"])




if __name__=='__main__':

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()

   from AthenaCommon.Logging import log
   from AthenaCommon.Constants import WARNING
   log.setLevel(WARNING)

   from LArMonitoring.LArMonConfigFlags import addLArMonFlags
   flags.addFlagsCategory("LArMon", addLArMonFlags)

   from AthenaConfiguration.TestDefaults import defaultTestFiles
   flags.Input.Files = defaultTestFiles.RAW_RUN2

   flags.Output.HISTFileName = 'LArCosmicsMonOutput.root'
   flags.DQ.enableLumiAccess = False
   flags.DQ.useTrigger = False
   flags.lock()

   from CaloRec.CaloRecoConfig import CaloRecoCfg
   cfg=CaloRecoCfg(flags)

   from LArCellRec.LArNoisyROSummaryConfig import LArNoisyROSummaryCfg
   cfg.merge(LArNoisyROSummaryCfg(flags))

   cosm_acc = LArCosmicsMonConfig(flags)
   cfg.merge(cosm_acc)

   flags.dump()
   f=open("LArCosmicsMon.pkl","wb")
   cfg.store(f)
   f.close()

#   cfg.run(10)







