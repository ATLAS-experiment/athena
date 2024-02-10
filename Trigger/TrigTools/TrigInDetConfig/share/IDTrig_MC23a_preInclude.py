from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
from Campaigns.Utils import Campaign
# do not let use this preInclude with campaigns other than MC23a
if flags.Input.MCCampaign is not Campaign.MC23a:
    raise Exception( "MC23a ID Trigger configuration is called when processing {}. Please remove this preInclude from the config/AMI tag".format(flags.Input.MCCampaign))

from AthenaCommon.SystemOfUnits import GeV; 
flags.Trigger.InDetTracking.tauCore.pTmin = 1*GeV; 
flags.Trigger.InDetTracking.tauIso.pTmin  = 1*GeV; 
flags.Trigger.InDetTracking.jetSuper.zedHalfWidth = 150.;
flags.Trigger.InDetTracking.bmumux.zedHalfWidth = 225.;
flags.Trigger.InDetTracking.bmumux.SuperRoI = False;

flags.Trigger.InDetTracking.RoiZedWidthDefault=0.0
flags.Trigger.InDetTracking.bjet.Xi2max=9.
