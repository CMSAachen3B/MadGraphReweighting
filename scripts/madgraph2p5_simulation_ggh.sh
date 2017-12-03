#!/bin/bash

export PATH=$CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/MG5_aMC_v2_6_0/bin:$PATH

if [ ! -d $CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/ggh_2p5 ]
then
	mkdir -p $CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/ggh_2p5
fi
rm -rf $CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/ggh_2p5/*

$CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/MG5_aMC_v2_5_5/bin/mg5_aMC $CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/configs/ggh_2p5.dat

ln -s $CMSSW_RELEASE_BASE/external/$SCRAM_ARCH/lib/* $CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/ggh_2p5/lib/
sed -i -e "s@\(F2PY.*\)\$@\1 --fcompiler=gnu95@g" $CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/ggh_2p5/SubProcesses/makefile

for MAKEFILE in $CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/ggh_2p5/SubProcesses/P*/makefile;
do
	echo -e "\e[92mStart compiling makefile \"$MAKEFILE\"\e[0m"
	cd `dirname $MAKEFILE`
	make matrix2py.so && echo -e "\e[42mSuccessfully compiled makefile \"$MAKEFILE\".\e[0m" || echo -e "\e[41mFailed to compile makefile \"$MAKEFILE\"!\e[0m"
	echo ""
done

