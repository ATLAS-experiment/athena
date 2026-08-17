Sim_tf.py \
    --CA \
    --multithreaded \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-04' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'PyJobTransforms.UseFrontier' 'default:DumpGeo.DumpGeoConfig.DumpGeoCfg' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23eSimulationMultipleIoV' \
    --preExec "print('### Original Flags:');flags.dump('GeoModel');flags.GeoModel.EMECStandard=True;flags.GeoModel.DumpGeo.FilterDetManagers=['BeamPipe','LArMgr'];flags.GeoModel.DumpGeo.ForceOverwrite=True;print('### Modified Flags:');flags.dump('GeoModel')" \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "simttbar.HITS.pool.root" \
    --maxEvents 1 \
    --jobNumber 1 \
    --imf False 

