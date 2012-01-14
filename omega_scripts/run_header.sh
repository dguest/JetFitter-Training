#!/bin/bash

echo 'submitted from: ' $PBS_O_WORKDIR 
cd $PBS_O_WORKDIR

export ATLAS_LOCAL_ROOT_BASE=/home/hep/share/app/atlas/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
alias asetup='source $AtlasSetup/scripts/asetup.sh' 

asetup 17.3.0,here