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
    parser = argparse.ArgumentParser(prog='python -m TRT_ConditionsAlgs.TRT_DBStatusUpdate', formatter_class=argparse.RawTextHelpFormatter,
                                     description='''Update TRT conditions: 
    python -m TRT_ConditionsAlgs.TRT_DBStatusUpdate                  -> for testing
    python -m TRT_ConditionsAlgs.TRT_DBStatusUpdate --prod           -> for data production
    python -m TRT_ConditionsAlgs.TRT_DBStatusUpdate --prod --isMC    -> for MC production''')
    
    parser.add_argument('--prod',action='store_true' ,help="Meant for production only")
    parser.add_argument('--isMC',action='store_true' ,help="By default is DATA. If set then MC")
    parser.add_argument('--pwd', required=True ,help="COOL DB password - used by experts only")
    parser.add_argument('--tag', default="" ,help="COOL DB password - used by experts only")
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
        
        if input("Do you want to upload new Status constants as a test tag (enter 1)? ") == "1":
            retag= (args.tag if args.tag else "TrtStrawStatus-WebTest-RUN2-UPD2-00-01")
            print(f"Uploading constants to tag {retag} :") 
            run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --rs=999999 --folder=/TRT/Cond/Status --tag=TextStatus --retag={retag} mycool.db CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")
        else:
            print("Different input provided. It should be 1 to run the actions.")

    elif args.isMC:

        nextstep("Publishing: \033[31;5;7m MC \033[0m upload")
        myupload = input("Uploading MC constants to /TRT/Cond/. Is it for Status (enter 1), is it for StatusHT (2) or is it for StatusPermanent (3)? ")

        if myupload == "1":
            retag = input("Uploading Status. What is the name of target tag (example: TRTCondStatus-MC-run2-run3_00-02)? ")
            run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --folder=/TRT/Cond/Status --tag=TextStatus --retag={retag} mycool.db OFLP200 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")

        elif myupload == "2":
            retag = input("Uploading StatusHT. What is the name of target tag (example: TrtStrawStatusHT-MC-run2-run3-01)? ")
            run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --folder=/TRT/Cond/StatusHT --tag=Textrt --retag={retag} mycool.db OFLP200 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")

        elif myupload == "3":
            retag = input("Uploading StatusPermanent. What is the name of target tag (example: TrtStrawStatusPermCol-02)? ")
            run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --folder=/TRT/Cond/StatusPermanent --tag=TextStatus --retag={retag} mycool.db OFLP200 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")

        else:
            print("Different input provided. It should be 1 (Status), 2 (StatusHT) or 3 (StatusPermanent)")

        print("\n"+"#"*100+"\n")
        print("Note that for a new MC campaign with new TRT gas, you will need to make a new tag eg TrtCalibxx-MC-run2-run3-00-02")
        print("with all the IoVs from TrtCalib-MC-run2-run3-00-01 PLUS a new IoV with the payload of the tag just written")
        print("This way we have the entire MC history in one tag" )
        print("\n"+"#"*100+"\n")

    elif args.prod:

        nextstep("Publishing: \033[31;5;7m PRODUCTION \033[0m upload")

        myupload = input("Uploading data production constants to /TRT/Cond/. Is it for Status (enter 1), is it for StatusHT (2) or is it for StatusPermanent (3)? ")

        if myupload == "1":

            if input("Do you want to upload new Status constants to the production ES1 tag (enter 1)? ") == "1":
                retag= args.tag if args.tag else "TRTCondStatus-RUN2-ES1-UPD1-00-00"
                run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --rs=999999 --folder=/TRT/Cond/Status --tag=TextStatus --retag={retag} mycool.db CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")

            if input("Do you want to upload new Status constants to the production BLK tag (enter 1)? ") == "1":
                retag= args.tag if args.tag else "TRTCondStatus-RUN2-BLK-UPD4-00-00"
                run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --rs=999999 --folder=/TRT/Cond/Status --tag=TextStatus --retag={retag} mycool.db CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")

        elif myupload == "2":
            
            if input("Do you want to upload new StatusHT constants to the production ES1 tag (enter 1)? ") == "1":
                retag= args.tag if args.tag else "TRTCondStatusHT-RUN2-UPD1-00-00"
                run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --rs=999999 --folder=/TRT/Cond/StatusHT --tag=TextStatus --retag={retag} mycool.db CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")

            if input("Do you want to upload new Status constants to the production BLK tag (enter 1)? ") == "1":
                retag= args.tag if args.tag else "TrtStrawStatusHT-RUN2-UPD4-02-00"
                run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --rs=999999 --folder=/TRT/Cond/StatusHT --tag=TextStatus --retag={retag} mycool.db CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")

        elif myupload == "3":
            
            if input("Do you want to upload new StatusPermanent constants to the production BLK tag (enter 1)? ") == "1":
                retag= args.tag if args.tag else "TRTCondStatusPermanent-RUN2-BLK-UPD4-02-00"
                run_command(f"~atlcond/utilsproxy/AtlCoolMerge.py --flask --rs=999999 --folder=/TRT/Cond/StatusPermanent --tag=TextStatus --retag={retag} mycool.db CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_TRT_W {args.pwd}")

        else:
            print("Different input provided. It should be 1 (Status), 2 (StatusHT) or 3 (StatusPermanent)")


    print("Environment setup complete.")