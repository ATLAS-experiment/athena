# TritonTool Example Package

The package reads image data from a file, sent it to a Triton server for inference,
and receives the result. 

The ML model identifies handwritten digits in the form of 28x28 pixels from `0 to 9`.
It prints the likelyhood of the digit being `0 to 9`.

```text
====================================================================================================================================
                                                   Welcome to ApplicationMgr (GaudiCoreSvc v39r4)   
                                          running on login40 on Wed Apr 30 14:13:16 2025            
====================================================================================================================================
ApplicationMgr       INFO Application Manager Configured successfully                                                                                                  
Py:ComponentAccumulator    INFO Athena job with pid 1582485                                                                                                            
CoreDumpSvc                                          INFO Handling signals: 11(Segmentation fault) 7(Bus error) 4(Illegal instruction) 14(Alarm clock) 
AthenaEventLoopMgr                                   INFO Initializing AthenaEventLoopMgr           
AthExTritonExample                                  DEBUG Property update for OutputLevel : new value = 2
AthExTritonExample.EvaluateModelTritonTool          DEBUG Property update for OutputLevel : new value = 2
AthExTritonExample.EvaluateModelTritonTool           INFO Triton client created for model: MNIST_testModel at url: localhost:8001
AthExTritonExample                                   INFO Using pixel file: /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/MLTest/2020-03-31/t10k-images-idx3-ubyt
e                                                                                                                                                                      
AthExTritonExample                                   INFO Total no. of samples: 10000               
AthExTritonExample                                  DEBUG input handles: 0                                                                                             
AthExTritonExample                                  DEBUG output handles: 0                                                                                            
AthExTritonExample                                  DEBUG Adding private ToolHandle tool AthExTritonExample.EvaluateModelTritonTool (AthInfer::TritonTool)
ClassIDSvc                                           INFO getRegistryEntries: read 3537 CLIDRegistry entries for module ALL
ApplicationMgr                                       INFO Application Manager Initialized successfully
ApplicationMgr                                       INFO Application Manager Started successfully  
AthenaEventLoopMgr                                   INFO   ===>>>  start of run 1    <<<===        
AthenaEventLoopMgr                                   INFO   ===>>>  start processing event #1, run #1 0 events processed so far  <<<===
AthExTritonExample                                  DEBUG Label for the input test data:                      
AthExTritonExample                                  DEBUG Score for class 0 = 1.43072e-09 in batch 0
AthExTritonExample                                  DEBUG Score for class 1 = 9.79006e-11 in batch 0
AthExTritonExample                                  DEBUG Score for class 2 = 7.92957e-09 in batch 0
AthExTritonExample                                  DEBUG Score for class 3 = 1.36138e-06 in batch 0          
AthExTritonExample                                  DEBUG Score for class 4 = 8.50513e-12 in batch 0                                  
AthExTritonExample                                  DEBUG Score for class 5 = 6.38869e-11 in batch 0                                   
AthExTritonExample                                  DEBUG Score for class 6 = 2.35857e-13 in batch 0
AthExTritonExample                                  DEBUG Score for class 7 = 0.999997 in batch 0   
AthExTritonExample                                  DEBUG Score for class 8 = 2.93678e-09 in batch 0
AthExTritonExample                                  DEBUG Score for class 9 = 1.15264e-06 in batch 0
AthExTritonExample                                  DEBUG Class: 7 has the highest score: 0.999997 in batch 0 
```