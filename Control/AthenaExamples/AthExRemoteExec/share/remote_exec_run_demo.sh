(
  python3 -m AthExRemoteExec.RemoteExecDemoHiveLoopConfig --port 50001 --evtMax 10 2>&1 | sed -u 's/^/SERVER   /' &
  python3 -m AthExRemoteExec.RemoteExecDemoClientConfig --target localhost:50001 --evtMax 10 2>&1 | sed -u 's/^/CLIENT   /' &
) | tee remote_exec_run_demo.log | sed -u ''/^SERVER/s//$(printf "\e[31mSERVER\e[0m")/'' | sed -u ''/^CLIENT/s//$(printf "\e[32mCLIENT\e[0m")/''
