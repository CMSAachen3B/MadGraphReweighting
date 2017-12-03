#!/bin/bash

wget https://launchpad.net/mg5amcnlo/2.0/2.5.x/+download/MG5_aMC_v2.5.5.tar.gz
tar -zxf MG5_aMC_v2.5.5.tar.gz -C $CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/
rm MG5_aMC_v2.5.5.tar.gz

wget feynrules.irmp.ucl.ac.be/raw-attachment/wiki/HiggsCharacterisation/HC_UFO_v4.1.zip
unzip HC_UFO_v4.1.zip -d $CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/MG5_aMC_v2_5_5/models
rm HC_UFO_v4.1.zip

wget feynrules.irmp.ucl.ac.be/raw-attachment/wiki/HiggsCharacterisation/HC_NLO_X0_UFO.zip
unzip HC_NLO_X0_UFO.zip -d $CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/MG5_aMC_v2_5_5/models
rm HC_NLO_X0_UFO.zip

