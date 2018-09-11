# chmod u+x server.sh
# !/user/bin/env bash

gcc -o server server.c ../Common/common.c ../Common/job.c ../Common/queue.c -laio -pthread

