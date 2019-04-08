#!/usr/bin/env bash

. ./common.bash

CWD=$(pwd)
cd_netre

for SCALE in 8-12 16-24 24-32 32-48; do
  /usr/bin/time -f "TIME\t${SCALE}\tltg\t%S\t%U" ./bin/netre experiments/time-${SCALE}.ini -a ltg
  /usr/bin/time -f "TIME\t${SCALE}\tpug-long\t%S\t%U" ./bin/netre experiments/time-${SCALE}.ini -a pug-long
  /usr/bin/time -f "TIME\t${SCALE}\tpug-lookback\t%S\t%U" ./bin/netre experiments/time-${SCALE}.ini -a pug-lookback
done |& tee $(CWD)/data/12-scale-time.log
cd ${CWD}
