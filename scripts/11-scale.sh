#!/usr/bin/env bash

. ./common.bash

CWD=$(pwd)
cd_netre

echo "Generating cache files ... might take a while."
for SCALE in 8-12 16-24 24-32 32-48; do
  ./bin/netre experiments/${SCALE}.ini -a long-term
done

mkdir -p scripts/data/scalability
echo "Running experiments."
for SCALE in 8-12 16-24 24-32 32-48; do
  ./bin/netre experiments/${SCALE}.ini -a ltg |& tee scripts/data/scalability/scalability-${SCALE}-ltg.log
  ./bin/netre experiments/${SCALE}.ini -a pug-long |& tee scripts/data/scalability/scalability-${SCALE}-pug-long.log
  ./bin/netre experiments/${SCALE}.ini -a pug-lookback |& tee scripts/data/scalability/scalability-${SCALE}-pug-lookback.log
done
cd ${CWD}
