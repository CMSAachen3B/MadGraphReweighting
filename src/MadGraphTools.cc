#include <algorithm>

#include "CMSAachen3B/MadGraphReweighting/interface/MadGraphTools.h"


MadGraphTools::MadGraphTools(float mixingAngleOverPiHalf, std::string madgraphProcessDirectory, std::string madgraphParamCard, float alphaS,
                             bool madGraphSortingHeavyBQuark) :
	m_madGraphSortingHeavyBQuark(madGraphSortingHeavyBQuark)
{
	// initialise interface to Python
	if (! Py_IsInitialized())
	{
		Py_Initialize();
	}
	PyObject* pyModulePath = PyString_FromString("CMSAachen3B.MadGraphReweighting.reweighting");
	PyObject* pyModule = PyImport_Import(pyModulePath);
	PyObject* pyModuleDict = PyModule_GetDict(pyModule);
	PyObject* pyClass = PyDict_GetItemString(pyModuleDict, "MadGraphTools");
	assert(pyClass != nullptr);
	
	// get instance of Python class MadGraphTools
	PyObject* pyMixingAngleOverPiHalf = PyFloat_FromDouble(mixingAngleOverPiHalf);
	PyObject* pyMadgraphProcessDirectory = PyString_FromString(madgraphProcessDirectory.c_str());
	PyObject* pyMadgraphParamCard = PyString_FromString(madgraphParamCard.c_str());
	PyObject* pyAlphaS = PyFloat_FromDouble(alphaS);
	PyObject* pyArguments = PyTuple_Pack(4, pyMixingAngleOverPiHalf, pyMadgraphProcessDirectory, pyMadgraphParamCard, pyAlphaS);
	m_pyMadGraphTools = PyObject_CallObject(pyClass, pyArguments);
	PyErr_Print();
	assert((m_pyMadGraphTools != nullptr) && PyObject_IsInstance(m_pyMadGraphTools, pyClass));
	
	// clean up; warning: http://stackoverflow.com/a/14678667
	Py_DECREF(pyModulePath);
	//Py_DECREF(pyModule);
	//Py_DECREF(pyModuleDict);
	//Py_DECREF(pyClass);
	Py_DECREF(pyMixingAngleOverPiHalf);
	Py_DECREF(pyMadgraphProcessDirectory);
	Py_DECREF(pyMadgraphParamCard);
	Py_DECREF(pyAlphaS);
	Py_DECREF(pyArguments);
}

MadGraphTools::~MadGraphTools()
{
	//Py_DECREF(m_pyMadGraphTools);
	
	if (Py_IsInitialized())
	{
		Py_Finalize();
	}
}

double MadGraphTools::GetMatrixElementSquared(std::vector<KLHEParticle*>& lheParticles) const
{
	// sorting of LHE particles for MadGraph
	if (m_madGraphSortingHeavyBQuark)
	{
		std::sort(lheParticles.begin(), lheParticles.begin()+2, &MadGraphTools::MadGraphParticleOrderingHeavyBQuark);
		std::sort(lheParticles.begin()+3, lheParticles.end(), &MadGraphTools::MadGraphParticleOrderingHeavyBQuark);
	}
	else
	{
		std::sort(lheParticles.begin(), lheParticles.begin()+2, &MadGraphTools::MadGraphParticleOrderingLightBQuark);
		std::sort(lheParticles.begin()+3, lheParticles.end(), &MadGraphTools::MadGraphParticleOrderingLightBQuark);
	}
	
	std::vector<CartesianRMFLV> particleFourMomenta = MadGraphTools::BoostToHiggsCMS(lheParticles);
	std::vector<int> particlePdgIds = MadGraphTools::GetPdgIds(lheParticles);

	// construct Python list of four-momenta
	PyObject* pyParticleFourMomenta = PyList_New(0);
	for (std::vector<CartesianRMFLV>::const_iterator particleLV = particleFourMomenta.begin(); particleLV != particleFourMomenta.end(); ++particleLV)
	{
		PyObject* pyParticleFourMomentum = PyList_New(0);
		PyList_Append(pyParticleFourMomentum, PyFloat_FromDouble(particleLV->E()));
		PyList_Append(pyParticleFourMomentum, PyFloat_FromDouble(particleLV->Px()));
		PyList_Append(pyParticleFourMomentum, PyFloat_FromDouble(particleLV->Py()));
		PyList_Append(pyParticleFourMomentum, PyFloat_FromDouble(particleLV->Pz()));
		PyList_Append(pyParticleFourMomenta, pyParticleFourMomentum);
	}
	
	//construct list of particle pdgs
	PyObject* pyParticlePdgIds = PyList_New(0);
	for (std::vector<int>::const_iterator particlepdgId = particlePdgIds.begin(); particlepdgId != particlePdgIds.end(); ++particlepdgId)
	{
		PyList_Append(pyParticlePdgIds, PyInt_FromLong(*particlepdgId));
	}
	
	// call MadGraphTools.matrix_element_squared
	PyObject* pyMethodName = PyString_FromString("matrix_element_squared");
	PyObject* pyMatrixElementSquared = PyObject_CallMethodObjArgs(m_pyMadGraphTools, pyMethodName, pyParticleFourMomenta, pyParticlePdgIds, NULL);
	PyErr_Print();
	double matrixElementSquared = -1.0;
	if (pyMatrixElementSquared != nullptr)
	{
		matrixElementSquared = PyFloat_AsDouble(pyMatrixElementSquared);
		Py_DECREF(pyMatrixElementSquared);
	}
	
	// clean up
	Py_DECREF(pyParticleFourMomenta);
	Py_DECREF(pyMethodName);
	
	return matrixElementSquared;
}

