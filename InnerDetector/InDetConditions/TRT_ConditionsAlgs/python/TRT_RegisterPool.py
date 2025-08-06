# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
import subprocess
import sys

def run_command(command):
    """Run a shell command and handle errors."""
    print(f"\n>>> Running: {command}\n")
    try:
        subprocess.run(command, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Error: Command failed: {e}")
        sys.exit(1)

def run_command_test(command):
    """Run a shell command and handle errors."""
    print(f"\n>>> Running: {command}\n")


if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(prog='python -m TRT_ConditionsAlgs.TRT_RegisterPool', formatter_class=argparse.RawTextHelpFormatter,
                                     description='''Update TRT conditions: 
    python -m TRT_ConditionsAlgs.TRT_RegisterPool                  -> for testing
    python -m TRT_ConditionsAlgs.TRT_RegisterPool --prod           -> for data production
    python -m TRT_ConditionsAlgs.TRT_RegisterPool --prod --isMC    -> for MC production''')
    
    parser.add_argument('--prod',action='store_true' ,help="Meant for production only")
    parser.add_argument('--isMC',action='store_true' ,help="Set for MC")
    parser.add_argument('--email', default="sergi.rodriguez@cern.ch" ,help="Email to be notify")
    parser.add_argument('--poolfile', default="pooloutputfile.root" ,help="POOL file to be registered")
    args = parser.parse_args()

    # Registering the constants
    # genCOND = "cond09_test.gen.COND" if not args.prod else "cond09_mc.gen.COND" if args.isMC else "condR2_data.gen.COND"
    genCOND = "cond09_mc.gen.COND" if args.isMC else "condR2_data.gen.COND" if args.prod else "cond09_test.gen.COND"
    run_command_test(f"/afs/cern.ch/user/a/atlcond/utils22/registerFiles2 --wait --email={args.email} {genCOND} {args.poolfile}")

    print("Environment setup complete.")