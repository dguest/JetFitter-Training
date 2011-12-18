#ifndef DO_NORMALIZATION_H
#define DO_NORMALIZATION_H

double sigmoid(double x); 

double norm_nVTX(int nVTX); 

int back_nVTX(double nVTX);

double norm_nTracksAtVtx(int nTracksAtVtx); 

int back_nTracksAtVtx(double nTracksAtVtx); 

double norm_nSingleTracks(int nSingleTracks); 

int back_nSingleTracks(double nSingleTracks); 

double norm_energyFraction(double energyFraction); 

double back_energyFraction(double energyFraction); 

double norm_mass(double mass); 

double back_mass(double mass); 

double norm_significance3d(double s3d); 

double back_significance3d(double s3d); 

double norm_IP3D(double ip3d); 

double back_IP3D(double ip3d); 

double norm_cat_pT(int cat_pT); 

int back_cat_pT(double cat_pT); 

double norm_cat_eta(double cat_eta); 

int back_cat_eta(int cat_eta); 

#endif // DO_NORMALIZATION_H