// pdgParticle->GetName() has no specific order
// madgraph sorts particle before antiparticle
// puts gluons first
// up type quarks second
// downtype quarks third
// => the order is: g u c d s b u_bar c_bar d_bar s_bar b_bar
bool MadGraphTools::MadGraphParticleOrderingLightBQuark(KLHEParticle* lheParticle1, KLHEParticle* lheParticle2)
{
	int pdgId1 = std::abs(lheParticle1->pdgId);
	int pdgId2 = std::abs(lheParticle2->pdgId);
	
	if ((lheParticle1->pdgId < 0) && (lheParticle2->pdgId > 0))
	{
		return false;
	}
	else if ((lheParticle1->pdgId > 0) && (lheParticle2->pdgId < 0))
	{
		return true;
	}
	else
	{
		if (pdgId1 == 21) // gluon
		{
			return true;
		}
		else if (pdgId2 == 21) // gluon
		{
			return false;
		}
		else if (pdgId1 == 2) // up quark
		{
			return true;
		}
		else if (pdgId2 == 2) // up quark
		{
			return false;
		}
		else if (pdgId1 == 4) // charm quark
		{
			return true;
		}
		else if (pdgId2 == 4) // charm quark
		{
			return false;
		}
		else if (pdgId1 == 1) // down quark
		{
			return true;
		}
		else if (pdgId2 == 1) // down quark
		{
			return false;
		}
		else if (pdgId1 == 3) // strange quark
		{
			return true;
		}
		else if (pdgId2 == 3) // strange quark
		{
			return false;
		}
		else if (pdgId1 == 5) // bottom quark
		{
			return true;
		}
		else if (pdgId2 == 5) // bottom quark
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

// pdgParticle->GetName() has no specific order
// madgraph sorts particle before antiparticle
// puts gluons first
// up type quarks second
// downtype quarks third
// heavy quarks last => the order is: g u c d s u_bar c_bar d_bar s_bar b b_bar
bool MadGraphTools::MadGraphParticleOrderingHeavyBQuark(KLHEParticle* lheParticle1, KLHEParticle* lheParticle2)
{
	int pdgId1 = std::abs(lheParticle1->pdgId);
	int pdgId2 = std::abs(lheParticle2->pdgId);
	
	if ((lheParticle1->pdgId < 0) && (lheParticle2->pdgId > 0) && (pdgId1 != 5) && (pdgId2 == 5)) // bottom quark
	{
		return true;
	}
	else if ((lheParticle1->pdgId > 0) && (lheParticle2->pdgId < 0) && (pdgId1 == 5) && (pdgId2 != 5)) // bottom quark
	{
		return false;
	}
	else
	{
		return MadGraphTools::MadGraphParticleOrderingLightBQuark(lheParticle1, lheParticle2);
	}
}

std::vector<CartesianRMFLV> MadGraphTools::BoostToHiggsCMS(std::vector<KLHEParticle*> lheParticles)
{
	std::vector<CartesianRMFLV> particleFourMomentaHiggsCMS;
	
	//extract 4-momentum of the higgs boson
	CartesianRMFLV higgsFourMomentum = CartesianRMFLV(0,0,0,1);
	for (std::vector<KLHEParticle*>::iterator lheParticle = lheParticles.begin(); lheParticle != lheParticles.end(); ++lheParticle)
	{
		if ((*lheParticle)->pdgId > 20) {
			 higgsFourMomentum = (*lheParticle)->p4;
		}
	}
	
	// Calculate boost to Higgs CMRF and boost particle LV to it. 
	CartesianRMFLV::BetaVector betaVector = higgsFourMomentum.BoostToCM();
	ROOT::Math::Boost boost(betaVector);
	
	// boost all particles
	for (std::vector<KLHEParticle*>::iterator lheParticle = lheParticles.begin(); lheParticle != lheParticles.end(); ++lheParticle)
	{
		particleFourMomentaHiggsCMS.push_back(boost * (*lheParticle)->p4);
	}
	return particleFourMomentaHiggsCMS;
}


std::vector<int> MadGraphTools::GetPdgIds(std::vector<KLHEParticle*> lheParticles)
{
	std::vector<int> particlePdgIds;
	for (std::vector<KLHEParticle*>::iterator lheParticle = lheParticles.begin(); lheParticle != lheParticles.end(); ++lheParticle)
	{
		particlePdgIds.push_back((*lheParticle)->pdgId);
	}
	return particlePdgIds;
}
