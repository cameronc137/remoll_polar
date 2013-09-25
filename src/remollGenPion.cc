#include "remollBeamTarget.hh"
#include "remollGenPion.hh"

#include "G4String.hh"
#include "CLHEP/Random/RandFlat.h"

#include "wiser_pion.h"
#include "remollEvent.hh"
#include "remollVertex.hh"
#include "G4Material.hh"

#include "remolltypes.hh"

remollGenPion::remollGenPion(){
    fApplyMultScatt = false;

    fTh_min = 20*deg;
    fTh_max = 70*deg;

    
    fE_min = 0.0*deg;
    fE_max = -1e9;

    fPionType = kPiMinus;
}

remollGenPion::~remollGenPion(){
}

void remollGenPion::SamplePhysics(remollVertex *vert, remollEvent *evt){
    // Generate Pion event

//    double beamE   = vert->GetBeamE();
    // Use unradiated beam vertex
    double beamE = remollBeamTarget::GetBeamTarget()->fBeamE;
    double rad_len = vert->GetRadLen();

    double th = acos(CLHEP::RandFlat::shoot(cos(fTh_max), cos(fTh_min)));
    double ph = CLHEP::RandFlat::shoot(fPh_min, fPh_max);

    double true_emax = 0.0;
    if( fE_max < 0.0 || fE_max > beamE ){
	true_emax = beamE;
    } else {
	true_emax = fE_max;
    }
    
    double pf = CLHEP::RandFlat::shoot(fE_min, true_emax);

    assert( pf > 0.0 );
    assert( pf < beamE );

    double V = (fPh_max-fPh_min)*(cos(fTh_min) - cos(fTh_max))*true_emax;

    double intrad = 2.0*alpha*log(beamE/electron_mass_c2)/pi;

    double sigpip = wiser_sigma(beamE/GeV, pf/GeV, th, rad_len*4.0/3.0 + intrad, 0)*nanobarn/GeV;
    double sigpim = wiser_sigma(beamE/GeV, pf/GeV, th, rad_len*4.0/3.0 + intrad, 1)*nanobarn/GeV;

    G4String piontypestr;

    double thisxs = 0.0;

    switch(fPionType){
	case kPiMinus:
	    piontypestr = G4String("pi-");
	    thisxs = vert->GetMaterial()->GetZ()*sigpim + (vert->GetMaterial()->GetA()*mole/g-vert->GetMaterial()->GetZ())*sigpip;
	    break;
	case kPiPlus:
	    piontypestr = G4String("pi+");
	    thisxs = vert->GetMaterial()->GetZ()*sigpip + (vert->GetMaterial()->GetA()*mole/g-vert->GetMaterial()->GetZ())*sigpim;
	    break;
	case kPi0:
	    piontypestr = G4String("pi0");
	    thisxs = vert->GetMaterial()->GetZ()*(sigpip+sigpim)/2.0 + (vert->GetMaterial()->GetA()*mole/g-vert->GetMaterial()->GetZ())*(sigpip+sigpim)/2.0;
	    break;

	default:
	    piontypestr = G4String("oops");
	    break;
    }


    

    evt->SetEffCrossSection(V*thisxs);

    if( vert->GetMaterial()->GetNumberOfElements() != 1 ){
	G4cerr << __FILE__ << " line " << __LINE__ << 
	    ": Error!  Some lazy programmer didn't account for complex materials in the moller process!" << G4endl;
	exit(1);
    }

    evt->SetAsymmetry(0.0);


    evt->ProduceNewParticle( G4ThreeVector(0.0, 0.0, 0.0), 
	    G4ThreeVector(pf*sin(ph)*sin(th), pf*cos(ph)*sin(th), pf*cos(th)), 
	    piontypestr );
    return;

}
