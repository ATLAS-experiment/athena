# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
import subprocess
import os
import sys

def run_command(command):
    """Run a shell command and handle errors."""
    print(f"\n>>> Running: {command}\n")
    try:
        subprocess.run(command, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Error: Command failed: {e}")
        sys.exit(1)

def nextstep(text):
    print("\n"+"#"*100)
    print("#")
    print("#    %s" % (text))
    print("#")
    print("#"*100,"\n")

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(prog='python -m TRT_ConditionsAlgs.TRT_DBUpdate', formatter_class=argparse.RawTextHelpFormatter,
                                     description='''Update TRT conditions: 
    python -m TRT_ConditionsAlgs.TRT_DBUpdate                  -> for testing
    python -m TRT_ConditionsAlgs.TRT_DBUpdate --prod           -> for data production
    python -m TRT_ConditionsAlgs.TRT_DBUpdate --prod --isMC    -> for MC production''')
    
    parser.add_argument('--prod',action='store_true' ,help="Meant for production only")
    parser.add_argument('--isMC',action='store_true' ,help="By default is DATA. If set then MC")
    parser.add_argument('--pwd', required=True ,help="COOL DB password - used by experts only")
    parser.add_argument('-s','--skipToken',action='store_true' ,help="Skips auth-get-user-token if provided (you must have one active)")
    args = parser.parse_args()

    # Export COOL_FLASK environment variable
    os.environ["COOL_FLASK"] = "https://cool-proxy-app.cern.ch"

    # Run auth-get-user-token
    if not args.skipToken:
        run_command("auth-get-user-token -c cool-flask-client -o ~/.client-exchange-token.json")
    else:
        print("Token skipped. Is supposed to be active..")

    if not args.prod and not args.isMC:

        nextstep("Publishing: \033[31;5;7m TEST \033[0m upload")
        
        tagt0="TrtCalibT0-Webtest-RUN2-UPD1-00-00"
        tagrt="TrtCalibRt-Webtest-RUN2-UPD1-00-00"

        if input("Are you all set to upload test T0 and RT constants? (1=yes):") == "1":
            print("Uploading T0s:") 
            run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --rs=999999 --folder=/TRT/Calib/T0 --tag=Textt0 --retag={tagt0} mycool.db CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")
            
            print("Uploading RTs:")
            run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --rs=999999 --folder=/TRT/Calib/RT --tag=Textrt --retag={tagrt} mycool.db CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")
        else:
            print("Skipping... set all the necessary")

    elif args.isMC:

        nextstep("Publishing: \033[31;5;7m MC \033[0m upload")

        if input("Are you all set to upload MC T0 and RT constants (1=yes)? ") == "1":
            tag = input("\nUploading T0s. What is the name of target tag (example: TrtCalibT0-MC-run3-test-00)? ")
            run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --folder=/TRT/Calib/T0 --tag=Textt0 --retag={tag} mycool.db OFLP200 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")
            
            tag = input("\nUploading RTs. What is the name of target tag (example: TrtCalibRT-MC-run3-test-00)? ")
            run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --folder=/TRT/Calib/RT --tag=Textrt --retag={tag} mycool.db OFLP200 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")
            
            print("\n"+"#"*100+"\n")
            print("Note that for a new MC campaign with new TRT gas, you will need to make a new tag eg TrtCalibxx-MC-run2-run3-00-02")
            print("with all the IoVs from TrtCalib-MC-run2-run3-00-01 PLUS a new IoV with the payload of the tag just written")
            print("This way we have the entire MC history in one tag")
            print("\n"+"#"*100+"\n")
        else:
            print("Skipping... set all the necessary")
    
    elif args.prod:

        nextstep("Publishing: \033[31;5;7m PRODUCTION \033[0m upload")

        if input("Do you want to upload new T0s to the production ES1 tag (1=yes)? ") == "1":
            run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --rs=999999 --folder=/TRT/Calib/T0 --tag=Textt0 --retag=TrtCalibT0-RUN2-Physics-UPD1-FieldOn-00-00 mycool.db CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")
        else:
            print("Skipping upload new T0s to the production ES1 tag\n")

        if input("Do you want to upload new RT relations to the production ES1 tag (1=yes)? ") == "1":
            run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --rs=999999 --folder=/TRT/Calib/RT --tag=Textrt --retag=TrtCalibRt-RUN2-Physics-UPD1-FieldOn-00-00 mycool.db CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")
        else:
            print("Skipping upload new RT relations to the production ES1 tag\n")
        
        if input("Do you want to upload new T0s to the production BLK UPD4 tag (1=yes)? ") == "1":
            run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --rs=999999 --folder=/TRT/Calib/T0 --tag=Textt0 --retag=TrtCalibT0-RUN2-Physics-BLK-UPD4-00-03 mycool.db CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")
        else:
            print("Skipping upload new T0s to the production BLK UPD4 tag\n")
        
        if input("Do you want to upload new RT constants to the BLK UPD4 tag (1=yes)? ") == "1":
            run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --rs=999999 --folder=/TRT/Calib/RT --tag=Textrt --retag=TrtCalibRt-RUN2-Physics-BLK-UPD4-00-03 mycool.db CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")
        else:
            print("Skipping upload new RT constants to the BLK UPD4 tag\n")


    print("Environment setup complete.")