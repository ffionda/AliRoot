/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id$ */

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  Inner Traking System version 11                                         //
//  This class contains the base procedures for the Inner Tracking System   //
//                                                                          //
// Authors: R. Barbera                                                      //
// version 6.                                                               //
// Created  2000.                                                           //
//                                                                          //
//  NOTE: THIS IS THE  SYMMETRIC PPR geometry of the ITS.                   //
// THIS WILL NOT WORK                                                       //
// with the geometry or module classes or any analysis classes. You are     //
// strongly encouraged to uses AliITSv5.                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// See AliITSv11::StepManager().
// General C/C++ includes
#include <stdio.h>
#include <stdlib.h>
// General Root includes
#include <Riostream.h>
#include <TMath.h>
#include <TFile.h>    // only required for Tracking function?
#include <TObjArray.h>
#include <TClonesArray.h>
#include <TLorentzVector.h>
#include <TObjString.h>
// Root Geometry includes
#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoPcon.h>
#include <TGeoCone.h>
#include <TGeoTube.h> // contaings TGeoTubeSeg
#include <TGeoArb8.h>
#include <TGeoCompositeShape.h>
#include <TGeoMatrix.h>
#include <TGeoNode.h>
#include <TGeoMaterial.h>
#include <TGeoMedium.h>
// General AliRoot includes
#include "AliRun.h"
#include "AliMagF.h"
#include "AliConst.h"
// ITS specific includes
#include "AliITShit.h"
#include "AliITSgeom.h"
#include "AliITSgeomSPD.h"
#include "AliITSgeomSDD.h"
#include "AliITSgeomSSD.h"
#include "AliITSDetType.h"
#include "AliITSresponseSPD.h"
#include "AliITSresponseSDD.h"
#include "AliITSresponseSSD.h"
#include "AliITSsegmentationSPD.h"
#include "AliITSsegmentationSDD.h"
#include "AliITSsegmentationSSD.h"
#include "AliITSsimulationSPD.h"
#include "AliITSsimulationSDD.h"
#include "AliITSsimulationSSD.h"
#include "AliITSClusterFinderSPD.h"
#include "AliITSClusterFinderSDD.h"
#include "AliITSClusterFinderSSD.h"
#include "AliITSBaseGeometry.h"
#include "AliITSv11.h"

// Units, Convert from k?? to cm,degree,GeV,seconds,
const Double_t kmm = 0.10; // Convert mm to TGeom's cm.
const Double_t kcm = 1.00; // Convert cv to TGeom's cm.
const Double_t kDegree = 1.0; // Convert degrees to TGeom's degrees
const Double_t kRadian = TMath::DegToRad(); // conver to Radians

#define SQ(A) ((A)*(A))

#define printArb8(A)  if(GetDebug()){\
   cout << A->GetName() << ":"; \
  for(Int_t iii=0;iii<8;iii+=2){ cout <<"("<<A->GetVertices()[iii]<<","     \
                          <<A->GetVertices()[iii+1]<<","<<-A->GetDz()<<")";}\
  for(Int_t iii=8;iii<16;iii+=2){ cout <<"("<<A->GetVertices()[iii]<<","     \
                          <<A->GetVertices()[iii+1]<<","<<A->GetDz()<<")";}\
   cout << endl; }

#define printPcon(A)  if(GetDebug()){\
     cout << A->GetName() << ": N=" << A->GetNz() << " Phi1=" << A->GetPhi1() \
          << ", Dphi=" << A->GetDphi() << endl;                              \
     cout << "i\t   Z   \t  Rmin \t  Rmax" << endl;                          \
     for(Int_t iii=0;iii<A->GetNz();iii++){                                 \
         cout << iii << "\t" << A->GetZ(iii) << "\t" << A->GetRmin(iii)     \
              << "\t" << A->GetRmax(iii) << endl;                           \
     }} // end for iii

#define printTube(A)  if(GetDebug()){\
   cout << A->GetName() <<": Rmin="<<A->GetRmin()\
                          <<" Rmax=" <<A->GetRmax()<<" Dz="<<A->GetDz()<<endl;}

#define printTubeSeg(A)   if(GetDebug()){\
    cout << A->GetName() <<": Phi1="<<A->GetPhi1()<< \
                           " Phi2="<<A->GetPhi2()<<" Rmin="<<A->GetRmin()\
                          <<" Rmax=" <<A->GetRmax()<<" Dz="<<A->GetDz()<<endl;}

#define printConeSeg(A)   if(GetDebug()){\
    cout << A->GetName() <<": Phi1="<<A->GetPhi1()<< \
                         " Phi2="<<A->GetPhi2()<<" Rmin1="<<A->GetRmin1()\
                       <<" Rmax1=" <<A->GetRmax1()<<" Rmin2="<<A->GetRmin2()\
                       <<" Rmax2=" <<A->GetRmax2()<<" Dz="<<A->GetDz()<<endl;}

#define printBBox(A)   if(GetDebug()){\
    cout << A->GetName() <<": Dx="<<A->GetDX()<< \
                           " Dy="<<A->GetDY()<<" Dz="<<A->GetDZ() <<endl;}

ClassImp(AliITSv11)

/*
  Some temparary #define's used untill ROOT has addoppted the proper
  Getter in it's classes.
  These Below are for TGeoPcon functions.
*/

//______________________________________________________________________
AliITSv11::AliITSv11() : AliITS() {
    // Standard default constructor for the ITS version 11.
    // Inputs:
    //   none.
    // Outputs:
    //   none.
    // Return
    //   A default constructed AliITSv11 class.

    //fITSV = 0;
    //fcS = 0;
//   fcD = 0;
}
//______________________________________________________________________
AliITSv11::AliITSv11(const char *title) : AliITS("ITS", title){
    // Standard constructor for the ITS version 11.
    // Inputs:
    //   const char *title  The title of for this geometry.
    // Outputs:
    //   none.
    // Return
    //   A Standard constructed AliITSv11 class.

    //fITSV = 0;
    //fcS = 0;
//    fcD = 0;
}
//______________________________________________________________________
AliITSv11::~AliITSv11() {
    // Standard destructor for the ITS version 11.
    // Inputs:
    //   none.
    // Outputs:
    //   none.
    // Return
    //   none.

//    if(fITSV!=0) delete fITSV;
//    if(fcS!=0) delete fcS;
//    if(fcD!=0) delete fcD;
}
//______________________________________________________________________
AliITSv11::AliITSv11(const AliITSv11 &source) : AliITS(source){
    //     Copy Constructor for ITS version 11.
    // Inputs:
    //   AliITSv11 &source  class to be copied from.
    // Outputs:
    //   none.
    // Return
    //   none.

    if(&source == this) return;
    Error("Copy Constructor","Not allowed to copy AliITSv11");
    return;
}
//______________________________________________________________________
AliITSv11& AliITSv11::operator=(const AliITSv11 &source){
    //    Assignment operator for the ITS version 11.
    // Inputs:
    //   AliITSv11 &source  class to be copied from.
    // Outputs:
    //   none.
    // Return
    //   none.

    if(&source == this) return *this;
    Error("= operator","Not allowed to copy AliITSv11");
    return *this;
}
//______________________________________________________________________
void AliITSv11::BuildGeometry(){
    // This routine defines and Creates the geometry for version 11 of 
    // the ITS for use in the simulation display routines. This is a 
    // very simplified geometry for speed of viewing.
    // Inputs:
    //   none.
    // Outputs:
    //   none.
    // Return
    //   none.
    TVector3 t(0.0,0.0,0.0);

    //if(fITSV==0) fITSV = new AliITSGeometryITSV(this,"ALIC");
    //if(fcS==0) fcS = new AliITSGeometrySSDCone(this,t,"TSV",1);

    //fcS->BuildDisplayGeometry();
}
//______________________________________________________________________
void AliITSv11::CreateGeometry(){
    // This routine defines and Creates the geometry for version 11 of 
    // the ITS. The geometry is used by the particle trasport routines,
    // and therefore, is very detailed.
    // Inputs:
    //   none.
    // Outputs:
    //   none.
    // Return
    //   none.
    TVector3 t(0.0,0.0,0.0);

    TGeoManager *mgr = gGeoManager;
    TGeoVolume *ALIC = mgr->GetTopVolume();

    TGeoPcon *itsv = new TGeoPcon("ITS Top Volume, Daughter of ALIC",
                                  0.0,360.0,2);
    // DefineSection(section number, Z, Rmin, Rmax).
    itsv->DefineSection(0,-300.0*kcm,0.01*kcm,50.0*kcm);
    itsv->DefineSection(1,+300.0*kcm,0.01*kcm,50.0*kcm);
    TGeoVolume *ITSV = new TGeoVolume("ITSV",itsv,0);
    //mgr->AddVolume(ITSV);
    ITSV->SetVisibility(kFALSE);
    ALIC->AddNode(ITSV,1,0);
    //
    SPDCone(ITSV);
    SDDCone(ITSV);
    SSDCone(ITSV);
    ServicesCableSupport(ITSV);
}
//______________________________________________________________________
Double_t AliITSv11::RmaxFrom2Points(TGeoPcon *p,Int_t i1,Int_t i2,Double_t z){
    // functions Require at parts of Volume A to be already defined.
    // Retruns the value of Rmax corresponding to point z alone the line
    // defined by the two points p.Rmax(i1),p-GetZ(i1) and p->GetRmax(i2),
    // p->GetZ(i2).

    return p->GetRmax(i2)+(p->GetRmax(i1)-p->GetRmax(i2))*(z-p->GetZ(i2))/
     (p->GetZ(i1)-p->GetZ(i2));
}
//______________________________________________________________________
Double_t AliITSv11::RminFrom2Points(TGeoPcon *p,Int_t i1,Int_t i2,Double_t z){
    // Retruns the value of Rmin corresponding to point z alone the line
    // defined by the two points p->GetRmin(i1),p->GetZ(i1) and 
    // p->GetRmin(i2),  p->GetZ(i2).

    return p->GetRmin(i2)+(p->GetRmin(i1)-p->GetRmin(i2))*(z-p->GetZ(i2))/
     (p->GetZ(i1)-p->GetZ(i2));
}
//______________________________________________________________________
Double_t AliITSv11::RFrom2Points(Double_t *p,Double_t *Z,Int_t i1,
                                 Int_t i2,Double_t z){
    // Retruns the value of Rmin corresponding to point z alone the line
    // defined by the two points p->GetRmin(i1),p->GetZ(i1) and 
    // p->GetRmin(i2), p->GetZ(i2).

    return p[i2]+(p[i1]-p[i2])*(z-Z[i2])/(Z[i1]-Z[i2]);
}
//______________________________________________________________________
Double_t AliITSv11::Zfrom2MinPoints(TGeoPcon *p,Int_t i1,Int_t i2,Double_t r){
    // Retruns the value of Z corresponding to point R alone the line
    // defined by the two points p->GetRmin(i1),p->GetZ(i1) and 
    // p->GetRmin(i2),p->GetZ(i2)

    return p->GetZ(i2)+(p->GetZ(i1)-p->GetZ(i2))*(r-p->GetRmin(i2))/
     (p->GetRmin(i1)-p->GetRmin(i2));
}
//______________________________________________________________________
Double_t AliITSv11::Zfrom2MaxPoints(TGeoPcon *p,Int_t i1,Int_t i2,Double_t r){
    // Retruns the value of Z corresponding to point R alone the line
    // defined by the two points p->GetRmax(i1),p->GetZ(i1) and 
    // p->GetRmax(i2),p->GetZ(i2)

    return p->GetZ(i2)+(p->GetZ(i1)-p->GetZ(i2))*(r-p->GetRmax(i2))/
     (p->GetRmax(i1)-p->GetRmax(i2));
}
//______________________________________________________________________
Double_t AliITSv11::Zfrom2Points(Double_t *Z,Double_t *p,Int_t i1,
                                 Int_t i2,Double_t r){
    // Retruns the value of Z corresponding to point R alone the line
    // defined by the two points p->GetRmax(i1),p->GetZ(i1) and 
    // p->GetRmax(i2),p->GetZ(i2)

    return Z[i2]+(Z[i1]-Z[i2])*(r-p[i2])/(p[i1]-p[i2]);
}
//______________________________________________________________________
Double_t AliITSv11::RmaxFromZpCone(TGeoPcon *p,Double_t tc,Double_t z,
                                   Double_t th){
    // General SSD Outer Cone surface equation Rmax.
    Double_t tantc = TMath::Tan(tc*TMath::DegToRad());
    Double_t costc = TMath::Cos(tc*TMath::DegToRad());

    return -tantc*(z-p->GetZ(4))+p->GetRmax(4)+th/costc;
}
//______________________________________________________________________
Double_t AliITSv11::RmaxFromZpCone(Double_t *GetRmax,Double_t *GetZ,
                                   Double_t tc,Double_t z,Double_t th){
    // General SSD Outer Cone surface equation Rmax.
    Double_t tantc = TMath::Tan(tc*TMath::DegToRad());
    Double_t costc = TMath::Cos(tc*TMath::DegToRad());

    return -tantc*(z-GetZ[4])+GetRmax[4]+th/costc;
}
//______________________________________________________________________
Double_t AliITSv11::RminFromZpCone(TGeoPcon *p,Double_t tc,Double_t z,
                                   Double_t th){
    // General SSD Inner Cone surface equation Rmin.
    Double_t tantc = TMath::Tan(tc*TMath::DegToRad());
    Double_t costc = TMath::Cos(tc*TMath::DegToRad());

    return -tantc*(z-p->GetZ(3))+p->GetRmin(3)+th/costc;
}
//______________________________________________________________________
Double_t AliITSv11::RminFromZpCone(Double_t *GetRmin,Double_t *GetZ,
                                   Double_t tc,Double_t z,Double_t th){
    // General SSD Inner Cone surface equation Rmin.
    Double_t tantc = TMath::Tan(tc*TMath::DegToRad());
    Double_t costc = TMath::Cos(tc*TMath::DegToRad());

    return -tantc*(z-GetZ[3])+GetRmin[3]+th/costc;
}
//______________________________________________________________________
Double_t AliITSv11::ZFromRmaxpCone(TGeoPcon *p,Double_t tc,Double_t r,
                                   Double_t th){
    // General SSD Outer cone Surface equation for z.
    Double_t tantc = TMath::Tan(tc*TMath::DegToRad());
    Double_t costc = TMath::Cos(tc*TMath::DegToRad());

    return p->GetZ(4)+(p->GetRmax(4)+th/costc-r)/tantc;
}
//______________________________________________________________________
Double_t AliITSv11::ZFromRmaxpCone(Double_t *GetRmax,Double_t *GetZ,
                                   Double_t tc,Double_t r,Double_t th){
    // General SSD Outer cone Surface equation for z.
    Double_t tantc = TMath::Tan(tc*TMath::DegToRad());
    Double_t costc = TMath::Cos(tc*TMath::DegToRad());

    return GetZ[4]+(GetRmax[4]+th/costc-r)/tantc;
}
//______________________________________________________________________
Double_t AliITSv11::ZFromRminpCone(TGeoPcon *p,Double_t tc,Double_t r,
                                   Double_t th){
    // General SSD Inner cone Surface equation for z.
    Double_t tantc = TMath::Tan(tc*TMath::DegToRad());
    Double_t costc = TMath::Cos(tc*TMath::DegToRad());

    return p->GetZ(3)+(p->GetRmin(3)+th/costc-r)/tantc;
}
//______________________________________________________________________
void AliITSv11::RadiusOfCurvature(Double_t rc,Double_t theta0,Double_t z0,
                 Double_t r0,Double_t theta1,Double_t &z1,
                 Double_t &r1){
    // Given a initial point z0,r0, the initial angle theta0, and the radius
    // of curvature, returns the point z1, r1 at the angle theta1. Theta
    // measured from the r axis in the clock wise direction [degrees].
    Double_t sin0 = TMath::Sin(theta0*TMath::DegToRad());
    Double_t cos0 = TMath::Cos(theta0*TMath::DegToRad());
    Double_t sin1 = TMath::Sin(theta1*TMath::DegToRad());
    Double_t cos1 = TMath::Cos(theta1*TMath::DegToRad());

    z1 = rc*(sin1-sin0)+z0;
    r1 = rc*(cos1-cos0)+r0;
    return;
}
//______________________________________________________________________
void AliITSv11::SPDCone(TGeoVolume *Moth){
    // Define the detail SPD support cone geometry.
    // Inputs:
    //   none.
    // Outputs:
    //  none.
    // Return:
    //  none.

    SPDThermalSheald(Moth);
}
//______________________________________________________________________
void AliITSv11::SPDThermalSheald(TGeoVolume *Moth){
    // Define the detail SPD Thermal Sheld geometry.
    // Inputs:
    //   none.
    // Outputs:
    //  none.
    // Return:
    //  none.
    // From ALICE-Thermal Screen (SPD) "Cylinder" file thermal-screen2_a3.ps
    // Volumes A1,A2,A2,Ah1,Ah2,Ah3, and B1,B2,B3,Bh1,Bh2,Bh3;
    // "CONE TRANSITION" file thermal-screen1_a3.ps Volumes C1,C2,C3,Ch1,Ch2,
    // Ch3; "FLANGE" file thermal-screen4_a3.ps Volumes D,Ds,Dw,Dws; and 
    // "HALF ASSEMBLY" file thermal-screen3_a3.ps. This object, both halfs,
    // are incased inside of a single minimum sized mother volume called M,
    // which is a union of two parts M1 and 4 copies of M2.
    const Double_t TSCarbonFiberThA = 0.03*kmm; // 
    //const Double_t TSCarbonFiberThB = 0.10*kmm; //
    const Double_t TSCLengthB  = 50.0*kmm; //
    const Double_t TSCLengthA  = 900.0*kmm-2.0*TSCLengthB; //
    const Double_t TSCLengthC  = 290.0*kmm; //
    const Double_t TSCLengthD  = 15.0*kmm; //
    const Double_t TSCAngle    = 36.0*kDegree;//Rep. angle of cent. accordin
    const Double_t TSCRoutA    = 99.255*kmm; // Outer radii
    const Double_t TSCRinA     = 81.475*kmm; // Iner radii
    const Double_t TSCRoutB    = 99.955*kmm; // Outer radii
    const Double_t TSCRinB     = 80.775*kmm; // Iner radii
    const Double_t TSCRoutCp   = 390.0*kmm;  // Outer radii
    const Double_t TSCRinCp    = 373.0*kmm;  // Iner radii
    Double_t TSCRoutC,TSCRinC; // values need to be calculated
    const Double_t TSCRwingD   = 492.5*kmm;  // Outer radii
    const Double_t TSCRoutD    = 0.5*840.*kmm;// Outer radii
    const Double_t TSCRinD     = 373.0*kmm;  // Iner radii
    const Double_t TSCAngleDD  = 60.*kmm/TSCRwingD/kRadian;//angular wing width
    //angular wing width of fill material
    const Double_t TSCAngleDDs = (60.*kmm-2.*TSCarbonFiberThA)/TSCRwingD/kRadian;
    const Double_t TSCAngleD0  = 45.*kDegree;//Strting angle of wing
    const Double_t TSCoutSA    = 24.372*kmm; // The other one Calculated
    const Double_t TSCinLA     = 31.674*kmm; // The ohter one Calculated
    const Double_t TSCoutSB    = 24.596*kmm; // The other one Calculated
    const Double_t TSCinLB     = 31.453*kmm; // The ohter one Calculated
    const Double_t TSCoutSC    = 148.831*kmm;// The other one Calculated
    const Double_t TSCinLC     = 90.915*kmm; // The ohter one Calculated
    Int_t i,k;
    Double_t th;
    Double_t xo[7],yo[7],xi[7],yi[7];
    Double_t xbo[7],ybo[7],xbi[7],ybi[7];
    Double_t xco[7],yco[7],xci[7],yci[7];
    TGeoArb8 *A1,*A2,*A3,*Ah1,*Ah2,*Ah3,*B1,*B2,*B3,*Bh1,*Bh2,*Bh3;
    TGeoArb8 *C1,*C2,*C3,*Ch1,*Ch2,*Ch3;
    TGeoTube  *D,*Ds;
    TGeoTubeSeg *Dw,*Dws,*M2;
    TGeoPcon *M1;
    TGeoCompositeShape *M;
    TGeoRotation *rot;
    TGeoTranslation *tranb,*tranbm,*tranc;
    TGeoTranslation *tranITSspdShealdVVt0;
    TGeoCombiTrans *rotITSspdShealdVVt1,*rotITSspdShealdVVt2;
    TGeoCombiTrans *rotITSspdShealdVVt3;
    TGeoMedium *SPDcf  = 0; // SPD support cone Carbon Fiber materal number.
    TGeoMedium *SPDfs  = 0; // SPD support cone inserto stesalite 4411w.
    TGeoMedium *SPDfo  = 0; // SPD support cone foam, Rohacell 50A.
    TGeoMedium *SPDss  = 0; // SPD support cone screw material,Stainless steal
    TGeoMedium *SPDair = 0; // SPD support cone Air
    //TGeoMedium *SPDal  = 0; // SPD support cone SDD mounting bracket Al

    TSCRoutC = TMath::Sqrt(TSCRoutCp*TSCRoutCp-0.25*TSCoutSC*TSCoutSC);
    TSCRinC  = TMath::Sqrt(TSCRinCp *TSCRinCp -0.25*TSCinLC *TSCinLC );
    A1  = new TGeoArb8("ITS SPD Therm Screen Clyinder A1",0.5*TSCLengthA);
    A2  = new TGeoArb8("ITS SPD Therm Screen Clyinder A2",0.5*TSCLengthA);
    A3  = new TGeoArb8("ITS SPD Therm Screen Clyinder A3",0.5*TSCLengthA);
    Ah1 = new TGeoArb8("ITS SPD Therm Screen Cylinder Ah1",0.5*TSCLengthA);
    Ah2 = new TGeoArb8("ITS SPD Therm Screen Cylinder Ah2",0.5*TSCLengthA);
    Ah3 = new TGeoArb8("ITS SPD Therm Screen Cylinder Ah3",0.5*TSCLengthA);
    B1  = new TGeoArb8("ITS SPD Therm Screen Clyinder B1",0.5*TSCLengthB);
    B2  = new TGeoArb8("ITS SPD Therm Screen Clyinder B2",0.5*TSCLengthB);
    B3  = new TGeoArb8("ITS SPD Therm Screen Clyinder B3",0.5*TSCLengthB);
    Bh1 = new TGeoArb8("ITS SPD Therm Screen Cylinder Bh1",0.5*TSCLengthB);
    Bh2 = new TGeoArb8("ITS SPD Therm Screen Cylinder Bh2",0.5*TSCLengthB);
    Bh3 = new TGeoArb8("ITS SPD Therm Screen Cylinder Bh3",0.5*TSCLengthB);
    C1  = new TGeoArb8("ITS SPD Therm Screen Clyinder C1",0.5*TSCLengthC);
    C2  = new TGeoArb8("ITS SPD Therm Screen Clyinder C2",0.5*TSCLengthC);
    C3  = new TGeoArb8("ITS SPD Therm Screen Clyinder C3",0.5*TSCLengthC);
    Ch1 = new TGeoArb8("ITS SPD Therm Screen Cylinder Ch1",0.5*TSCLengthC);
    Ch2 = new TGeoArb8("ITS SPD Therm Screen Cylinder Ch2",0.5*TSCLengthC);
    Ch3 = new TGeoArb8("ITS SPD Therm Screen Cylinder Ch3",0.5*TSCLengthC);
    D = new TGeoTube("ITS SPD Therm Screen Flange D",TSCRinD,TSCRoutD,
                    0.5*TSCLengthD);
    Ds = new TGeoTube("ITS SPD Therm Screen Flange fill Ds",
                      TSCRinD+TSCarbonFiberThA,TSCRoutD-TSCarbonFiberThA,
                      0.5*TSCLengthD);
    printTube(D);
    printTube(Ds);
    Dw = new TGeoTubeSeg("ITS SPD Therm Screen Flange Wing Dw",
                         TSCRoutD,TSCRwingD ,0.5*TSCLengthD,
                         TSCAngleD0-0.5*TSCAngleDD,TSCAngleD0+0.5*TSCAngleDD);
    Dws = new TGeoTubeSeg("ITS SPD Therm Screen Flange Wing Fill Ds",
                          TSCRoutD,TSCRwingD-TSCarbonFiberThA,
                          0.5*TSCLengthD,TSCAngleD0-0.5*TSCAngleDDs,
                          TSCAngleD0+0.5*TSCAngleDDs);
    printTubeSeg(Dw);
    printTubeSeg(Dws);
    k = 0;
    for(i=-1;i<2;i++){
      th = ((Double_t)(i+1))*TSCAngle*kRadian;
      xo[k] = TSCRoutA*TMath::Sin(th) - 0.5*TSCoutSA*TMath::Cos(th);
      yo[k] = TSCRoutA*TMath::Cos(th) + 0.5*TSCoutSA*TMath::Sin(th);
      xi[k] = TSCRinA *TMath::Sin(th) - 0.5*TSCinLA *TMath::Cos(th);
      yi[k] = TSCRinA *TMath::Cos(th) + 0.5*TSCinLA *TMath::Sin(th);
      xbo[k] = TSCRoutB*TMath::Sin(th) - 0.5*TSCoutSB*TMath::Cos(th);
      ybo[k] = TSCRoutB*TMath::Cos(th) + 0.5*TSCoutSB*TMath::Sin(th);
      xbi[k] = TSCRinB *TMath::Sin(th) - 0.5*TSCinLB *TMath::Cos(th);
      ybi[k] = TSCRinB *TMath::Cos(th) + 0.5*TSCinLB *TMath::Sin(th);
      xco[k] = TSCRoutC*TMath::Sin(th) - 0.5*TSCoutSC*TMath::Cos(th);
      yco[k] = TSCRoutC*TMath::Cos(th) + 0.5*TSCoutSC*TMath::Sin(th);
      xci[k] = TSCRinC *TMath::Sin(th) - 0.5*TSCinLC *TMath::Cos(th);
      yci[k] = TSCRinC *TMath::Cos(th) + 0.5*TSCinLC *TMath::Sin(th);
      k++;
      xo[k] = TSCRoutA*TMath::Sin(th) + 0.5*TSCoutSA*TMath::Cos(th);
      yo[k] = TSCRoutA*TMath::Cos(th) - 0.5*TSCoutSA*TMath::Sin(th);
      xi[k] = TSCRinA *TMath::Sin(th) + 0.5*TSCinLA *TMath::Cos(th);
      yi[k] = TSCRinA *TMath::Cos(th) - 0.5*TSCinLA *TMath::Sin(th);
      xbo[k] = TSCRoutB*TMath::Sin(th) + 0.5*TSCoutSB*TMath::Cos(th);
      ybo[k] = TSCRoutB*TMath::Cos(th) - 0.5*TSCoutSB*TMath::Sin(th);
      xbi[k] = TSCRinB *TMath::Sin(th) + 0.5*TSCinLB *TMath::Cos(th);
      ybi[k] = TSCRinB *TMath::Cos(th) - 0.5*TSCinLB *TMath::Sin(th);
      xco[k] = TSCRoutC*TMath::Sin(th) + 0.5*TSCoutSC*TMath::Cos(th);
      yco[k] = TSCRoutC*TMath::Cos(th) - 0.5*TSCoutSC*TMath::Sin(th);
      xci[k] = TSCRinC *TMath::Sin(th) + 0.5*TSCinLC *TMath::Cos(th);
      yci[k] = TSCRinC *TMath::Cos(th) - 0.5*TSCinLC *TMath::Sin(th);
      k++;
    } // end for i
    xo[6] = xo[5];
    yo[6] = 0.0;
    xi[6] = xi[5];
    yi[6] = 0.0;
    xbo[6] = xbo[5];
    ybo[6] = 0.0;
    xbi[6] = xbi[5];
    ybi[6] = 0.0;
    xco[6] = xco[5];
    yco[6] = 0.0;
    xci[6] = xci[5];
    yci[6] = 0.0;
    if(GetDebug()){
    cout.precision(4);
    cout.width(7);
    cout <<"i     \t  xo  yo    \t  xi yi     \t  xbo ybo   \t   xbi ybi  "
        "\t   xco yco   \t   xci yxi"<<endl;
    for(i=0;i<7;i++){
        cout << i <<"\t"<<xo[i]<<","<<yo[i];
        cout      <<"\t"<<xi[i]<<","<<yi[i];
        cout      <<"\t"<<xbo[i]<<","<<ybo[i];
        cout      <<"\t"<<xbi[i]<<","<<ybi[i];
        cout      <<"\t"<<xco[i]<<","<<yco[i];
        cout      <<"\t"<<xci[i]<<","<<yci[i];
        cout<<endl;}
    } // end if GetDebug()
    //+++++++++++++++++++++++++
    A1->SetVertex(0,xo[0],yo[0]);
    A1->SetVertex(1,xo[1],yo[1]);
    A1->SetVertex(2,xi[1],yi[1]);
    A1->SetVertex(3,xi[0],yi[0]);
    //
    A2->SetVertex(0,xo[1],yo[1]);
    A2->SetVertex(1,xo[2],yo[2]);
    A2->SetVertex(2,xi[2],yi[2]);
    A2->SetVertex(3,xi[1],yi[1]);
    //
    A3->SetVertex(0,xo[5],yo[5]);
    A3->SetVertex(1,xo[6],yo[6]);
    A3->SetVertex(2,xi[6],yi[6]);
    A3->SetVertex(3,xi[5],yi[5]);
    //--------------------------
    B1->SetVertex(0,xbo[0],ybo[0]);
    B1->SetVertex(1,xbo[1],ybo[1]);
    B1->SetVertex(2,xbi[1],ybi[1]);
    B1->SetVertex(3,xbi[0],ybi[0]);
    //
    B2->SetVertex(0,xbo[1],ybo[1]);
    B2->SetVertex(1,xbo[2],ybo[2]);
    B2->SetVertex(2,xbi[2],ybi[2]);
    B2->SetVertex(3,xbi[1],ybi[1]);
    //
    B3->SetVertex(0,xbo[5],ybo[5]);
    B3->SetVertex(1,xbo[6],ybo[6]);
    B3->SetVertex(2,xbi[6],ybi[6]);
    B3->SetVertex(3,xbi[5],ybi[5]);
    //--------------------------
    C1->SetVertex(0,xco[0],yco[0]);
    C1->SetVertex(1,xco[1],yco[1]);
    C1->SetVertex(2,xci[1],yci[1]);
    C1->SetVertex(3,xci[0],yci[0]);
    //
    C2->SetVertex(0,xco[1],yco[1]);
    C2->SetVertex(1,xco[2],yco[2]);
    C2->SetVertex(2,xci[2],yci[2]);
    C2->SetVertex(3,xci[1],yci[1]);
    //
    C3->SetVertex(0,xco[5],yco[5]);
    C3->SetVertex(1,xco[6],yco[6]);
    C3->SetVertex(2,xci[6],yci[6]);
    C3->SetVertex(3,xci[5],yci[5]);
    // Defining the hole, filled with air
    Double_t p1,c1,x,y,x7[3],y7[3];
    p1 = (xo[0]-xi[0])/(yo[0]-yi[0]);
    c1 = xo[0]+0.5*TSCarbonFiberThA*TMath::Sqrt(SQ(xo[0]-xi[0])+
                                                SQ(yo[0]-yi[0]))/(xo[0]-xi[0]);
    y = TSCRoutA-2.*TSCarbonFiberThA;
    x = p1*(y-yo[0])+c1;
    Ah1->SetVertex(0,x,y);
    Bh1->SetVertex(0,x,y);
    Ch1->SetVertex(4,x,y);
    y = TSCRinA+TSCarbonFiberThA;
    x = p1*(y-yo[0])+c1;
    Ah1->SetVertex(3,x,y);
    Bh1->SetVertex(3,x,y);
    x7[0] = x; y7[0] = y; // vortexing done after last point
    //Ch1->SetVertex(7,x,y);
    p1 = (xo[1]-xi[1])/(yo[1]-yi[1]);
    c1 = xo[1]-0.5*TSCarbonFiberThA*TMath::Sqrt(SQ(xo[1]-xi[1])+
                                                SQ(yo[1]-yi[1]))/(xo[1]-xi[1]);
    y = TSCRoutA-2.*TSCarbonFiberThA;
    x = p1*(y-yo[1])+c1;
    Ah1->SetVertex(1,x,y);
    Bh1->SetVertex(1,x,y);
    Ch1->SetVertex(5,x,y);
    y = TSCRinA+TSCarbonFiberThA;
    x = p1*(y-yo[1])+c1;
    Ah1->SetVertex(2,x,y);
    Bh1->SetVertex(2,x,y);
    Ch1->SetVertex(6,x,y);
    //
    // The easist way to get the points for the hole in volume A2 is to
    // rotate it to the Y axis where the y coordinates are easier to know
    // and then rotate it back.
    Double_t xp,yp,xa,ya,xb,yb;
    th = 0.5*TSCAngle*kRadian;
    xa = TMath::Cos(th)*xo[1]-TMath::Sin(th)*yo[1];
    ya = TMath::Sin(th)*xo[1]+TMath::Cos(th)*yo[1];
    xb = TMath::Cos(th)*xi[1]-TMath::Sin(th)*yi[1];
    yb = TMath::Sin(th)*xi[1]+TMath::Cos(th)*yi[1];
    p1 = (xa-xb)/(ya-yb);
    c1 = xa+0.5*TSCarbonFiberThA*TMath::Sqrt(SQ(xa-xb)+SQ(ya-yb))/(xa-xb);
    y = ya-TSCarbonFiberThA;
    x = p1*(y-ya)+c1;
    xp = TMath::Cos(-th)*x-TMath::Sin(-th)*y;
    yp = TMath::Sin(-th)*x+TMath::Cos(-th)*y;
    Ah2->SetVertex(0,xp,yp);
    Bh2->SetVertex(0,xp,yp);
    Ch2->SetVertex(4,xp,yp);
    y = yb+2.0*TSCarbonFiberThA;
    x = p1*(y-ya)+c1;
    xp = TMath::Cos(-th)*x-TMath::Sin(-th)*y;
    yp = TMath::Sin(-th)*x+TMath::Cos(-th)*y;
    Ah2->SetVertex(3,xp,yp);
    Bh2->SetVertex(3,xp,yp);
    x7[1] = x; y7[1] = y; // vortexing done after last point
    //Ch2->SetVertex(7,xp,yp);
    xa = TMath::Cos(th)*xo[2]-TMath::Sin(th)*yo[2];
    ya = TMath::Sin(th)*xo[2]+TMath::Cos(th)*yo[2];
    xb = TMath::Cos(th)*xi[2]-TMath::Sin(th)*yi[2];
    yb = TMath::Sin(th)*xi[2]+TMath::Cos(th)*yi[2];
    p1 = (xa-xb)/(ya-yb);
    c1 = xa-0.5*TSCarbonFiberThA*TMath::Sqrt(SQ(xa-xb)+SQ(ya-yb))/(xa-xb);
    y = ya-TSCarbonFiberThA;
    x = p1*(y-ya)+c1;
    xp = TMath::Cos(-th)*x-TMath::Sin(-th)*y;
    yp = TMath::Sin(-th)*x+TMath::Cos(-th)*y;
    Ah2->SetVertex(1,xp,yp);
    Bh2->SetVertex(1,xp,yp);
    Ch2->SetVertex(5,xp,yp);
    y = yb+2.0*TSCarbonFiberThA;
    x = p1*(y-ya)+c1;
    xp = TMath::Cos(-th)*x-TMath::Sin(-th)*y;
    yp = TMath::Sin(-th)*x+TMath::Cos(-th)*y;
    Ah2->SetVertex(2,xp,yp);
    Bh2->SetVertex(2,xp,yp);
    Ch2->SetVertex(6,xp,yp);
    //
    p1 = (yo[5]-yi[5])/(xo[5]-xi[5]);
    c1 = yo[5]+0.5*TSCarbonFiberThA*TMath::Sqrt(SQ(yo[5]-yi[5])+
                                                SQ(xo[5]-xi[5]))/(yo[5]-yi[5]);
    x = xo[5]-TSCarbonFiberThA;
    y = p1*(x-xo[5])+c1;
    Ah3->SetVertex(0,x,y);
    Bh3->SetVertex(0,x,y);
    Ch3->SetVertex(4,x,y);
    x = xi[5]+2.0*TSCarbonFiberThA;
    y = p1*(x-xo[5])+c1;
    Ah3->SetVertex(3,x,y);
    Bh3->SetVertex(3,x,y);
    x7[2] = x; y7[2] = y; // vortexing done after last point
    //Ch3->SetVertex(7,x,y);
    y = 2.0*TSCarbonFiberThA;
    x = xo[5]-TSCarbonFiberThA;
    Ah3->SetVertex(1,x,y);
    Bh3->SetVertex(1,x,y);
    Ch3->SetVertex(5,x,y);
    y = 2.0*TSCarbonFiberThA;
    x = xi[5]+2.0*TSCarbonFiberThA;
    Ah3->SetVertex(2,x,y);
    Bh3->SetVertex(2,x,y);
    Ch3->SetVertex(6,x,y);
    //
    for(i=0;i<4;i++){ // define points at +dz
      A1->SetVertex(i+4,(A1->GetVertices())[2*i],(A1->GetVertices())[1+2*i]);
      A2->SetVertex(i+4,(A2->GetVertices())[2*i],(A2->GetVertices())[1+2*i]);
      A3->SetVertex(i+4,(A3->GetVertices())[2*i],(A3->GetVertices())[1+2*i]);
      //
      B1->SetVertex(i+4,(B1->GetVertices())[2*i],(B1->GetVertices())[1+2*i]);
      B2->SetVertex(i+4,(B2->GetVertices())[2*i],(B2->GetVertices())[1+2*i]);
      B3->SetVertex(i+4,(B3->GetVertices())[2*i],(B3->GetVertices())[1+2*i]);
      // C's are a cone which must match up with B's.
      C1->SetVertex(i+4,(B1->GetVertices())[2*i],(B1->GetVertices())[1+2*i]);
      C2->SetVertex(i+4,(B2->GetVertices())[2*i],(B2->GetVertices())[1+2*i]);
      C3->SetVertex(i+4,(B3->GetVertices())[2*i],(B3->GetVertices())[1+2*i]);
      //
      Ah1->SetVertex(i+4,(Ah1->GetVertices())[2*i],
                         (Ah1->GetVertices())[1+2*i]);
      Ah2->SetVertex(i+4,(Ah2->GetVertices())[2*i],
                         (Ah2->GetVertices())[1+2*i]);
      Ah3->SetVertex(i+4,(Ah3->GetVertices())[2*i],
                         (Ah3->GetVertices())[1+2*i]);
      //
      Bh1->SetVertex(i+4,(Bh1->GetVertices())[2*i],
                         (Bh1->GetVertices())[1+2*i]);
      Bh2->SetVertex(i+4,(Bh2->GetVertices())[2*i],
                         (Bh2->GetVertices())[1+2*i]);
      Bh3->SetVertex(i+4,(Bh3->GetVertices())[2*i],
                         (Bh3->GetVertices())[1+2*i]);
    } // end for
    //
    p1 = (xco[0]-xci[0])/(yco[0]-yci[0]);
    c1 = xco[0]+0.5*TSCarbonFiberThA*TMath::Sqrt(SQ(xco[0]-xci[0])+
                                           SQ(yco[0]-yci[0]))/(xco[0]-xci[0]);
    y = TSCRoutC-2.*TSCarbonFiberThA;
    x = p1*(y-yco[0])+c1;
    Ch1->SetVertex(0,x,y);
    y = TSCRinC+TSCarbonFiberThA;
    x = p1*(y-yci[0])+c1;
    Ch1->SetVertex(2,x,y);
    p1 = (xco[1]-xci[1])/(yco[1]-yci[1]);
    c1 = xco[1]-0.5*TSCarbonFiberThA*TMath::Sqrt(SQ(xco[1]-xci[1])+
                                           SQ(yco[1]-yci[1]))/(xco[1]-xci[1]);
    y = TSCRoutC-2.*TSCarbonFiberThA;
    x = p1*(y-yco[1])+c1;
    Ch1->SetVertex(1,x,y);
    y = TSCRinC+TSCarbonFiberThA;
    x = p1*(y-yci[1])+c1;
    Ch1->SetVertex(3,x,y);
    //
    th = 0.5*TSCAngle*kRadian;
    xa = TMath::Cos(th)*xco[1]-TMath::Sin(th)*yco[1];
    ya = TMath::Sin(th)*xco[1]+TMath::Cos(th)*yco[1];
    xb = TMath::Cos(th)*xci[1]-TMath::Sin(th)*yci[1];
    yb = TMath::Sin(th)*xci[1]+TMath::Cos(th)*yci[1];
    p1 = (xa-xb)/(ya-yb);
    c1 = xa+0.5*TSCarbonFiberThA*TMath::Sqrt(SQ(xa-xb)+SQ(ya-yb))/(xa-xb);
    y = ya-TSCarbonFiberThA;
    x = p1*(y-ya)+c1;
    xp = TMath::Cos(-th)*x-TMath::Sin(-th)*y;
    yp = TMath::Sin(-th)*x+TMath::Cos(-th)*y;
    yp = ya-TSCarbonFiberThA;
    xp = p1*(y-ya)+c1;
    Ch2->SetVertex(0,xp,yp);
    y = yb+2.0*TSCarbonFiberThA;
    x = p1*(y-ya)+c1;
    xp = TMath::Cos(-th)*x-TMath::Sin(-th)*y;
    yp = TMath::Sin(-th)*x+TMath::Cos(-th)*y;
    Ch2->SetVertex(2,xp,yp);
    xa = TMath::Cos(th)*xco[2]-TMath::Sin(th)*yco[2];
    ya = TMath::Sin(th)*xco[2]+TMath::Cos(th)*yco[2];
    xb = TMath::Cos(th)*xci[2]-TMath::Sin(th)*yci[2];
    yb = TMath::Sin(th)*xci[2]+TMath::Cos(th)*yci[2];
    p1 = (xa-xb)/(ya-yb);
    c1 = xa-0.5*TSCarbonFiberThA*TMath::Sqrt(SQ(xa-xb)+SQ(ya-yb))/(xa-xb);
    y = ya-TSCarbonFiberThA;
    x = p1*(y-ya)+c1;
    xp = TMath::Cos(-th)*x-TMath::Sin(-th)*y;
    yp = TMath::Sin(-th)*x+TMath::Cos(-th)*y;
    Ch2->SetVertex(1,xp,yp);
    y = yb+2.0*TSCarbonFiberThA;
    x = p1*(y-ya)+c1;
    xp = TMath::Cos(-th)*x-TMath::Sin(-th)*y;
    yp = TMath::Sin(-th)*x+TMath::Cos(-th)*y;
    Ch2->SetVertex(3,xp,yp);
    //
    p1 = (yco[5]-yci[5])/(xco[5]-xci[5]);
    c1 = yco[5]+0.5*TSCarbonFiberThA*TMath::Sqrt(SQ(yco[5]-yci[5])+
                                          SQ(xco[5]-xci[5]))/(yco[5]-yci[5]);
    x = xco[5]-TSCarbonFiberThA;
    y = p1*(x-xco[5])+c1;
    Ch3->SetVertex(0,x,y);
    x = xci[5]+2.0*TSCarbonFiberThA;
    y = p1*(x-xci[5])+c1;
    Ch3->SetVertex(2,x,y);
    y = 2.0*TSCarbonFiberThA;
    x = xco[5]-TSCarbonFiberThA;
    Ch3->SetVertex(1,x,y);
    y = 2.0*TSCarbonFiberThA;
    x = xci[5]+2.0*TSCarbonFiberThA;
    Ch3->SetVertex(3,x,y);
    Ch1->SetVertex(7,x7[0],y7[0]); // 7th point most be done last ???
    Ch2->SetVertex(7,x7[1],y7[1]); // 7th point most be done last ???
    Ch3->SetVertex(7,x7[2],y7[2]); // 7th point most be done last ???
    printArb8(A1);
    printArb8(Ah1);
    printArb8(A2);
    printArb8(Ah2);
    printArb8(A3);
    printArb8(Ah3);
    printArb8(B1);
    printArb8(Bh1);
    printArb8(B2);
    printArb8(Bh2);
    printArb8(B3);
    printArb8(Bh3);
    printArb8(C1);
    printArb8(Ch1);
    printArb8(C2);
    printArb8(Ch2);
    printArb8(C3);
    printArb8(Ch3);
    //
    // Define Minimal volume to inclose this SPD Thermal Sheald.
    M1 = new TGeoPcon("ITSspdShealdVV",0.0,360.0,9);
    M1->Z(0)    = 0.5*TSCLengthA+TSCLengthB;
    M1->Rmin(0) = TSCRinB;
    x = B1->GetVertices()[0]; // [0][0]
    y = B1->GetVertices()[1]; // [0][1]
    M1->Rmax(0) = TMath::Sqrt(x*x+y*y);
    M1->Z(1)    = M1->GetZ(0)-TSCLengthB;
    M1->Rmin(1) = M1->GetRmin(0);
    M1->Rmax(1) = M1->GetRmax(0);
    M1->Z(2)    = M1->GetZ(1);
    M1->Rmin(2) = TSCRinA;
    x = A1->GetVertices()[0]; // [0]0]
    y = A1->GetVertices()[1]; // [0][1]
    M1->Rmax(2) = TMath::Sqrt(x*x+y*y);
    M1->Z(3)    = -(M1->GetZ(0)-TSCLengthB);
    M1->Rmin(3) = M1->GetRmin(2);
    M1->Rmax(3) = M1->GetRmax(2);
    M1->Z(4)    = M1->GetZ(3);
    M1->Rmin(4) = M1->GetRmin(1);
    M1->Rmax(4) = M1->GetRmax(1);
    M1->Z(5)    = -(M1->GetZ(0));
    M1->Rmin(5) = M1->GetRmin(0);
    M1->Rmax(5) = M1->GetRmax(0);
    M1->Z(6)    = M1->GetZ(5) - TSCLengthC;
    M1->Rmin(6) = TSCRinC;
    x = C1->GetVertices()[0]; // [0][0]
    y = C1->GetVertices()[1]; // [0][1]
    M1->Rmax(6) = TMath::Sqrt(x*x+y*y);
    M1->Z(7)    = M1->GetZ(6);
    M1->Rmin(7) = D->GetRmin();
    M1->Rmax(7) = D->GetRmax();
    M1->Z(8)    = M1->Z(7) - TSCLengthD;
    M1->Rmin(8) = M1->GetRmin(7);
    M1->Rmax(8) = M1->GetRmax(7);
    M2 = new TGeoTubeSeg("ITSspdShealdWingVV",
         M1->GetRmax(8),Dw->GetRmax(),Dw->GetDz(),Dw->GetPhi1(),Dw->GetPhi2());
    printTubeSeg(M2);
    //
    x = 0.5*(M1->GetZ(8) + M1->GetZ(7));
    tranITSspdShealdVVt0 = new TGeoTranslation("ITSspdShealdVVt0",0.0,0.0,x);
    tranITSspdShealdVVt0->RegisterYourself();
    TGeoRotation rotz90("",0.0,0.0,90.0); // never registered.
    rotITSspdShealdVVt1 = new TGeoCombiTrans(*tranITSspdShealdVVt0,rotz90);
    rotITSspdShealdVVt1->SetName("ITSspdShealdVVt1");
    rotITSspdShealdVVt1->RegisterYourself();
    TGeoRotation rotz180("",0.0,0.0,180.0); // never registered
    rotITSspdShealdVVt2 = new TGeoCombiTrans(*tranITSspdShealdVVt0,rotz180);
    rotITSspdShealdVVt2->SetName("ITSspdShealdVVt2");
    rotITSspdShealdVVt2->RegisterYourself();
    TGeoRotation rotz270("",0.0,0.0,270.0); // never registered
    rotITSspdShealdVVt3 = new TGeoCombiTrans(*tranITSspdShealdVVt0,rotz270);
    rotITSspdShealdVVt3->SetName("ITSspdShealdVVt3");
    rotITSspdShealdVVt3->RegisterYourself();
    M = new TGeoCompositeShape("ITS SPD Thermal sheald volume",
                              "(((ITSspdShealdVV+"
                              "ITSspdShealdWingVV:ITSspdShealdVVt0)+"
                              "ITSspdShealdWingVV:ITSspdShealdVVt1)+"
                              "ITSspdShealdWingVV:ITSspdShealdVVt2)+"
                              "ITSspdShealdWingVV:ITSspdShealdVVt3");
    //
    TGeoManager *mgr = gGeoManager;
    SPDcf = mgr->GetMedium("ITSspdCarbonFiber");
    SPDfs = mgr->GetMedium("ITSspdStaselite4411w");
    SPDfo = mgr->GetMedium("ITSspdRohacell50A");
    SPDss = mgr->GetMedium("ITSspdStainlessSteal");
    SPDair= mgr->GetMedium("ITSspdAir");
    TGeoVolume *A1v,*A2v,*A3v,*Ah1v,*Ah2v,*Ah3v;
    TGeoVolume *B1v,*B2v,*B3v,*Bh1v,*Bh2v,*Bh3v;
    TGeoVolume *C1v,*C2v,*C3v,*Ch1v,*Ch2v,*Ch3v;
    TGeoVolume *Dv,*Dsv,*Dwv,*Dwsv,*Mv;
    Mv = new TGeoVolume("ITSspdThermalSheald",M,SPDair);
    Mv->SetVisibility(kTRUE);
    Mv->SetLineColor(7); // light Blue
    Mv->SetLineWidth(1);
    Mv->SetFillColor(Mv->GetLineColor());
    Mv->SetFillStyle(4090); // 90% transparent
    Moth->AddNode(Mv,1,0); ///////////////////// Virtual Volume ////////
    A1v = new TGeoVolume("ITSspdCentCylA1CF",A1,SPDcf);
    A1v->SetVisibility(kTRUE);
    A1v->SetLineColor(4);
    A1v->SetLineWidth(1);
    A2v = new TGeoVolume("ITSspdCentCylA2CF",A2,SPDcf);
    A2v->SetVisibility(kTRUE);
    A2v->SetLineColor(4);
    A2v->SetLineWidth(1);
    A3v = new TGeoVolume("ITSspdCentCylA3CF",A3,SPDcf);
    A3v->SetVisibility(kTRUE);
    A3v->SetLineColor(4);
    A3v->SetLineWidth(1);
    B1v = new TGeoVolume("ITSspdCentCylB1CF",B1,SPDcf);
    B1v->SetVisibility(kTRUE);
    B1v->SetLineColor(4);
    B1v->SetLineWidth(1);
    B2v = new TGeoVolume("ITSspdCentCylB2CF",B2,SPDcf);
    B2v->SetVisibility(kTRUE);
    B2v->SetLineColor(4);
    B2v->SetLineWidth(1);
    B3v = new TGeoVolume("ITSspdCentCylB3CF",B3,SPDcf);
    B3v->SetVisibility(kTRUE);
    B3v->SetLineColor(4);
    B3v->SetLineWidth(1);
    C1v = new TGeoVolume("ITSspdCentCylC1CF",C1,SPDcf);
    C1v->SetVisibility(kTRUE);
    C1v->SetLineColor(4);
    C1v->SetLineWidth(1);
    C2v = new TGeoVolume("ITSspdCentCylC2CF",C2,SPDcf);
    C2v->SetVisibility(kTRUE);
    C2v->SetLineColor(4);
    C2v->SetLineWidth(1);
    C3v = new TGeoVolume("ITSspdCentCylC3CF",C3,SPDcf);
    C3v->SetVisibility(kTRUE);
    C3v->SetLineColor(4);
    C3v->SetLineWidth(1);
    Ah1v = new TGeoVolume("ITSspdCentCylA1AirA",Ah1,SPDair);
    Ah1v->SetVisibility(kTRUE);
    Ah1v->SetLineColor(5); // Yellow
    Ah1v->SetFillColor(Ah1v->GetLineColor());
    Ah1v->SetFillStyle(4090); // 90% transparent
    Ah2v = new TGeoVolume("ITSspdCentCylA2AirA",Ah2,SPDair);
    Ah2v->SetVisibility(kTRUE);
    Ah2v->SetLineColor(5); // Yellow
    Ah2v->SetFillColor(Ah2v->GetLineColor());
    Ah2v->SetFillStyle(4090); // 90% transparent
    Ah3v = new TGeoVolume("ITSspdCentCylA3AirA",Ah3,SPDair);
    Ah3v->SetVisibility(kTRUE);
    Ah3v->SetLineColor(5); // Yellow
    Ah3v->SetFillColor(Ah3v->GetLineColor());
    Ah3v->SetFillStyle(4090); // 90% transparent
    Bh1v = new TGeoVolume("ITSspdCentCylA1AirB",Bh1,SPDair);
    Bh1v->SetVisibility(kTRUE);
    Bh1v->SetLineColor(5); // Yellow
    Bh1v->SetFillColor(Bh1v->GetLineColor());
    Bh1v->SetFillStyle(4090); // 90% transparent
    Bh2v = new TGeoVolume("ITSspdCentCylA2AirB",Bh2,SPDair);
    Bh2v->SetVisibility(kTRUE);
    Bh2v->SetLineColor(5); // Yellow
    Bh2v->SetFillColor(Bh2v->GetLineColor());
    Bh2v->SetFillStyle(4090); // 90% transparent
    Bh3v = new TGeoVolume("ITSspdCentCylA3AirB",Bh3,SPDair);
    Bh3v->SetVisibility(kTRUE);
    Bh3v->SetLineColor(5); // Yellow
    Bh3v->SetFillColor(Bh3v->GetLineColor());
    Bh3v->SetFillStyle(4090); // 90% transparent
    Ch1v = new TGeoVolume("ITSspdCentCylA1AirC",Ch1,SPDair);
    Ch1v->SetVisibility(kTRUE);
    Ch1v->SetLineColor(5); // Yellow
    Ch1v->SetFillColor(Ch1v->GetLineColor());
    Ch1v->SetFillStyle(4090); // 90% transparent
    Ch2v = new TGeoVolume("ITSspdCentCylA2AirC",Ch2,SPDair);
    Ch2v->SetVisibility(kTRUE);
    Ch2v->SetLineColor(5); // Yellow
    Ch2v->SetFillColor(Ch2v->GetLineColor());
    Ch2v->SetFillStyle(4090); // 90% transparent
    Ch3v = new TGeoVolume("ITSspdCentCylA3AirC",Ch3,SPDair);
    Ch3v->SetVisibility(kTRUE);
    Ch3v->SetLineColor(5); // Yellow
    Ch3v->SetFillColor(Ch3v->GetLineColor());
    Ch3v->SetFillStyle(4090); // 90% transparent
    Dv = new TGeoVolume("ITSspdCentCylA1CD",D,SPDcf);
    Dv->SetVisibility(kTRUE);
    Dv->SetLineColor(4);
    Dv->SetLineWidth(1);
    Dwv = new TGeoVolume("ITSspdCentCylA1CDw",Dw,SPDcf);
    Dwv->SetVisibility(kTRUE);
    Dwv->SetLineColor(4);
    Dwv->SetLineWidth(1);
    Dsv = new TGeoVolume("ITSspdCentCylA1Dfill",Ds,SPDfs);
    Dsv->SetVisibility(kTRUE);
    Dsv->SetLineColor(3); // Green
    Dsv->SetFillColor(Dsv->GetLineColor());
    Dsv->SetFillStyle(4010); // 10% transparent
    Dwsv = new TGeoVolume("ITSspdCentCylA1DwingFill",Dws,SPDfs);
    Dwsv->SetVisibility(kTRUE);
    Dwsv->SetLineColor(3); // Green
    Dwsv->SetFillColor(Dwsv->GetLineColor());
    Dwsv->SetFillStyle(4010); // 10% transparent
    //
    A1v->AddNode(Ah1v,1,0);
    A2v->AddNode(Ah2v,1,0);
    A3v->AddNode(Ah3v,1,0);
    B1v->AddNode(Bh1v,1,0);
    B2v->AddNode(Bh2v,1,0);
    B3v->AddNode(Bh3v,1,0);
    C1v->AddNode(Ch1v,1,0);
    C2v->AddNode(Ch2v,1,0);
    C3v->AddNode(Ch3v,1,0);
    Dv ->AddNode(Dsv ,1,0);
    Dwv->AddNode(Dwsv,1,0);
    //
    Mv->AddNode(A1v,1,0);
    Mv->AddNode(A2v,1,0);
    Mv->AddNode(A3v,1,0);
    tranb  = new TGeoTranslation("",0.0,0.0,0.5*(TSCLengthA+TSCLengthB));
    tranbm = new TGeoTranslation("",0.0,0.0,0.5*(-TSCLengthA-TSCLengthB));
    Mv->AddNode(B1v,1,tranb);
    Mv->AddNode(B2v,1,tranb);
    Mv->AddNode(B3v,1,tranb);
    Mv->AddNode(B1v,2,tranbm);
    Mv->AddNode(B2v,2,tranbm);
    Mv->AddNode(B3v,2,tranbm);
    // Muon side (rb26) is at -Z.
    tranc = new TGeoTranslation("",0.0,0.0,
                                0.5*(-TSCLengthA-TSCLengthB-TSCLengthC));
    Mv->AddNode(C1v,1,tranc);
    Mv->AddNode(C2v,1,tranc);
    Mv->AddNode(C3v,1,tranc);
    Mv->AddNode(Dv,1,tranITSspdShealdVVt0);
    Mv->AddNode(Dwv,1,tranITSspdShealdVVt0);
    Mv->AddNode(Dwv,2,rotITSspdShealdVVt1);
    Mv->AddNode(Dwv,3,rotITSspdShealdVVt2);
    Mv->AddNode(Dwv,4,rotITSspdShealdVVt3);
    k=2;
    for(i=1;i<10;i++) {
      th = ((Double_t)i)*TSCAngle*kDegree;
      rot = new TGeoRotation("",0.0,0.0,th);
      Mv->AddNode(A1v,i+1,rot);
      Mv->AddNode(B1v,i+2,new TGeoCombiTrans(*tranb,*rot));
      Mv->AddNode(B1v,i+12,new TGeoCombiTrans(*tranbm,*rot));
      Mv->AddNode(C1v,i+1,new TGeoCombiTrans(*tranc,*rot));
      if(i!=0||i!=2||i!=7){
        Mv->AddNode(A2v,k++,rot);
        Mv->AddNode(B2v,k++,new TGeoCombiTrans(*tranb,*rot));
        Mv->AddNode(B2v,k++,new TGeoCombiTrans(*tranbm,*rot));
        Mv->AddNode(C2v,k++,new TGeoCombiTrans(*tranc,*rot));
      } // end if
      if(i==5) {
        Mv->AddNode(A3v,2,rot);
        Mv->AddNode(B3v,3,new TGeoCombiTrans(*tranb,*rot));
        Mv->AddNode(B3v,4,new TGeoCombiTrans(*tranbm,*rot));
        Mv->AddNode(C3v,2,new TGeoCombiTrans(*tranc,*rot));
      } // end if
    } // end for i
    rot = new TGeoRotation("",180.,0.0,0.0);
    Mv->AddNode(A3v,3,rot);
    Mv->AddNode(B3v,5,new TGeoCombiTrans(*tranb,*rot));
    Mv->AddNode(B3v,6,new TGeoCombiTrans(*tranbm,*rot));
    Mv->AddNode(C3v,3,new TGeoCombiTrans(*tranc,*rot));
    rot = new TGeoRotation("",180.,0.0,180.0);
    Mv->AddNode(A3v,4,rot);
    Mv->AddNode(B3v,7,new TGeoCombiTrans(*tranb,*rot));
    Mv->AddNode(B3v,8,new TGeoCombiTrans(*tranbm,*rot));
    Mv->AddNode(C3v,4,new TGeoCombiTrans(*tranc,*rot));
    if(GetDebug()){
        A1v->PrintNodes();
        Ah1v->PrintNodes();
        A2v->PrintNodes();
        Ah2v->PrintNodes();
        A3v->PrintNodes();
        Ah3v->PrintNodes();
        B1v->PrintNodes();
        Bh1v->PrintNodes();
        B2v->PrintNodes();
        Bh2v->PrintNodes();
        B3v->PrintNodes();
        Bh3v->PrintNodes();
        C1v->PrintNodes();
        Ch1v->PrintNodes();
        C2v->PrintNodes();
        Ch2v->PrintNodes();
        C3v->PrintNodes();
        Ch3v->PrintNodes();
        Dv->PrintNodes();
        Dsv->PrintNodes();
        Dwv->PrintNodes();
        Dwsv->PrintNodes();
        //Mv->PrintNodes();
    } // end if
}
//______________________________________________________________________
void AliITSv11::SDDCone(TGeoVolume *Moth){
    // Define the detail SDD support cone geometry.
    // Inputs:
    //   none.
    // Outputs:
    //  none.
    // Return:
    //  none.
    //
    // From Cilindro Centrale - Lavorazioni, ALR 0816/1 04/08/03 File
    // name SDD/Cilindro.hpgl
    const Double_t TSLength       = 790.0*kmm; // Thermal Sheeld length
    const Double_t TSInsertoLength= 15.0*kmm;    // ????
    const Double_t TSOuterR       = 0.5*(220.+10.)*kmm; // ????
    const Double_t TSInnerR       = 0.5*(220.-10.)*kmm; // ????
    const Double_t TSCarbonFiberth= 0.02*kmm;     // ????
    const Double_t TSBoltDiameter = 6.0*kmm; // M6 screw
    const Double_t TSBoltDepth    = 6.0*kmm; // in volume C
    const Double_t TSBoltRadius   = 0.5*220.*kmm; // Radius in volume C
    const Double_t TSBoltAngle0   = 0.0*kDegree; // Angle in volume C
    const Double_t TSBoltdAngle   = 30.0*kDegree; // Angle in Volume C
    Double_t x,y,z,t,t0;
    Int_t i,n;
    TGeoTube *A,*B,*C,*D;
    TGeoTranslation *tran;
    TGeoRotation *rot;
    TGeoCombiTrans *rotran;
    TGeoMedium *SDDcf,*SDDfs,*SDDfo,*SDDss;

    A = new TGeoTube("ITS SDD Central Cylinder",TSInnerR,TSOuterR,.5*TSLength);
    B = new TGeoTube("ITS SDD CC Foam",TSInnerR+TSCarbonFiberth,
                    TSOuterR-TSCarbonFiberth,
                    0.5*(TSLength-2.0*TSInsertoLength));
    C = new TGeoTube("ITS SDD CC Inserto",TSInnerR+TSCarbonFiberth,
                    TSOuterR-TSCarbonFiberth,0.5*TSLength);
    D = new TGeoTube("ITS SDD CC M6 bolt end",0.0,0.5*TSBoltDiameter,
                    0.5*TSBoltDepth);
    printTube(A);
    printTube(B);
    printTube(C);
    printTube(D);
    //
    TGeoManager *mgr = gGeoManager;
    SDDcf = mgr->GetMedium("ITSssdCarbonFiber");
    SDDfs = mgr->GetMedium("ITSssdStaselite4411w");
    SDDfo = mgr->GetMedium("ITSssdRohacell50A");
    SDDss = mgr->GetMedium("ITSssdStainlessSteal");
    TGeoVolume *Av,*Bv,*Cv,*Dv;
    Av = new TGeoVolume("ITSsddCentCylCF",A,SDDcf);
    Av->SetVisibility(kTRUE);
    Av->SetLineColor(4);
    Av->SetLineWidth(1);
    Av->SetFillColor(Av->GetLineColor());
    Av->SetFillStyle(4000); // 0% transparent
    Bv = new TGeoVolume("ITSsddCentCylF",B,SDDfo);
    Bv->SetVisibility(kTRUE);
    Bv->SetLineColor(3);
    Bv->SetLineWidth(1);
    Bv->SetFillColor(Bv->GetLineColor());
    Bv->SetFillStyle(4000); // 0% transparent
    Cv = new TGeoVolume("ITSsddCentCylSt",C,SDDfs);
    Cv->SetVisibility(kTRUE);
    Cv->SetLineColor(2);
    Cv->SetLineWidth(1);
    Cv->SetFillColor(Cv->GetLineColor());
    Cv->SetFillStyle(4000); // 0% transparent
    Dv = new TGeoVolume("ITSsddCentCylSS",D,SDDss);
    Dv->SetVisibility(kTRUE);
    Dv->SetLineColor(1);
    Dv->SetLineWidth(1);
    Dv->SetFillColor(Dv->GetLineColor());
    Dv->SetFillStyle(4000); // 0% transparent
    //
    Moth->AddNode(Av,1,0);
    Av->AddNode(Cv,1,0);
    Cv->AddNode(Bv,1,0);
    n = (Int_t)((360.*kDegree)/TSBoltdAngle);
    for(i=0;i<n;i++){
        t = TSBoltAngle0+((Double_t)i)*TSBoltdAngle;
        x = TSBoltRadius*TMath::Cos(t*kRadian);
        y = TSBoltRadius*TMath::Sin(t*kRadian);
        z = 0.5*(TSLength-TSBoltDepth);
        tran = new TGeoTranslation("",x,y,z);
        Cv->AddNode(Dv,i+1,tran);
        tran = new TGeoTranslation("",x,y,-z);
        Cv->AddNode(Dv,i+n+1,tran);
    } // end for i
    if(GetDebug()){
        Av->PrintNodes();
        Bv->PrintNodes();
        Cv->PrintNodes();
        Dv->PrintNodes();
    } // end if
    // SDD Suport Cone
    //
    //
    const Double_t Thickness = 10.5*kmm; // Thickness of Rohacell+carbon fiber
    const Double_t Cthick    = 1.5*kmm; // Carbon finber thickness
    const Double_t Rcurv     = 15.0*kmm; // Radius of curvature.
    const Double_t Tc        = 45.0; // angle of SSD cone [degrees].
    const Double_t Sintc = TMath::Sin(Tc*TMath::DegToRad());
    const Double_t Costc = TMath::Cos(Tc*TMath::DegToRad());
    const Double_t Tantc = TMath::Tan(Tc*TMath::DegToRad());
    const Double_t ZouterMilled = 23.0*kmm;
    const Double_t Zcylinder    = 186.0*kmm;
    const Double_t Z0           = Zcylinder + 0.5*TSLength;
    //const Int_t Nspoaks         = 12;
    //const Int_t Nmounts         = 4;
    //const Double_t DmountAngle  = 9.0; // degrees
    const Double_t RoutMax      = 0.5*560.0*kmm;
    const Double_t RoutMin      = 0.5*539.0*kmm;
    // Holes in cone for cables
    const Double_t PhiHole1     = 0.0*kDegree;
    const Double_t dPhiHole1    = 25.0*kDegree;
    const Double_t RholeMax1    = 0.5*528.*kmm;
    const Double_t RholeMin1    = 0.5*464.*kmm;
    const Double_t PhiHole2     = 0.0*kDegree;
    const Double_t dPhiHole2    = 50.0*kDegree;
    const Double_t RholeMax2    = 0.5*375.*kmm;
    const Double_t RholeMin2    = 0.5*280.*kmm;
    //
    //const Int_t NpostsOut       = 6;
    //const Int_t NpostsIn        = 3;
    //const Double_t Phi0PostOut  = 0.0; // degree
    //const Double_t Phi0PostIn   = 0.0; // degree
    //const Double_t dRpostOut    = 16.0*kmm;
    //const Double_t dRpostIn     = 16.0*kmm;
    //const Double_t ZpostMaxOut  = 116.0*kmm;
    //const Double_t ZpostMaxIn   = 190.0*kmm;
    const Double_t RinMax       = 0.5*216*kmm;
    const Double_t RinCylinder  = 0.5*231.0*kmm;
    //const Double_t RinHole      = 0.5*220.0*kmm;
    const Double_t RinMin       = 0.5*210.0*kmm;
    const Double_t dZin         = 15.0*kmm; // ???
    //
    Double_t dza = Thickness/Sintc-(RoutMax-RoutMin)/Tantc;
    Double_t Z,Rmin,Rmax; // Temp variables.
    if(dza<=0){ // The number or order of the points are in error for a proper
     // call to pcons!
     Error("SDDcone","The definition of the points for a call to PCONS is"
           " in error. abort.");
     return;
    } // end if
    TGeoPcon *E = new TGeoPcon("ITSsddSuportConeCarbonFiberSurfaceE",
                               0.0,360.0,12);
    E->Z(0)    = 0.0;
    E->Rmin(0) = RoutMin;
    E->Rmax(0) = RoutMax;
    E->Z(1)    = ZouterMilled - dza;
    E->Rmin(1) = E->GetRmin(0);
    E->Rmax(1) = E->GetRmax(0);
    E->Z(2)    = ZouterMilled;
    E->Rmax(2) = E->GetRmax(0);
    RadiusOfCurvature(Rcurv,0.,E->GetZ(1),E->GetRmin(1),Tc,Z,Rmin);
    E->Z(3)    = Z;
    E->Rmin(3) = Rmin;
    E->Rmin(2) = RminFrom2Points(E,3,1,E->GetZ(2));
    RadiusOfCurvature(Rcurv,0.,E->GetZ(2),E->GetRmax(2),Tc,Z,Rmax);
    E->Z(4)    = Z;
    E->Rmax(4) = Rmax;
    E->Rmin(4) = RminFromZpCone(E,Tc,E->GetZ(4),0.0);
    E->Rmax(3) = RmaxFrom2Points(E,4,2,E->GetZ(3));
    E->Rmin(7) = RinMin;
    E->Rmin(8) = RinMin;
    RadiusOfCurvature(Rcurv,90.0,0.0,RinMax,90.0-Tc,Z,Rmax);
    E->Rmax(8) = Rmax;
    E->Z(8)    = ZFromRmaxpCone(E,Tc,E->GetRmax(8));
    E->Z(9)    = Zcylinder;
    E->Rmin(9) = RinMin;
    E->Z(10)    = E->GetZ(9);
    E->Rmin(10) = RinCylinder;
    E->Rmin(11) = RinCylinder;
    E->Rmax(11) = E->GetRmin(11);
    Rmin        = E->GetRmin(8);
    RadiusOfCurvature(Rcurv,90.0-Tc,E->GetZ(8),E->GetRmax(8),90.0,Z,Rmax);
    Rmax = RinMax;
    E->Z(11)    = Z+(E->GetZ(8)-Z)*(E->GetRmax(11)-Rmax)/(E->GetRmax(8)-Rmax);
    E->Rmax(9) = RmaxFrom2Points(E,11,8,E->GetZ(9));
    E->Rmax(10) = E->GetRmax(9);
    E->Z(6)    = Z-dZin;
    E->Z(7)    = E->GetZ(6);
    E->Rmax(6) = RmaxFromZpCone(E,Tc,E->GetZ(6));
    E->Rmax(7) = E->GetRmax(6);
    RadiusOfCurvature(Rcurv,90.,E->GetZ(6),0.0,90.0-Tc,Z,Rmin);
    E->Z(5)    = Z;
    E->Rmin(5) = RminFromZpCone(E,Tc,Z);
    E->Rmax(5) = RmaxFromZpCone(E,Tc,Z);
    RadiusOfCurvature(Rcurv,90.-Tc,0.0,E->Rmin(5),90.0,Z,Rmin);
    E->Rmin(6) = Rmin;
    printPcon(E);
    // Inner Core, Inserto material
    TGeoPcon *F = new TGeoPcon("ITSsddSuportConeInsertoStesaliteF",0.,360.0,9);
    F->Z(0)    = E->GetZ(0);
    F->Rmin(0) = E->GetRmin(0)+Cthick;
    F->Rmax(0) = E->GetRmax(0)-Cthick;
    F->Z(1)    = E->GetZ(1);
    F->Rmin(1) = F->GetRmin(0);
    F->Rmax(1) = F->GetRmax(0);
    F->Z(2)    = E->GetZ(2);
    F->Rmax(2) = F->GetRmax(1);
    RadiusOfCurvature(Rcurv-Cthick,0.,F->GetZ(1),F->GetRmax(1),Tc,Z,Rmin);
    F->Z(3)    = Z;
    F->Rmin(3) = Rmin;
    F->Rmin(2) = RminFrom2Points(F,3,1,F->GetZ(2));
    RadiusOfCurvature(Rcurv+Cthick,0.,F->GetZ(2),F->GetRmax(2),Tc,Z,Rmax);
    F->Z(4)    = Z;
    F->Rmax(4) = Rmax;
    F->Rmin(4) = RmaxFromZpCone(E,Tc,F->GetZ(4),-Cthick);
    F->Rmax(3) = RmaxFrom2Points(F,4,2,F->GetZ(3));
    F->Rmin(7) = E->GetRmin(7);
    F->Rmin(8) = E->GetRmin(8);
    F->Z(6)    = E->GetZ(6)+Cthick;
    F->Rmin(6) = E->GetRmin(6);
    F->Z(7)    = F->GetZ(6);
    F->Rmax(8) = E->GetRmax(8)-Cthick*Sintc;
    RadiusOfCurvature(Rcurv+Cthick,90.,F->GetZ(6),F->GetRmin(6),90.-Tc,Z,Rmin);
    F->Z(5)    = Z;
    F->Rmin(5) = Rmin;
    F->Rmax(5) = RmaxFromZpCone(F,Tc,Z);
    F->Rmax(6) = RmaxFromZpCone(F,Tc,F->GetZ(6));
    F->Rmax(7) = F->GetRmax(6);
    F->Z(8)    = ZFromRmaxpCone(F,Tc,F->GetRmax(8),-Cthick);
    printPcon(F);
    // Inner Core, Inserto material
    TGeoPcon *G = new TGeoPcon("ITSsddSuportConeFoamCoreG",0.0,360.0,4);
    RadiusOfCurvature(Rcurv+Cthick,0.0,F->GetZ(1),F->GetRmin(1),Tc,Z,Rmin);
    G->Z(0)    = Z;
    G->Rmin(0) = Rmin;
    G->Rmax(0) = G->GetRmin(0);
    G->Z(1)    = G->GetZ(0)+(Thickness-2.0*Cthick)/Sintc;;
    G->Rmin(1) = RminFromZpCone(F,Tc,G->GetZ(1));
    G->Rmax(1) = RmaxFromZpCone(F,Tc,G->GetZ(1));
    G->Z(2)    = E->GetZ(5)-Cthick;
    G->Rmin(2) = RminFromZpCone(F,Tc,G->GetZ(2));
    G->Rmax(2) = RmaxFromZpCone(F,Tc,G->GetZ(2));
    G->Z(3)    = F->GetZ(5)+(Thickness-2.0*Cthick)*Costc;
    G->Rmax(3) = RmaxFromZpCone(F,Tc,G->GetZ(3));
    G->Rmin(3) = G->GetRmax(3);
    printPcon(G);
    //
    TGeoPcon *H = new TGeoPcon("ITSsddSuportConeHoleH",PhiHole1,dPhiHole1,4);
    H->Rmin(0) = RholeMax1;
    H->Rmax(0) = H->GetRmin(0);
    H->Z(0)    = ZFromRminpCone(E,Tc,H->GetRmin(0));
    H->Rmax(1) = H->GetRmax(0);
    H->Z(1)    = ZFromRmaxpCone(E,Tc,H->GetRmax(1));
    H->Rmin(1) = RminFromZpCone(E,Tc,H->GetZ(1));
    H->Rmin(2) = RholeMin1;
    H->Z(2)    = ZFromRminpCone(E,Tc,H->GetRmin(2));
    H->Rmax(2) = RmaxFromZpCone(E,Tc,H->GetZ(2));
    H->Rmin(3) = H->GetRmin(2);
    H->Rmax(3) = H->GetRmin(3);
    H->Z(3)    = ZFromRminpCone(E,Tc,H->GetRmin(3));
    printPcon(H);
    //
    x = Cthick/(0.5*(RholeMax1+RholeMin1));
    t0 = PhiHole1 - x/kRadian;
    t  = dPhiHole1 + 2.0*x/kRadian;
    TGeoPcon *I = new TGeoPcon("ITSsddSuportConeHoleI",t0,t,4);
    I->Rmin(0) = RholeMax1+Cthick;
    I->Rmax(0) = I->GetRmin(0);
    I->Z(0)    = ZFromRminpCone(F,Tc,I->GetRmin(0));
    I->Rmax(1) = I->GetRmax(0);
    I->Z(1)    = ZFromRmaxpCone(F,Tc,I->GetRmax(1));
    I->Rmin(1) = RminFromZpCone(F,Tc,I->GetZ(1));
    I->Rmin(2) = RholeMin1-Cthick;
    I->Z(2)    = ZFromRminpCone(F,Tc,I->GetRmin(2));
    I->Rmax(2) = RmaxFromZpCone(F,Tc,I->GetZ(2));
    I->Rmin(3) = I->GetRmin(2);
    I->Rmax(3) = I->GetRmin(3);
    I->Z(3)    = ZFromRmaxpCone(F,Tc,I->GetRmax(3));
    printPcon(I);
    //
    TGeoPcon *J = new TGeoPcon("ITSsddSuportConeHoleJ",PhiHole2,dPhiHole2,4);
    J->Rmin(0) = RholeMax2;
    J->Rmax(0) = J->GetRmin(0);
    J->Z(0)    = ZFromRminpCone(E,Tc,J->GetRmin(0));
    J->Rmax(1) = J->GetRmax(0);
    J->Z(1)    = ZFromRmaxpCone(E,Tc,J->GetRmax(1));
    J->Rmin(1) = RminFromZpCone(E,Tc,J->GetZ(1));
    J->Rmin(2) = RholeMin2;
    J->Z(2)    = ZFromRminpCone(E,Tc,J->GetRmin(2));
    J->Rmax(2) = RmaxFromZpCone(E,Tc,J->GetZ(2));
    J->Rmin(3) = J->GetRmin(2);
    J->Rmax(3) = J->GetRmin(3);
    J->Z(3)    = ZFromRmaxpCone(E,Tc,J->GetRmax(3));
    printPcon(J);
    //
    x = Cthick/(0.5*(RholeMax2+RholeMin2));
    t0 = PhiHole2 - x/kRadian;
    t  = dPhiHole2 + 2.0*x/kRadian;
    TGeoPcon *K = new TGeoPcon("ITSsddSuportConeHoleK",t0,t,4);
    K->Rmin(0) = RholeMax2+Cthick;
    K->Rmax(0) = K->GetRmin(0);
    K->Z(0)    = ZFromRminpCone(F,Tc,K->GetRmin(0));
    K->Rmax(1) = K->GetRmax(0);
    K->Z(1)    = ZFromRmaxpCone(F,Tc,K->GetRmax(1));
    K->Rmin(1) = RminFromZpCone(F,Tc,K->GetZ(1));
    K->Rmin(2) = RholeMin2-Cthick;
    K->Z(2)    = ZFromRminpCone(F,Tc,K->GetRmin(2));
    K->Rmax(2) = RmaxFromZpCone(F,Tc,K->GetZ(2));
    K->Rmin(3) = K->GetRmin(2);
    K->Rmax(3) = K->GetRmin(3);
    K->Z(3)    = ZFromRmaxpCone(F,Tc,K->GetRmax(3));
    printPcon(K);
    //
    TGeoCompositeShape *L,*M,*N;
    rot = new TGeoRotation("ITSsddRotZ30",0.0,0.0,30.0);
    rot->RegisterYourself();
    rot = new TGeoRotation("ITSsddRotZ60",0.0,0.0,60.0);
    rot->RegisterYourself();
    rot = new TGeoRotation("ITSsddRotZ90",0.0,0.0,90.0);
    rot->RegisterYourself();
    rot = new TGeoRotation("ITSsddRotZ120",0.0,0.0,120.0);
    rot->RegisterYourself();
    rot = new TGeoRotation("ITSsddRotZ150",0.0,0.0,150.0);
    rot->RegisterYourself();
    rot = new TGeoRotation("ITSsddRotZ180",0.0,0.0,180.0);
    rot->RegisterYourself();
    rot = new TGeoRotation("ITSsddRotZ210",0.0,0.0,210.0);
    rot->RegisterYourself();
    rot = new TGeoRotation("ITSsddRotZ240",0.0,0.0,240.0);
    rot->RegisterYourself();
    rot = new TGeoRotation("ITSsddRotZ270",0.0,0.0,270.0);
    rot->RegisterYourself();
    rot = new TGeoRotation("ITSsddRotZ300",0.0,0.0,300.0);
    rot->RegisterYourself();
    rot = new TGeoRotation("ITSsddRotZ330",0.0,0.0,330.0);
    rot->RegisterYourself();
    L = new TGeoCompositeShape("ITS SDD Suport Cone","((((((((((((((((("
                               "ITSsddSuportConeCarbonFiberSurfaceE -"
                               "ITSsddSuportConeHoleH)  -"
                               "ITSsddSuportConeHoleH:ITSsddRotZ30) -"
                               "ITSsddSuportConeHoleH:ITSsddRotZ60) -"
                               "ITSsddSuportConeHoleH:ITSsddRotZ90) -"
                               "ITSsddSuportConeHoleH:ITSsddRotZ120) -"
                               "ITSsddSuportConeHoleH:ITSsddRotZ150) -"
                               "ITSsddSuportConeHoleH:ITSsddRotZ180) -"
                               "ITSsddSuportConeHoleH:ITSsddRotZ210) -"
                               "ITSsddSuportConeHoleH:ITSsddRotZ240) -"
                               "ITSsddSuportConeHoleH:ITSsddRotZ270) -"
                               "ITSsddSuportConeHoleH:ITSsddRotZ300) -"
                               "ITSsddSuportConeHoleH:ITSsddRotZ330) -"
                               "ITSsddSuportConeHoleJ)  -"
                               "ITSsddSuportConeHoleJ:ITSsddRotZ60) -"
                               "ITSsddSuportConeHoleJ:ITSsddRotZ120) -"
                               "ITSsddSuportConeHoleJ:ITSsddRotZ180) -"
                               "ITSsddSuportConeHoleJ:ITSsddRotZ240) -"
                               "ITSsddSuportConeHoleJ:ITSsddRotZ300");
    M = new TGeoCompositeShape("ITS SDD Suport Cone Inserto Stesalite",
                               "((((((((((((((((("
                               "ITSsddSuportConeInsertoStesaliteF -"
                               "ITSsddSuportConeHoleI)  -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ30) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ60) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ90) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ120) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ150) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ180) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ210) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ240) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ270) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ300) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ330) -"
                               "ITSsddSuportConeHoleK)  -"
                               "ITSsddSuportConeHoleK:ITSsddRotZ60) -"
                               "ITSsddSuportConeHoleK:ITSsddRotZ120) -"
                               "ITSsddSuportConeHoleK:ITSsddRotZ180) -"
                               "ITSsddSuportConeHoleK:ITSsddRotZ240) -"
                               "ITSsddSuportConeHoleK:ITSsddRotZ300");
    N = new TGeoCompositeShape("ITS SDD Suport Cone Foam Core",
                               "((((((((((((((((("
                               "ITSsddSuportConeFoamCoreG -"
                               "ITSsddSuportConeHoleI)  -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ30) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ60) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ90) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ120) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ150) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ180) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ210) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ240) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ270) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ300) -"
                               "ITSsddSuportConeHoleI:ITSsddRotZ330) -"
                               "ITSsddSuportConeHoleK)  -"
                               "ITSsddSuportConeHoleK:ITSsddRotZ60) -"
                               "ITSsddSuportConeHoleK:ITSsddRotZ120) -"
                               "ITSsddSuportConeHoleK:ITSsddRotZ180) -"
                               "ITSsddSuportConeHoleK:ITSsddRotZ240) -"
                               "ITSsddSuportConeHoleK:ITSsddRotZ300");
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    TGeoVolume *Lv,*Mv,*Nv;
    Lv = new TGeoVolume("ITSsddConeL",L,SDDcf);
    Lv->SetVisibility(kTRUE);
    Lv->SetLineColor(4);
    Lv->SetLineWidth(1);
    Lv->SetFillColor(Lv->GetLineColor());
    Lv->SetFillStyle(4000); // 0% transparent
    Mv = new TGeoVolume("ITSsddConeM",M,SDDfs);
    Mv->SetVisibility(kTRUE);
    Mv->SetLineColor(2);
    Mv->SetLineWidth(1);
    Mv->SetFillColor(Mv->GetLineColor());
    Mv->SetFillStyle(4010); // 10% transparent
    Nv = new TGeoVolume("ITSsddConeN",N,SDDfo);
    Nv->SetVisibility(kTRUE);
    Nv->SetLineColor(7);
    Nv->SetLineWidth(1);
    Nv->SetFillColor(Nv->GetLineColor());
    Nv->SetFillStyle(4050); // 50% transparent
    //
    Mv->AddNode(Nv,1,0);
    Lv->AddNode(Mv,1,0);
    tran = new TGeoTranslation("",0.0,0.0,-Z0);
    Moth->AddNode(Lv,1,tran);
    rot = new TGeoRotation("",0.0,180.0*kDegree,0.0);
    rotran = new TGeoCombiTrans("",0.0,0.0,Z0,rot);
    delete rot;// rot not explicity used in AddNode functions.
    Moth->AddNode(Lv,2,rotran);
    if(GetDebug()){
        Lv->PrintNodes();
        Mv->PrintNodes();
        Nv->PrintNodes();
    } // end if
}
//______________________________________________________________________
void AliITSv11::SSDCone(TGeoVolume *Moth){
    // Define the detail SSD support cone geometry.
    // Inputs:
    //   none.
    // Outputs:
    //  none.
    // Return:
    //  none.
    //
    TGeoMedium *SSDcf  = 0; // SSD support cone Carbon Fiber materal number.
    TGeoMedium *SSDfs  = 0; // SSD support cone inserto stesalite 4411w.
    TGeoMedium *SSDfo  = 0; // SSD support cone foam, Rohacell 50A.
    TGeoMedium *SSDss  = 0; // SSD support cone screw material,Stainless steal
    TGeoMedium *SSDair = 0; // SSD support cone Air
    TGeoMedium *SSDal  = 0; // SSD support cone SDD mounting bracket Al
    TGeoManager *mgr = gGeoManager;
    SSDcf = mgr->GetMedium("ITSssdCarbonFiber");
    SSDfs = mgr->GetMedium("ITSssdStaselite4411w");
    SSDfo = mgr->GetMedium("ITSssdRohacell50A");
    SSDss = mgr->GetMedium("ITSssdStainlessSteal");
    SSDair= mgr->GetMedium("ITSssdAir");
    SSDal = mgr->GetMedium("ITSssdAl");
    //
    // SSD Central cylinder/Thermal Sheald.
    const Double_t ZThCylinder = 1140.0*kmm; //
    const Double_t ZThCylFoam  = 1020.0*kmm; //
    const Double_t RThCylOuter = 0.5*595.0*kmm; //
    const Double_t CthickThCyl = 0.64*kmm; //
    const Double_t FoamThickTh = 5.0*kmm; //
    const Double_t dZThCylThEnd = 6.0*kmm; //
    const Double_t RThCylInerEnd= 0.5*560.5*kmm; //
    TGeoPcon *CA,*CB;
    TGeoTube*CC;
    //
    CA = new TGeoPcon("ITS SSD Thermal Centeral CylinderA",0.0,360.0,6);
    CB = new TGeoPcon("ITS SSD Thermal Centeral CylinderB",0.0,360.0,6);
    CC = new TGeoTube("ITS SSD Thermal Centeral CylinderC",
                      RThCylOuter-CthickThCyl-FoamThickTh,
                      RThCylOuter-CthickThCyl,0.5*ZThCylFoam);
    CA->Z(0)    = -0.5*ZThCylinder;
    CA->Rmin(0) = RThCylInerEnd;
    CA->Rmax(0) = RThCylOuter;
    CA->Z(1)    = CA->GetZ(0) + dZThCylThEnd;
    CA->Rmin(1) = CA->GetRmin(0);
    CA->Rmax(1) = CA->GetRmax(0);
    CA->Z(2)    = -0.5*ZThCylFoam;
    CA->Rmin(2) = RThCylOuter - 2.0*CthickThCyl-FoamThickTh;
    CA->Rmax(2) = CA->GetRmax(0);
    CA->Z(3)    = -CA->GetZ(2);
    CA->Rmin(3) = CA->GetRmin(2);
    CA->Rmax(3) = CA->GetRmax(2);
    CA->Z(4)    = -CA->GetZ(1);
    CA->Rmin(4) = CA->GetRmin(1);
    CA->Rmax(4) = CA->GetRmax(1);
    CA->Z(5)    = -CA->GetZ(0);
    CA->Rmin(5) = CA->GetRmin(0);
    CA->Rmax(5) = CA->GetRmax(0);
    //
    CB->Z(0)    = CA->GetZ(0);
    CB->Rmin(0) = CA->GetRmin(0) + CthickThCyl;
    CB->Rmax(0) = CA->GetRmax(0) - CthickThCyl;
    CB->Z(1)    = CA->GetZ(1);
    CB->Rmin(1) = CA->GetRmin(1) + CthickThCyl;
    CB->Rmax(1) = CA->GetRmax(1) - CthickThCyl;
    CB->Z(2)    = CA->GetZ(2);
    CB->Rmin(2) = CA->GetRmin(2) + CthickThCyl;
    CB->Rmax(2) = CA->GetRmax(2) - CthickThCyl;
    CB->Z(3)    = CA->GetZ(3);
    CB->Rmin(3) = CA->GetRmin(3) + CthickThCyl;
    CB->Rmax(3) = CA->GetRmax(3) - CthickThCyl;
    CB->Z(4)    = CA->GetZ(4);
    CB->Rmin(4) = CA->GetRmin(4) + CthickThCyl;
    CB->Rmax(4) = CA->GetRmax(4) - CthickThCyl;
    CB->Z(5)    = CA->GetZ(5);
    CB->Rmin(5) = CA->GetRmin(5) + CthickThCyl;
    CB->Rmax(5) = CA->GetRmax(5) - CthickThCyl;
    //
    printPcon(CA);
    printPcon(CB);
    printTube(CC);
    TGeoVolume *CAv,*CBv,*CCv;
    CAv = new TGeoVolume("ITSssdCentCylCA",CA,SSDcf);
    CAv->SetVisibility(kTRUE);
    CAv->SetLineColor(4); // blue
    CAv->SetLineWidth(1);
    CAv->SetFillColor(CAv->GetLineColor());
    CAv->SetFillStyle(4010); // 10% transparent
    CBv = new TGeoVolume("ITSssdCentCylCB",CB,SSDfs);
    CBv->SetVisibility(kTRUE);
    CBv->SetLineColor(2); // red
    CBv->SetLineWidth(1);
    CBv->SetFillColor(CBv->GetLineColor());
    CBv->SetFillStyle(4010); // 10% transparent
    CCv = new TGeoVolume("ITSssdCentCylCC",CC,SSDfo);
    CCv->SetVisibility(kTRUE);
    CCv->SetLineColor(3); // green
    CCv->SetLineWidth(1);
    CCv->SetFillColor(CCv->GetLineColor());
    CCv->SetFillStyle(4010); // 10% transparent
    CBv->AddNode(CCv,1,0);
    CAv->AddNode(CBv,1,0);
    Moth->AddNode(CAv,1,0);
    if(GetDebug()){
        CAv->PrintNodes();
        CBv->PrintNodes();
        CCv->PrintNodes();
    } // end if
    //
    const Double_t Thickness = 13.0*kmm; // Thickness of Rohacell+carbon fiber
    const Double_t Cthick    = 1.5*kmm; // Carbon finber thickness
    const Double_t Rcurv     = 15.0*kmm; // Radius of curvature.
    const Double_t Tc        = 51.0; // angle of SSD cone [degrees].
    const Double_t Sintc = TMath::Sin(Tc*TMath::DegToRad());
    const Double_t Costc = TMath::Cos(Tc*TMath::DegToRad());
    const Double_t Tantc = TMath::Tan(Tc*TMath::DegToRad());
    const Double_t ZouterMilled = (13.5-5.0)*kmm;
    const Double_t Zcylinder    = 170.0*kmm;
    const Double_t Z0           = Zcylinder + 0.5*ZThCylinder;
    const Int_t Nspoaks         = 12;
    const Int_t Nmounts         = 4;
    const Double_t DmountAngle  = 9.0; // degrees
    const Double_t RoutMax      = 0.5*985.0*kmm;
    const Double_t RoutHole     = 0.5*965.0*kmm;
    const Double_t RoutMin      = 0.5*945.0*kmm;
    const Double_t RholeMax     = 0.5*890.0*kmm;
    const Double_t RholeMin     = 0.5*740.0*kmm;
    const Double_t RpostMin     = 316.0*kmm;
    const Double_t ZpostMax     = 196.0*kmm;
    const Int_t Nposts          = 6;
    const Double_t Phi0Post     = 0.0; // degree
    const Double_t dRpost       = 23.0*kmm;
    const Double_t RinMax       = 0.5*590.0*kmm;
    const Double_t RinCylinder  = 0.5*597.0*kmm;
    const Double_t RinHole      = 0.5*575.0*kmm;
    const Double_t RinMin       = 0.5*562.0*kmm;
    const Double_t dZin         = 15.0*kmm;
    // SSD-SDD Thermal/Mechanical cylinder mounts
    const Int_t NinScrews          = 40;
    const Double_t Phi0Screws      = 0.5*360.0/((const Double_t)NinScrews);//d
    const Double_t RcylinderScrews = 0.5*570.0*kmm;//from older drawing????
    const Double_t DscrewHead      = 8.0*kmm;
    const Double_t DscrewShaft     = 4.6*kmm;
    const Double_t ThScrewHeadHole = 8.5*kmm;
    // SDD mounting bracket, SSD part
    const Double_t NssdSupports      = 3;// mounting of U and T
    const Double_t DssdsddBracketAngle = 9.0; // degrees
    const Double_t Phi0SDDsupports   = 0.0; // degree
    const Double_t RsddSupportPlate  = 0.5*585.0*kmm;
    const Double_t ThSDDsupportPlate = 4.0*kmm;
    const Double_t WsddSupportPlate  = 70.0*kmm;
    //
    // Lets start with the upper left outer carbon fiber surface.
    // Between za[2],rmaxa[2] and za[4],rmaxa[4] there is a curved section
    // given by rmaxa = rmaxa[2]-r*Sind(t) for 0<=t<=Tc and 
    // za = za[2] + r*Cosd(t) for 0<=t<=Tc. Simularly between za[1],rmina[1
    // and za[3],rmina[3] there is a curve section given by
    // rmina = rmina[1]-r*Sind(t) for 0<=t<=Tc and za = za[1]+r&Sind(t)
    // for t<=0<=Tc. These curves have been replaced by straight lines
    // between the equivelent points for simplicity.
    Double_t dza = Thickness/Sintc-(RoutMax-RoutMin)/Tantc;
    Int_t i,j;
    Double_t x,y,z[9],rn[9],rx[9],phi,dphi;
    Double_t t,t0,Z,Rmin,Rmax; // Temp variables.
    if(dza<=0){ // The number or order of the points are in error for a proper
     // call to pcons!
     Error("SSDcone","The definition of the points for a call to PCONS is"
           " in error. abort.");
     return;
    } // end if
    // Poly-cone Volume A. Top part of SSD cone Carbon Fiber.
    phi   = 0.0;
    dphi  = 360.0;
    z[0]  = 0.0;
    rn[0] = RoutMin;
    rx[0] = RoutMax;
    z[1]  = z[0]+ZouterMilled - dza; // za[2] - dza.
    rn[1] = rn[0];
    rx[1] = rx[0];
    z[2]  = z[0]+ZouterMilled;//From Drawing ALR-0767 and ALR-0767/3
    rx[2] = rx[0];
    RadiusOfCurvature(Rcurv,0.,z[1],rn[1],Tc,z[3],rn[3]);
    rn[2] = RFrom2Points(rn,z,3,1,z[2]);
    RadiusOfCurvature(Rcurv,0.,z[2],rx[2],Tc,z[4],rx[4]);
    rn[4] = RminFromZpCone(rn,z,Tc,z[4]);
    rx[3] = RFrom2Points(rx,z,4,2,z[3]);
    rn[5] = RholeMax;
    z[5]  = Zfrom2Points(z,rn,4,3,rn[5]);
    rx[5] = RmaxFromZpCone(rx,z,Tc,z[5]);
    rn[6] = RholeMax;
    rx[6] = rn[6];
    z[6]  = ZFromRmaxpCone(rx,z,Tc,rx[6]);
    TGeoPcon *A = new TGeoPcon("ITS SSD Suport cone Carbon Fiber "
                       "Surface outer left",phi,dphi,7);
    for(i=0;i<A->GetNz();i++){
        A->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(A);
    //
    // Poly-cone Volume B. Stesalite inside volume A.
    // Now lets define the Inserto Stesalite 4411w material volume.
    phi   = 0.0;
    dphi  = 360.0;
    z[0]  = A->GetZ(0);
    rn[0] = A->GetRmin(0)+Cthick;
    rx[0] = A->GetRmax(0)-Cthick;
    z[1]  = A->GetZ(1);
    rn[1] = rn[0];
    rx[1] = rx[0];
    z[2]  = A->GetZ(2);
    rx[2] = rx[1];
    RadiusOfCurvature(Rcurv-Cthick,0.,z[2],rx[2],Tc,z[3],rx[3]);
    RadiusOfCurvature(Rcurv+Cthick,0.,z[1],rn[1],Tc,z[4],rn[4]);
    rn[2] = RFrom2Points(rn,z,4,1,z[2]);
    rn[3] = RFrom2Points(rn,z,4,1,z[3]);
    z[5]  = z[4]+(Thickness-2.0*Cthick)/Sintc;
    rn[5] = RmaxFromZpCone(A,Tc,z[5],-Cthick);
    rx[5] = rn[5];
    rx[4] = RFrom2Points(rx,z,5,3,z[4]);
    TGeoPcon *B = new TGeoPcon("ITS SSD Suport cone Inserto Stesalite "
                       "left edge",phi,dphi,6);
    for(i=0;i<B->GetNz();i++){
        B->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(B);
    //
    // Poly-cone Volume C. Foam inside volume A.
    // Now lets define the Rohacell foam material volume.
    phi   = 0.0;
    dphi  = 360.0;
    z[0]  = B->GetZ(4);
    rn[0] = B->GetRmin(4);
    rx[0] = rn[0];
    z[1]  = B->GetZ(5);
    rx[1] = B->GetRmin(5);
    rn[2] = A->GetRmin(5)+Cthick;//space for carbon fiber covering hole
    z[2]  = ZFromRminpCone(A,Tc,rn[2],+Cthick);
    rn[1] = RFrom2Points(rn,z,2,0,z[1]);
    rx[3] = A->GetRmin(6)+Cthick;
    rn[3] = rx[3];
    z[3]  = ZFromRmaxpCone(A,Tc,rx[3],-Cthick);
    rx[2] = RFrom2Points(rx,z,3,1,z[2]);
    TGeoPcon *C = new TGeoPcon("ITS SSD Suport cone Rohacell foam "
                       "left edge",phi,dphi,4);
    for(i=0;i<C->GetNz();i++){
        C->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(C);
    //
    // In volume SCB, th Inserto Stesalite 4411w material volume, there
    // are a number of Stainless steel screw and pin studs which will be
    // filled with screws/studs.
    rn[0] = 0.0*kmm,rx[0] = 6.0*kmm,z[0] = 0.5*10.0*kmm; // mm
    TGeoTube *D = new TGeoTube("ITS Screw+stud used to mount things to "
                       "the SSD support cone",rn[0],rx[0],z[0]);
    printTube(D);
    rn[0] = 0.0*kmm;rx[0] = 6.0*kmm;z[0] = 0.5*12.0*kmm; // mm
    TGeoTube *E = new TGeoTube("ITS pin used to mount things to the "
                       "SSD support cone",rn[0],rx[0],z[0]);
    printTube(E);
    //
    // Poly-cone Volume F. Foam in spoak reagion, inside volume A.
    // There is no carbon fiber between this upper left section and the
    // SSD spoaks. We remove it by replacing it with Rohacell foam.
    t = Cthick/(0.5*(RholeMax+RholeMin));// It is not posible to get
    // the carbon fiber thickness uniform in this phi direction. We can only
    // make it a fixed angular thickness.
    t *= 180.0/TMath::Pi();
    phi  = 12.5+t; // degrees see drawing ALR-0767.
    dphi  = 5.0 - 2.0*t; // degrees
    z[0]  = C->GetZ(2);
    rn[0] = C->GetRmin(3);
    rx[0] = rn[0];
    rn[1] = A->GetRmin(5);
    rx[1] = rn[0];
    z[1]  = ZFromRminpCone(A,Tc,rn[1],+Cthick);
    z[2]  = C->GetZ(3);
    rn[2] = rn[1];
    rx[2] = rx[1];
    rn[3] = A->GetRmin(6);
    rx[3] = rn[3];
    z[3]  = ZFromRmaxpCone(A,Tc,rx[3],-Cthick);
    TGeoPcon *F = new TGeoPcon("ITS SSD Top Suport cone Rohacell foam "
                       "Spoak",phi,dphi,4);
    for(i=0;i<F->GetNz();i++){
        F->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(F);
    //=================================================================
     // Poly-cone Volume G.
    // Now for the spoak part of the SSD cone.
    // It is not posible to inclue the radius of curvature between
    // the spoak part and the upper left part of the SSD cone or lowwer right
    // part. This would be discribed by the following curves.
    // R = Rmax - (5mm)*Sin(t) phi = phi0+(5mm*180/(Pi*RoutHole))*Sin(t) 
    // where 0<=t<=90 For the inner curve a simular equiation holds.
    phi   = 12.5; // degrees see drawing ALR-0767.
    dphi  = 5.0; // degrees
    z[0]  = A->GetZ(5);
    rn[0] = A->GetRmin(5);
    rx[0] = rn[0];
    z[1]  = A->GetZ(6);
    rn[1] = RminFromZpCone(A,Tc,z[1]);
    rx[1] = rx[0];
    rn[2] = RholeMin;
    z[2]  = ZFromRminpCone(A,Tc,rn[2]);
    rx[2] = RmaxFromZpCone(A,Tc,z[2]);
    rn[3] = rn[2];
    rx[3] = rn[3];
    z[3]  = ZFromRmaxpCone(A,Tc,rx[3]);
    TGeoPcon *G = new TGeoPcon("ITS SSD spoak carbon fiber surfaces",
                       phi,dphi,4);
    for(i=0;i<G->GetNz();i++){
        G->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(G);
    // For the foam core.
    // Poly-cone Volume H.
    t = Cthick/(0.5*(RholeMax+RholeMin));// It is not posible to get the
    // carbon fiber thickness uniform in this phi direction. We can only
    // make it a fixed angular thickness.
    t *= 180.0/TMath::Pi();
    phi   = 12.5+t; // degrees
    dphi  = 5.0 - 2.0*t; // degrees see drawing ALR-0767.
    z[0]  = F->GetZ(1);
    rn[0] = G->GetRmin(0);
    rx[0] = rn[0];
    z[1]  = F->GetZ(3);
    rn[1] = RminFromZpCone(A,Tc,z[1],+Cthick);
    rx[1] = rx[0];
    z[2]  = ZFromRminpCone(A,Tc,G->GetRmin(2),+Cthick);
    rn[2] = G->GetRmin(2);
    rx[2] = RmaxFromZpCone(A,Tc,z[2],-Cthick);
    z[3]  = ZFromRmaxpCone(A,Tc,G->GetRmin(3),-Cthick);
    rn[3] = G->GetRmin(3);
    rx[3] = rn[3];
    TGeoPcon *H = new TGeoPcon("ITS SSD support cone Rohacell foam Spoak",
                       phi,dphi,4); 
    for(i=0;i<H->GetNz();i++){
        H->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(H);
    //
    //==================================================================
    // Now for the Inner most part of the SSD cone.
    //Poly-cone Volume I.
    phi   = 0.0;
    dphi  = 360.0;
    z[0]  = G->GetZ(2);
    rn[0] = G->GetRmin(2);
    rx[0] = rn[0];
    z[1]  = G->GetZ(3);
    rn[1] = RminFromZpCone(A,Tc,z[1]);
    rx[1] = rx[0];
    rn[4] = RinMin;
    rn[5] = RinMin;
    RadiusOfCurvature(Rcurv,90.0,0.0,RinMax,90.0-Tc,Z,rx[5]); // z dummy
    z[5]  = ZFromRmaxpCone(A,Tc,rx[5]);
    z[6]  = Zcylinder;
    rn[6] = RinMin;
    z[7]  = z[6];
    rn[7] = RinCylinder;
    rn[8] = RinCylinder;
    rx[8] = rn[8];
    Rmin   = rn[5];
    RadiusOfCurvature(Rcurv,90.0-Tc,z[5],rx[5],90.0,Z,Rmax);
    Rmax   = RinMax;
    z[8]  = Z+(z[5]-Z)*(rx[8]-Rmax)/(rx[5]-Rmax);
    rx[6] = RFrom2Points(rx,z,8,5,z[6]);
    rx[7] = rx[6];
    z[3]  = Z-dZin;
    z[4]  = z[3];
    rx[3] = RmaxFromZpCone(A,Tc,z[3]);
    rx[4] = rx[3];
    //rmin dummy
    RadiusOfCurvature(Rcurv,90.,z[3],0.,90.-Tc,z[2],Rmin);
    rn[2] = RminFromZpCone(A,Tc,z[2]);
    rx[2] = RmaxFromZpCone(A,Tc,z[2]);
    // z dummy
    RadiusOfCurvature(Rcurv,90.-Tc,0.0,rn[2],90.0,Z,rn[3]);
    TGeoPcon *I = new TGeoPcon("ITS SSD lower/inner right part of SSD "
                       "cone",phi,dphi,9);
    for(i=0;i<I->GetNz();i++){
        I->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(I);
    // Now for Inserto volume at the inner most radius.
    // Poly-cone Volume K.
    phi   = 0.0;
    dphi  = 360.0;
    z[1]  = I->GetZ(3)+Cthick;
    rn[1] = I->GetRmin(3);
    z[2]  = z[1];
    rn[2] = I->GetRmin(4);
    rn[3] = rn[2];
    rn[4] = rn[2];
    rx[4] = I->GetRmax(5)-Cthick*Sintc;
    RadiusOfCurvature(Rcurv+Cthick,90.0,z[1],rn[1],90.0-Tc,z[0],rn[0]);
    rx[0] = rn[0];
    z[3]  = z[0]+(Thickness-2.0*Cthick)*Costc;;
    rx[3] = rx[0]+(Thickness-2.0*Cthick)*Sintc;
    rx[1] = RFrom2Points(rx,z,3,0,z[1]);
    rx[2] = rx[1];
    z[4]  = ZFromRmaxpCone(A,Tc,rx[4],-Cthick);
    rn[5] = rn[2];
    z[5]  = I->GetZ(6);
    rx[5] = (I->GetRmax(5)-I->GetRmax(8))/(I->GetZ(5)-I->GetZ(8))*(z[5]-z[4])+
          rx[4];
    TGeoPcon *K = new TGeoPcon("ITS SSD inner most inserto material",
                       phi,dphi,6);
    for(i=0;i<K->GetNz();i++){
        K->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(K);
    // Now for foam core at the inner most radius.
    // Poly-cone Volume J.
    phi   = 0.0;
    dphi  = 360.0;
    rn[0] = I->GetRmin(0)-Cthick;
    z[0]  = ZFromRminpCone(A,Tc,rn[0],+Cthick);
    rx[0] = rn[0];
    rx[1] = rx[0];
    z[1]  = ZFromRmaxpCone(A,Tc,rx[1],-Cthick);
    rn[1] = RminFromZpCone(A,Tc,z[1],+Cthick);
    z[2]  = K->GetZ(0);
    rn[2] = K->GetRmin(0);
    rx[2] = RmaxFromZpCone(A,Tc,z[2],-Cthick);
    z[3]  = K->GetZ(3);
    rn[3] = K->GetRmax(3);
    rx[3] = rn[3];
    TGeoPcon *J = new TGeoPcon("ITS SSD inner most foam core",phi,dphi,4); 
    for(i=0;i<J->GetNz();i++){
        J->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(J);
    // Now for foam core at the top of the inner most radius where 
    // the spoaks are.
    t = Cthick/(0.5*(RholeMax+RholeMin));// It is not posible to get the
    // carbon fiber thickness uniform in this phi direction. We can only
    // make it a fixed angular thickness.
    // Poly-cone Volume L.
    t *= 180.0/TMath::Pi();
    phi   = 12.5+t; // degrees
    dphi  = 5.0 - 2.0*t; // degrees see drawing ALR-0767.
    z[0]  = H->GetZ(2);
    rn[0] = H->GetRmin(2);
    rx[0] = rn[0];
    z[1]  = J->GetZ(0);
    rn[1] = J->GetRmin(0);
    rx[1] = I->GetRmax(1);
    z[2]  = H->GetZ(3);
    rn[2] = rn[1];
    rx[2] = rx[1];
    z[3]  = J->GetZ(1);
    rn[3] = rn[2];
    rx[3] = rn[3];
    TGeoPcon *L = new TGeoPcon("ITS SSD Bottom cone Rohacell foam Spoak",
                       phi,dphi,4);
    for(i=0;i<L->GetNz();i++){
        L->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(L);
    // Now for the SSD mounting posts
    // Poly-cone Volume O.
    dphi  = 180.0*dRpost/(RpostMin+0.5*dRpost)/TMath::Pi(); //
    phi   = Phi0Post-0.5*dphi; // degrees
    rn[0] = RpostMin+dRpost;
    rx[0] = rn[0];
    z[0]  = ZFromRmaxpCone(A,Tc,rx[0]);
    rn[1] = RpostMin;
    z[1]  = ZFromRmaxpCone(A,Tc,rn[1]);
    rx[1] = rx[0];
    z[2]  = ZpostMax;
    rn[2] = RpostMin;
    rx[2] = rn[2]+dRpost;
    TGeoPcon *O = new TGeoPcon("ITS SSD mounting post, carbon fiber",
                       phi,dphi,3);
    for(i=0;i<O->GetNz();i++){
        O->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(O);
    // Now for the SSD mounting posts
    // Poly-cone Volume P.
    t = 180.0*Cthick/(RpostMin+0.5*dRpost)/TMath::Pi();
    dphi  = O->GetDphi()-2.0*t; // degrees
    phi   = O->GetPhi1()+t; //
    rn[0] = O->GetRmin(0)-Cthick;
    rx[0] = rn[0];
    z[0]  = ZFromRmaxpCone(A,Tc,rx[0]);
    rn[1] = O->GetRmin(1)+Cthick;
    rx[1] = O->GetRmin(0)-Cthick;
    z[1]  = ZFromRmaxpCone(A,Tc,rn[1]);
    rn[2] = rn[1];
    rx[2] = rx[1];
    z[2]  = ZpostMax;
    TGeoPcon *P = new TGeoPcon("ITS SSD mounting post, Inserto",
                       phi,dphi,3);
    for(i=0;i<P->GetNz();i++){
        P->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(P);
    // This insrto continues into the SSD cone displacing the foam
    // and the carbon fiber surface at those points where the posts are.
    //Poly-cone Vol. M
    phi   = P->GetPhi1();
    dphi  = P->GetDphi();
    rn[0] = RpostMin+dRpost-Cthick;
    rx[0] = rn[0];
    z[0]  = ZFromRminpCone(A,Tc,rn[0],+Cthick);
    rx[1] = rx[0];
    z[1]  = ZFromRmaxpCone(A,Tc,rx[1],-Cthick);
    rn[1] = RminFromZpCone(A,Tc,z[1],+Cthick);
    rn[2] = RpostMin+Cthick;
    z[2]  = ZFromRminpCone(A,Tc,rn[2],+Cthick);
    rx[2] = RmaxFromZpCone(A,Tc,z[2],-Cthick);
    rn[3] = rn[2];
    rx[3] = rn[3];
    z[3]  = ZFromRmaxpCone(A,Tc,rx[3],-Cthick);
    TGeoPcon *M = new TGeoPcon("ITS SSD mounting post foam substitute, "
                       "Inserto",phi,dphi,4);
    for(i=0;i<M->GetNz();i++){
        M->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(M);
    //
    //Poly-cone Vol. N
    phi   = P->GetPhi1();
    dphi  = P->GetDphi();
    z[0]  = M->GetZ(1);
    rn[0] = M->GetRmax(1);
    rx[0] = rn[0];
    rx[1] = rx[0];
    z[1]  = ZFromRmaxpCone(A,Tc,rx[1]);
    rn[1] = RmaxFromZpCone(A,Tc,z[1],-Cthick);
    z[2]  = M->GetZ(3);
    rn[2] = M->GetRmin(3);
    rx[2] = RmaxFromZpCone(A,Tc,z[2]);
    rn[3] = rn[2];
    rx[3] = rn[3];
    z[3]  = ZFromRmaxpCone(A,Tc,rx[3]);
    TGeoPcon *N = new TGeoPcon("ITS SSD mounting post CF subsititute, "
                       "Inserto",phi,dphi,4);
    for(i=0;i<N->GetNz();i++){ 
        N->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(N);
    // Bolt heads holding the SSD-SDD tube to the SSD cone.
    // Bolt -- PolyCone
    //Poly-cone Volume Q.
    phi   = 0.0;
    dphi  = 360.0;
    z[0]  = I->GetZ(4)+ThSDDsupportPlate;
    rn[0] = 0.0;
    rx[0] = 0.5*DscrewHead;
    z[1]  = I->GetZ(4)+ThScrewHeadHole;
    rn[1] = 0.0;
    rx[1] = 0.5*DscrewHead;
    z[2]  = z[1];
    rn[2] = 0.0;
    rx[2] = 0.5*DscrewShaft;
    z[3]  = I->GetZ(6);
    rn[3] = 0.0;
    rx[3] = rx[2];
    TGeoPcon *Q = new TGeoPcon("ITS SSD Thermal sheal stainless steel "
                       "bolts",phi,dphi,4);
    for(i=0;i<Q->GetNz();i++){
        Q->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(Q);
    // air infront of bolt (stasolit Volume K) -- Tube
    z[0]  = 0.5*(ThSDDsupportPlate-Cthick);
    rn[0] = 0.0*kmm;
    rx[0] = 0.5*DscrewHead;
    TGeoTube *R = new TGeoTube("ITS Air in front of bolt (in stasolit)",
                       rn[0],rx[0],z[0]);
    // air infront of bolt (carbon fiber volume I) -- Tube
    z[0]  = 0.5*Cthick;
    rn[0] = 0.0*kmm;
    rx[0] = R->GetRmax();
    TGeoTube *S = new TGeoTube("ITS Air in front of Stainless Steal "
                       "Screw end, N6",rn[0],rx[0],z[0]);
    printTube(S);
    // SDD support plate, SSD side.
    //Poly-cone Volume T.
    dphi  = TMath::RadToDeg()*TMath::ATan2(0.5*WsddSupportPlate,
                                           RsddSupportPlate);
    phi   = Phi0SDDsupports-0.5*dphi;
    z[0]  = K->GetZ(2);
    rn[0] = I->GetRmin(4);
    rx[0] = RsddSupportPlate;
    z[1]  = I->GetZ(4) - ThSDDsupportPlate;
    rn[1] = rn[0];
    rx[1] = rx[0];
    TGeoPcon *T = new TGeoPcon("ITS SSD-SDD mounting bracket Inserto->Al.",
                       phi,dphi,2);
    for(i=0;i<T->GetNz();i++){
        T->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(T);
    //
    // Poly-cone Volume U.
    TGeoPcon *U;
    if(I->GetRmin(3)<T->GetRmax(0)){
     dphi  = T->GetDphi();
     phi   = T->GetPhi1();
     z[2]  = I->GetZ(4);
     rn[2] = T->GetRmin(0);
     rx[2] = T->GetRmax(0);
     z[3]  = K->GetZ(2);
     rn[3] = rn[2];
     rx[3] = rx[2];
     z[1]  = z[2];
     rn[1] = I->GetRmin(3);
     rx[1] = rx[3];
     rx[0] = T->GetRmax(0);
     rn[0] = rx[0];
     z[0]  = Zfrom2MinPoints(I,2,3,rn[0]);
     U = new TGeoPcon("ITS SSD-SDD mounting bracket CF->Al.",phi,dphi,4);
    }else{
     dphi  = T->GetDphi();
     phi   = T->GetPhi1();
     z[0]  = I->GetZ(4);
     rn[0] = T->GetRmin(0);
     rx[0] = T->GetRmax(0);
     z[1]  = K->GetZ(2);
     rn[1] = rn[0];
     rx[1] = rx[0];
     U = new TGeoPcon("ITS SSD-SDD mounting bracket CF->Al.",phi,dphi,2);
    }// end if
    for(i=0;i<U->GetNz();i++){
        U->DefineSection(i,z[i],rn[i],rx[i]);
    } // end for i
    printPcon(U);
    //
    TGeoVolume *Av,*Bv,*Cv,*Dv,*Ev,*Fv,*Gv,*Hv,*Iv,*Jv,*Kv,*Lv,*Mv,*Nv,
            *Ov,*Pv,*Qv,*Rv,*Sv,*Tv,*Uv;
    Av = new TGeoVolume("ITSssdConeA",A,SSDcf);
    Av->SetVisibility(kTRUE);
    Av->SetLineColor(4); // blue
    Av->SetLineWidth(1);
    Av->SetFillColor(Av->GetLineColor());
    Av->SetFillStyle(4010); // 10% transparent
    Bv = new TGeoVolume("ITSssdConeB",B,SSDfs);
    Bv->SetVisibility(kTRUE);
    Bv->SetLineColor(2); // red
    Bv->SetLineWidth(1);
    Bv->SetFillColor(Bv->GetLineColor());
    Bv->SetFillStyle(4010); // 10% transparent
    Cv = new TGeoVolume("ITSssdConeC",C,SSDfo);
    Cv->SetVisibility(kTRUE);
    Cv->SetLineColor(3); // green
    Cv->SetLineWidth(1);
    Cv->SetFillColor(Cv->GetLineColor());
    Cv->SetFillStyle(4010); // 10% transparent
    Dv = new TGeoVolume("ITSssdConeD",D,SSDss);
    Dv->SetVisibility(kTRUE);
    Dv->SetLineColor(1); // black
    Dv->SetLineWidth(1);
    Dv->SetFillColor(Dv->GetLineColor());
    Dv->SetFillStyle(4010); // 10% transparent
    Ev = new TGeoVolume("ITSssdConeE",E,SSDss);
    Ev->SetVisibility(kTRUE);
    Ev->SetLineColor(1); // black
    Ev->SetLineWidth(1);
    Ev->SetFillColor(Ev->GetLineColor());
    Ev->SetFillStyle(4010); // 10% transparent
    Fv = new TGeoVolume("ITSssdConeF",F,SSDfo);
    Fv->SetVisibility(kTRUE);
    Fv->SetLineColor(3); // green
    Fv->SetLineWidth(1);
    Fv->SetFillColor(Fv->GetLineColor());
    Fv->SetFillStyle(4010); // 10% transparent
    Gv = new TGeoVolume("ITSssdConeG",G,SSDcf);
    Gv->SetVisibility(kTRUE);
    Gv->SetLineColor(4); // blue
    Gv->SetLineWidth(2);
    Gv->SetFillColor(Gv->GetLineColor());
    Gv->SetFillStyle(4010); // 10% transparent
    Hv = new TGeoVolume("ITSssdConeH",H,SSDfo);
    Hv->SetVisibility(kTRUE);
    Hv->SetLineColor(3); // green
    Hv->SetLineWidth(1);
    Hv->SetFillColor(Hv->GetLineColor());
    Hv->SetFillStyle(4010); // 10% transparent
    Iv = new TGeoVolume("ITSssdConeI",I,SSDcf);
    Iv->SetVisibility(kTRUE);
    Iv->SetLineColor(4); // blue
    Iv->SetLineWidth(1);
    Iv->SetFillColor(Iv->GetLineColor());
    Iv->SetFillStyle(4010); // 10% transparent
    Jv = new TGeoVolume("ITSssdConeJ",J,SSDfo);
    Jv->SetVisibility(kTRUE);
    Jv->SetLineColor(3); // green
    Jv->SetLineWidth(3);
    Jv->SetFillColor(Jv->GetLineColor());
    Jv->SetFillStyle(4010); // 10% transparent
    Kv = new TGeoVolume("ITSssdConeK",K,SSDfs);
    Kv->SetVisibility(kTRUE);
    Kv->SetLineColor(2); // red
    Kv->SetLineWidth(1);
    Kv->SetFillColor(Kv->GetLineColor());
    Kv->SetFillStyle(4010); // 10% transparent
    Lv = new TGeoVolume("ITSssdConeL",L,SSDfo);
    Lv->SetVisibility(kTRUE);
    Lv->SetLineColor(3); // green
    Lv->SetLineWidth(3);
    Lv->SetFillColor(Lv->GetLineColor());
    Lv->SetFillStyle(4010); // 10% transparent
    Mv = new TGeoVolume("ITSssdConeM",M,SSDfs);
    Mv->SetVisibility(kTRUE);
    Mv->SetLineColor(2); // red
    Mv->SetLineWidth(1);
    Mv->SetFillColor(Mv->GetLineColor());
    Mv->SetFillStyle(4010); // 10% transparent
    Nv = new TGeoVolume("ITSssdConeN",N,SSDfs);
    Nv->SetVisibility(kTRUE);
    Nv->SetLineColor(2); // red
    Nv->SetLineWidth(1);
    Nv->SetFillColor(Nv->GetLineColor());
    Nv->SetFillStyle(4010); // 10% transparent
    Ov = new TGeoVolume("ITSssdConeO",O,SSDcf);
    Ov->SetVisibility(kTRUE);
    Ov->SetLineColor(4); // blue
    Ov->SetLineWidth(1);
    Ov->SetFillColor(Iv->GetLineColor());
    Ov->SetFillStyle(4010); // 10% transparent
    Pv = new TGeoVolume("ITSssdConeP",P,SSDfs);
    Pv->SetVisibility(kTRUE);
    Pv->SetLineColor(2); // red
    Pv->SetLineWidth(1);
    Pv->SetFillColor(Pv->GetLineColor());
    Pv->SetFillStyle(4010); // 10% transparent
    Qv = new TGeoVolume("ITSssdConeQ",Q,SSDss);
    Qv->SetVisibility(kTRUE);
    Qv->SetLineColor(1); // black
    Qv->SetLineWidth(1);
    Qv->SetFillColor(Qv->GetLineColor());
    Qv->SetFillStyle(4010); // 10% transparent
    Rv = new TGeoVolume("ITSssdConeR",R,SSDair);
    Rv->SetVisibility(kTRUE);
    Rv->SetLineColor(5); // yellow
    Rv->SetLineWidth(1);
    Rv->SetFillColor(Rv->GetLineColor());
    Rv->SetFillStyle(4010); // 10% transparent
    Sv = new TGeoVolume("ITSssdConeS",S,SSDair);
    Sv->SetVisibility(kTRUE);
    Sv->SetLineColor(5); // yellow
    Sv->SetLineWidth(1);
    Sv->SetFillColor(Sv->GetLineColor());
    Sv->SetFillStyle(4010); // 10% transparent
    Tv = new TGeoVolume("ITSssdConeT",T,SSDal);
    Tv->SetVisibility(kTRUE);
    Tv->SetLineColor(17); // gray
    Tv->SetLineWidth(1);
    Tv->SetFillColor(Tv->GetLineColor());
    Tv->SetFillStyle(4010); // 10% transparent
    Uv = new TGeoVolume("ITSssdConeU",U,SSDal);
    Uv->SetVisibility(kTRUE);
    Uv->SetLineColor(17); // gray
    Uv->SetLineWidth(1);
    Uv->SetFillColor(Uv->GetLineColor());
    Uv->SetFillStyle(4010); // 10% transparent
    //
    TGeoTranslation *tran = new TGeoTranslation("ITSssdConeTrans",0.0,0.0,-Z0);
    TGeoRotation *rot180  = new TGeoRotation("",0.0,180.0,0.0);
    TGeoCombiTrans *flip  = new TGeoCombiTrans("ITSssdConeFlip",0.0,0.0,Z0,rot180);
    delete rot180;// rot not explicity used in AddNode functions.
    TGeoTranslation *tranR,*tranS;
    TGeoCombiTrans *fliptran,*rottran;
    TGeoRotation *rot,*zspoaks,*zspoaks180;
    Int_t NcD=1,NcE=1,NcQ=1,NcR=1,NcS=1,NcT=1,NcU=1;
    Av->AddNode(Bv,1,0);
    Av->AddNode(Cv,1,0);
    Moth->AddNode(Av,1,tran); // RB24 side
    Moth->AddNode(Av,2,flip); // RB26 side (Absorber)
    Moth->AddNode(Iv,1,tran); // RB24 side
    Moth->AddNode(Iv,2,flip); // RB26 side (Absorber)
    Gv->AddNode(Hv,1,0);
    for(i=0;i<Nspoaks;i++){ // SSD Cone Spoaks
     zspoaks = new TGeoRotation("",0.0,0.0,
                       ((Double_t)i*360.)/((Double_t)Nspoaks));
     rottran = new TGeoCombiTrans("",0.0,0.0,-Z0,zspoaks);
     Moth->AddNode(Gv,i+1,rottran); // RB24 side
     Av->AddNode(Fv,i+1,zspoaks);
     Iv->AddNode(Lv,i+1,zspoaks);
     zspoaks180 =  new TGeoRotation("",0.0,180.0,
                           ((Double_t)i*360.)/((Double_t)Nspoaks));
     fliptran = new TGeoCombiTrans("",0.0,0.0,Z0,zspoaks180);
     delete zspoaks180;// rot not explicity used in AddNode functions.
     Moth->AddNode(Gv,Nspoaks+i+1,fliptran); // RB26 side
    } // end for i
    Iv->AddNode(Jv,1,0);
    Iv->AddNode(Kv,1,0);
    Ov->AddNode(Pv,1,0);
    t0 = (P->GetPhi1()+0.5*P->GetDphi())*kRadian;
    t  = (0.25* P->GetDphi())*kRadian;
    z[0] = 0.5*(P->GetRmin(2)+P->GetRmax(2))+
          0.25*(P->GetRmax(2)-P->GetRmin(2));
    x = z[0]*TMath::Cos(t0+t);
    y = z[0]*TMath::Sin(t0+t);
    tran = new TGeoTranslation("",x,y,P->GetZ(2)-Q->GetZ(3));
    Pv->AddNode(Qv,NcQ++,tran); // Screw head
    z[0] = 0.5*(P->GetRmin(2)+P->GetRmax(2))-
          0.25*(P->GetRmax(2)-P->GetRmin(2));
    x = z[0]*TMath::Cos(t0-t);
    y = z[0]*TMath::Sin(t0-t);
    tran = new TGeoTranslation("",x,y,P->GetZ(2)-Q->GetZ(3));
    Pv->AddNode(Qv,NcQ++,tran); // Screw head
    //Pv->AddNode(Vv,1,?); // Air hole in Posts
    //Pv->AddNode(Vv,2,?); // Air hole in Posts
    //Mv->AddNode(Wv,1,?); // Air hole in Posts
    //Mv->AddNode(Wv,2,?); // Air hole in Posts
    //Nv->AddNode(Xv,1,?); // Air hole in Posts
    //Nv->AddNode(Xv,2,?); // Air hole in Posts
    TGeoRotation *zposts,*zposts180;
    for(i=0;i<Nposts;i++){ // SSD Cone mounting posts
        zposts = new TGeoRotation("",0.0,0.0,
                                  ((Double_t)i*360.)/((Double_t)Nposts));
        rottran = new TGeoCombiTrans("",0.0,0.0,-Z0,zposts);
        Moth->AddNode(Ov,i+1,rottran); // RB24 side
        Jv->AddNode(Mv,i+1,zposts);
        Iv->AddNode(Nv,i+1,zposts);
        //Jv->AddNode(Xv,2*i+3,?); // Air hole in Posts
        //Jv->AddNode(Xv,2*i+4,?); // Air hole in Posts
        zposts180 = new TGeoRotation("",0.0,180.0,
                                     ((Double_t)i*360.)/((Double_t)Nposts));
        fliptran = new TGeoCombiTrans("",0.0,0.0,Z0,zposts180);
        delete zposts180;// rot not explicity used in AddNode functions.
        Moth->AddNode(Ov,Nposts+i+1,fliptran); // RB26 side
    } // end for i
    //
    for(i=0;i<NinScrews;i++){
        t = (Phi0Screws+360.*((Double_t)i)/((Double_t)NinScrews))*kRadian;
        tran= new TGeoTranslation("",RcylinderScrews*TMath::Cos(t),
                                  RcylinderScrews*TMath::Sin(t),0.0);
        Kv->AddNode(Qv,NcQ++,tran);
        tran = new TGeoTranslation("",RcylinderScrews*TMath::Cos(t),
                                  RcylinderScrews*TMath::Sin(t),
                                   CB->GetZ(0)+D->GetDz());
        CBv->AddNode(Dv,NcD++,tran);
        tran = new TGeoTranslation("",RcylinderScrews*TMath::Cos(t),
                                  RcylinderScrews*TMath::Sin(t),
                                   CB->GetZ(5)-D->GetDz());
        CBv->AddNode(Dv,NcD++,tran);
        if(/*not where volumes U and T are*/kTRUE){
            tranR = new TGeoTranslation("",RcylinderScrews*TMath::Cos(t),
                                        RcylinderScrews*TMath::Sin(t),
                                        K->GetZ(2)+R->GetDz());
            tranS = new TGeoTranslation("",RcylinderScrews*TMath::Cos(t),
                                        RcylinderScrews*TMath::Sin(t),
                                        I->GetZ(4)+S->GetDz());
            Kv->AddNode(Rv,NcR++,tranR);
            Iv->AddNode(Sv,NcS++,tranS);
        } // end if
    } // end for i
    const Int_t Nbscrew=2,Nbpins=3,Nrailsc=4,Nrailp=2;
    Double_t da[] = {-3.5,-1.5,1.5,3.5};
    for(i=0;i<2;i++){ // Mounting for ITS-TPC bracket or ITS-Rails
        t0 = 180.*((Double_t)i)*kRadian;
        for(j=-Nbscrew/2;j<=Nbscrew/2;j++)if(j!=0){//screws per ITS-TPC bracket
            t = t0 + 5.0*((Double_t)j)*kRadian;
            tran = new TGeoTranslation("",RoutHole*TMath::Cos(t),
                                       RoutHole*TMath::Sin(t),
                                       B->GetZ(0)+D->GetDz());
            Bv->AddNode(Dv,NcD++,tran);
        } // end or j
        for(j=-Nbpins/2;j<=Nbpins/2;j++){ // pins per ITS-TPC bracket
            t = t0 + 3.0*((Double_t)j)*kRadian;
            tran = new TGeoTranslation("",RoutHole*TMath::Cos(t),
                                       RoutHole*TMath::Sin(t),
                                       B->GetZ(0)+D->GetDz());
            Bv->AddNode(Ev,NcE++,tran);
        } // end or j
        t0 = (96.5+187.*((Double_t)i))*kRadian;
        for(j=0;j<Nrailsc;j++){ // screws per ITS-rail bracket
            t = t0+da[j]*kRadian;
            tran = new TGeoTranslation("",RoutHole*TMath::Cos(t),
                                       RoutHole*TMath::Sin(t),
                                       B->GetZ(0)+D->GetDz());
            Bv->AddNode(Dv,NcD++,tran);
        } // end or j
        t0 = (91.5+184.*((Double_t)i))*kRadian;
        for(j=-Nrailp/2;j<=Nrailp/2;j++)if(j!=0){ // pins per ITS-rail bracket
            t = t0+(7.0*((Double_t)j))*kRadian;
            tran = new TGeoTranslation("",RoutHole*TMath::Cos(t),
                                       RoutHole*TMath::Sin(t),
                                       B->GetZ(0)+D->GetDz());
            Bv->AddNode(Ev,NcE++,tran);
        } // end or j
    } // end for i
    for(i=0;i<Nmounts;i++){ // mounting points for SPD-cone+Beam-pipe support
        t0 = (45.0+((Double_t)i)*360./((Double_t)Nmounts))*kRadian;
        for(j=-1;j<=1;j++)if(j!=0){ // 2 screws per bracket
            t = t0+((Double_t)j)*0.5*DmountAngle*kRadian;
            tran = new TGeoTranslation("",RoutHole*TMath::Cos(t),
                                       RoutHole*TMath::Sin(t),
                                       B->GetZ(0)+D->GetDz());
            Bv->AddNode(Dv,NcD++,tran);
        } // end for j
        for(j=0;j<1;j++){ // 1 pin per bracket
            t = t0;
            tran = new TGeoTranslation("",RoutHole*TMath::Cos(t),
                                       RoutHole*TMath::Sin(t),
                                       B->GetZ(0)+D->GetDz());
            Bv->AddNode(Ev,NcE++,tran);
        } // end for j
    } // end for i
    t = (T->GetPhi1()+0.5*T->GetDphi())*kRadian;
    tran = new TGeoTranslation("",RinHole*TMath::Cos(t),RinHole*TMath::Sin(t),
                               T->GetZ(T->GetNz()-1)+R->GetDz());
    Tv->AddNode(Rv,NcR++,tran);
    t = (U->GetPhi1()+0.5*U->GetDphi())*kRadian;
    tran = new TGeoTranslation("",RinHole*TMath::Cos(t),RinHole*TMath::Sin(t),
                               U->GetZ(U->GetNz()-1)+S->GetDz());
    Uv->AddNode(Sv,NcS++,tran);
    for(i=0;i<NssdSupports;i++){ // mounting braclets for SSD/SDD 
        t0 = ((Double_t)i*360./((Double_t)NssdSupports));
        rot = new TGeoRotation("",0.0,0.0,t0);
        Kv->AddNode(Tv,NcT++,rot);
        Iv->AddNode(Uv,NcU++,rot);
        for(j=0;j<2;j++)if(j!=0){ // 2 pin per bracket
            t = t0 + ((Double_t)j)*0.5*DssdsddBracketAngle;
            tran = new TGeoTranslation("",RinHole*TMath::Cos(t),
                                       RinHole*TMath::Sin(t),
                                       T->GetZ(T->GetNz()-1)-E->GetDz());
            Kv->AddNode(Ev,NcE++,tran);
        } // end for j
    } // end for i
    if(GetDebug()){
        Av->PrintNodes();
        Bv->PrintNodes();
        Cv->PrintNodes();
        Dv->PrintNodes();
        Ev->PrintNodes();
        Fv->PrintNodes();
        Gv->PrintNodes();
        Hv->PrintNodes();
        Iv->PrintNodes();
        Jv->PrintNodes();
        Kv->PrintNodes();
        Lv->PrintNodes();
        Mv->PrintNodes();
        Nv->PrintNodes();
        Ov->PrintNodes();
        Pv->PrintNodes();
        Qv->PrintNodes();
        Rv->PrintNodes();
        Sv->PrintNodes();
        Tv->PrintNodes();
        Uv->PrintNodes();
    } // end if
}
//______________________________________________________________________
void AliITSv11::CreateMaterials(){
    // Create ITS materials
    //     This function defines the default materials used in the Geant
    // Monte Carlo simulations for the geometries AliITSv11.
    // In general it is automatically replaced by
    // Inputs:
    //   none.
    // Outputs:
    //   none.
    // Return
    //   none.

    //TGeoMaterial *C  = new TGeoMaterial("ITSCarbon",12.0,6.0,2.265);
    TGeoMaterial *Al = new TGeoMaterial("ITSAluminum",26.981539,13.0,2.07);
    TGeoMixture *Cfiber = new TGeoMixture("ITSCarbonFiber",6,1.930);
    TGeoMixture *Rohacell = new TGeoMixture("ITSRohacell",6,1.930);
    TGeoMixture *Staselite = new TGeoMixture("ITSStaselite4411w",6,1.930);
    TGeoMixture *Air = new TGeoMixture("ITSAir",6,1.205*1.E-3);
    TGeoMixture *Stainless = new TGeoMixture("ITSStainless",6,1.930);
    //
    Double_t SPDcone[20];
    SPDcone[0] = 1.0; // imat
    SPDcone[1] = 0.0; // isvol
    SPDcone[2] = gAlice->Field()->Integ(); // ifield
    SPDcone[3] = gAlice->Field()->Max(); // fieldm
    SPDcone[4] = 1.0; // tmaxfd [degrees]
    SPDcone[5] = 1.0; // stemax [cm]
    SPDcone[6] = 0.5; // deemax [fraction]
    SPDcone[7] = 1.0E-3; // epsil [cm]
    SPDcone[8] = 0.0; // stmin [cm]
    new TGeoMedium("ITSspdCarbonFiber",1,Cfiber,SPDcone);
    SPDcone[0] += 1.0;
    new TGeoMedium("ITSspdStaselite4411w",2,Staselite,SPDcone);
    SPDcone[0] += 1.0;
    new TGeoMedium("ITSspdRohacell50A",3,Rohacell,SPDcone);
    SPDcone[0] += 1.0;
    new TGeoMedium("ITSspdStainlesSteal",4,Stainless,SPDcone);
    SPDcone[0] += 1.0;
    new TGeoMedium("ITSspdAir",5,Air,SPDcone);
    SPDcone[0] += 1.0;
    new TGeoMedium("ITSspdAl",6,Al,SPDcone);
    //
    Double_t SSDcone[20];
    SSDcone[0] = 1.0; // imat
    SSDcone[1] = 0.0; // isvol
    SSDcone[2] = gAlice->Field()->Integ(); // ifield
    SSDcone[3] = gAlice->Field()->Max(); // fieldm
    SSDcone[4] = 1.0; // tmaxfd [degrees]
    SSDcone[5] = 1.0; // stemax [cm]
    SSDcone[6] = 0.5; // deemax [fraction]
    SSDcone[7] = 1.0E-3; // epsil [cm]
    SSDcone[8] = 0.0; // stmin [cm]
    new TGeoMedium("ITSssdCarbonFiber",1,Cfiber,SSDcone);
    SSDcone[0] += 1.0;
    new TGeoMedium("ITSssdStaselite4411w",2,Staselite,SSDcone);
    SSDcone[0] += 1.0;
    new TGeoMedium("ITSssdRohacell50A",3,Rohacell,SSDcone);
    SSDcone[0] += 1.0;
    new TGeoMedium("ITSssdStainlesSteal",4,Stainless,SSDcone);
    SSDcone[0] += 1.0;
    new TGeoMedium("ITSssdAir",5,Air,SSDcone);
    SSDcone[0] += 1.0;
    new TGeoMedium("ITSssdAl",6,Al,SSDcone);
}
//______________________________________________________________________
void AliITSv11::ServicesCableSupport(TGeoVolume *Moth){
    // Define the detail ITS cable support trays on both the RB24 and 
    // RB26 sides..
    // Inputs:
    //   none.
    // Outputs:
    //  none.
    // Return:
    //  none.
    // Based on the Drawings SSup_201A.jpg unless otherwise stated, 
    // Volumes A..., 
    TGeoMedium *SUPcf    = 0; // SUP support cone Carbon Fiber materal number.
    TGeoMedium *SUPfs    = 0; // SUP support cone inserto stesalite 4411w.
    TGeoMedium *SUPfo    = 0; // SUP support cone foam, Rohacell 50A.
    TGeoMedium *SUPss    = 0; // SUP support cone screw material,Stainless
    TGeoMedium *SUPair   = 0; // SUP support cone Air
    TGeoMedium *SUPal    = 0; // SUP support cone SDD mounting bracket Al
    TGeoMedium *SUPwater = 0; // SUP support cone Water
    TGeoManager *mgr = gGeoManager;
    SUPcf    = mgr->GetMedium("ITSssdCarbonFiber");
    SUPfs    = mgr->GetMedium("ITSssdStaselite4411w");
    SUPfo    = mgr->GetMedium("ITSssdRohacell50A");
    SUPss    = mgr->GetMedium("ITSssdStainlessSteal");
    SUPair   = mgr->GetMedium("ITSssdAir");
    SUPal    = mgr->GetMedium("ITSssdAl");
    SUPwater = mgr->GetMedium("ITSssdWater");
    //
    Int_t i,j;
    Double_t x,y,z,t,t0,dt,di,r;

    // RB 24 side
    const Double_t Z024         = 900*kmm;//SSup_203A.jpg
    const Double_t ThssFrame24  = 5.0*kmm;
    const Double_t RssFrame24   = 444.5*kmm-ThssFrame24; // SSup_204A.jpg
    const Double_t WidthFrame24 = 10.0*kmm;
    const Double_t HightFrame24 = 10.0*kmm;
    const Double_t Phi0Frame24  = 15.2*kDegree; // SSup_602A.jpg
    const Double_t Phi1Frame24  = (90.0-7.6)*kDegree; // SSup_802A.jpg
    const Double_t ZssFrameSection24 = (415.0-10.0)*kmm;
    const Int_t    NZsections24      = 4;
    const Int_t    NPhiSections24    = 4;
    const Int_t    NFramesPhi24      = 4;
    TGeoTubeSeg *A24,*M24; // Cylinderial support structure
    TGeoBBox    *B24; // Cylinderial support structure

    M24 = new TGeoTubeSeg("ITS sup Cable tray support frame mother volume M24",
                          RssFrame24,RssFrame24+ThssFrame24,
                          0.5*(4.*ZssFrameSection24+5*WidthFrame24),
                          Phi0Frame24,Phi1Frame24);
    A24 = new TGeoTubeSeg("ITS sup Cable tray support frame radial section A24",
                          RssFrame24,RssFrame24+ThssFrame24,0.5*WidthFrame24,
                          Phi0Frame24,Phi1Frame24);
    B24 = new TGeoBBox("ITS sup Cable tray support frame Z section B24",
                       0.5*ThssFrame24,0.5*HightFrame24,0.5*ZssFrameSection24);
    printTubeSeg(A24);
    printTubeSeg(M24);
    printBBox(B24);
    TGeoVolume *A24v,*B24v,*M24v;
    TGeoTranslation *tran;
    TGeoRotation    *rot;
    TGeoCombiTrans  *tranrot;
    //
    A24v = new TGeoVolume("ITSsupFrameA24",A24,SUPss);
    A24v->SetVisibility(kTRUE);
    A24v->SetLineColor(1); // black
    A24v->SetLineWidth(1);
    A24v->SetFillColor(A24v->GetLineColor());
    A24v->SetFillStyle(4000); // 0% transparent
    B24v = new TGeoVolume("ITSsupFrameB24",B24,SUPss);
    B24v->SetVisibility(kTRUE);
    B24v->SetLineColor(1); // black
    B24v->SetLineWidth(1);
    B24v->SetFillColor(B24v->GetLineColor());
    B24v->SetFillStyle(4000); // 0% transparent
    M24v = new TGeoVolume("ITSsupFrameM24",M24,SUPair);
    M24v->SetVisibility(kTRUE);
    M24v->SetLineColor(7); // light blue
    M24v->SetLineWidth(1);
    M24v->SetFillColor(M24v->GetLineColor());
    M24v->SetFillStyle(4090); // 90% transparent
    //
    Int_t NcA24=1,NcB24=1;
    t0 = Phi0Frame24;
    dt = (Phi1Frame24-Phi0Frame24)/((Double_t)NPhiSections24);
    for(i=0;i<=NZsections24;i++){
        di = (Double_t) i;
        z = -M24->GetDz()+A24->GetDz() + di*(ZssFrameSection24+WidthFrame24);
        tran = new TGeoTranslation("",0.0,0.0,z);
        M24v->AddNode(A24v,NcA24++,tran);
        r = RssFrame24+B24->GetDX();
        z = z + A24->GetDz()+B24->GetDZ();
       if(i<NZsections24) for(j=0;j<=NPhiSections24;j++){
            t = t0 + ((Double_t)j)*dt;
            rot = new TGeoRotation("",0.0,0.0,t);
            y = r*TMath::Sin(t*kRadian);
            x = r*TMath::Cos(t*kRadian);
            tranrot = new TGeoCombiTrans("",x,y,z,rot);
            delete rot;// rot not explicity used in AddNode functions.
            M24v->AddNode(B24v,NcB24++,tranrot);
        } // end for j
    } // end for i
    tran = new TGeoTranslation("",0.0,0.0,Z024+M24->GetDz());
    Moth->AddNode(M24v,1,tran);
    for(i=1;i<NFramesPhi24;i++){
        di = (Double_t) i;
        rot = new TGeoRotation("",0.0,0.0,90.0*di);
        tranrot = new TGeoCombiTrans("",0.0,0.0,Z024+M24->GetDz(),rot);
        delete rot;// rot not explicity used in AddNode functions.
        Moth->AddNode(M24v,i+1,tranrot);
    } // end for i
    if(GetDebug()){
        A24v->PrintNodes();
        B24v->PrintNodes();
        M24v->PrintNodes();
    } // end if
    // Cable support tray 
    // Material is Aluminum
    const Double_t RS24in     = TMath::Max(RssFrame24,444.5*kmm);
                                           // SSup_204A & SSup_206A
    const Double_t RS24Airout = 459.5*kmm; // SSup_204A & SSup_206A
    const Double_t RS24out    = 494.5*kmm; // SSup_206A & SSup_204A
    const Double_t RS24PPout  = 550.0*kmm; // SSup_206A
    const Double_t LS24PP     = 350.0*kmm; // SSup_202A
    const Double_t LS24       = (2693.0-900.0)*kmm; //SSup_205A & SSup_207A
    const Double_t ThS24wall  = 1.0*kmm; // SSup_209A & SSup_210A
    const Double_t WbS24      = 42.0*kmm; // SSup_209A & SSup_210A
    const Double_t WtS24      = 46.9*kmm; // SSup_209A & SSup_210A
    const Double_t WcapS24    = 50.0*kmm; // SSup_209A & SSup_210A
    const Double_t WdS24      = 41.0*kmm; // SSup_209A ? should be 41.46938776
    const Double_t HS24       = 50.0*kmm; // SSup_209A & SSup_210A
    const Double_t OutDcoolTub= 12.0*kmm; // SSup_209A
    const Double_t InDcoolTub = 10.0*kmm; // SSup_209A
    const Double_t BlkNozInDS24= 6.0*kmm; // SSup_209A
    // The following are deduced or guessed at
    const Double_t LtopLipS24 = 6.0*kmm; // Guessed at.
    const Double_t LdLipS24   = 6.0*kmm; // Guessed at.
    const Double_t HdS24      = OutDcoolTub; //
    const Double_t BlkNozZS24 = 6.0*kmm; // Guessed at.
    // Simplifided exterior shape. The side wall size is 2.5*thicker than
    // it should be (due to simplification).
    TGeoArb8 *C24 = new TGeoArb8("ITS Sup Cable Tray Element C24",0.5*LS24);
    C24->SetVertex(0,-0.5*WcapS24,HS24+ThS24wall);
    C24->SetVertex(1,+0.5*WcapS24,HS24+ThS24wall);
    C24->SetVertex(2,+0.5*WbS24,0.0);
    C24->SetVertex(3,-0.5*WbS24,0.0);
    C24->SetVertex(4,-0.5*WcapS24,HS24+ThS24wall);
    C24->SetVertex(5,+0.5*WcapS24,HS24+ThS24wall);
    C24->SetVertex(6,+0.5*WbS24,0.0);
    C24->SetVertex(7,-0.5*WbS24,0.0);
    TGeoArb8 *D24 = new TGeoArb8("ITS Sup Cable Tray lower Element D24",
                                 0.5*LS24);
    // Because of question about the value of WdS24, compute what it
    // should be assuming cooling tube fixes hight of volume.
    x = OutDcoolTub*(0.5*WcapS24-0.5*WbS24-ThS24wall)/(HS24-ThS24wall);
    D24->SetVertex(0,-x,OutDcoolTub+ThS24wall);
    D24->SetVertex(1,+x,OutDcoolTub+ThS24wall);
    D24->SetVertex(2,+0.5*WbS24-ThS24wall,ThS24wall);
    D24->SetVertex(3,-0.5*WbS24+ThS24wall,ThS24wall);
    D24->SetVertex(4,-x,OutDcoolTub+ThS24wall);
    D24->SetVertex(5,+x,OutDcoolTub+ThS24wall);
    D24->SetVertex(6,+0.5*WbS24-ThS24wall,ThS24wall);
    D24->SetVertex(7,-0.5*WbS24+ThS24wall,ThS24wall);
    TGeoTube *E24 = new TGeoTube("ITS Sup Cooling Tube E24",0.5*InDcoolTub,
                                 0.5*OutDcoolTub,0.5*LS24-BlkNozZS24);
    TGeoArb8 *F24 = new TGeoArb8("ITS Sup Cable Tray lower Element block F24",
                                 0.5*BlkNozZS24);
    for(i=0;i<8;i++) F24->SetVertex(i,D24->GetVertices()[i*2+0],
                                      D24->GetVertices()[i*2+1]); //
    TGeoTube *G24 = new TGeoTube("ITS Sup Cooling Tube hole in block G24",
                                 0.0,0.5*BlkNozInDS24,0.5*BlkNozZS24);
    TGeoArb8 *H24 = new TGeoArb8("ITS Sup Cable Tray upper Element H24",
                                 0.5*(LS24- LS24PP));
    H24->SetVertex(0,C24->GetVertices()[0*2+0]+2.*ThS24wall,
                     C24->GetVertices()[0*2+1]-ThS24wall);
    H24->SetVertex(1,C24->GetVertices()[1*2+0]-2.*ThS24wall,
                     C24->GetVertices()[1*2+1]-ThS24wall);
    H24->SetVertex(2,D24->GetVertices()[1*2+0]-ThS24wall,
                     D24->GetVertices()[1*2+1]+ThS24wall);
    H24->SetVertex(3,D24->GetVertices()[0*2+0]+ThS24wall,
                     D24->GetVertices()[0*2+1]+ThS24wall);
    for(i=4;i<8;i++) H24->SetVertex(i,H24->GetVertices()[(i-4)*2+0],
                                      H24->GetVertices()[(i-4)*2+1]); //
    printArb8(C24);
    printArb8(D24);
    printTube(E24);
    printArb8(F24);
    printTube(G24);
    printArb8(H24);
    TGeoVolume *C24v,*D24v,*E24v,*F24v,*Ga24v,*Gw24v,*Gf24v,*H24v;
    //
    C24v = new TGeoVolume("ITSsupCableTrayC24",C24,SUPal);
    C24v->SetVisibility(kTRUE);
    C24v->SetLineColor(6); //
    C24v->SetLineWidth(1);
    C24v->SetFillColor(C24v->GetLineColor());
    C24v->SetFillStyle(4000); // 0% transparent
    D24v = new TGeoVolume("ITSsupCableTrayLowerD24",D24,SUPair);
    D24v->SetVisibility(kTRUE);
    D24v->SetLineColor(6); //
    D24v->SetLineWidth(1);
    D24v->SetFillColor(D24v->GetLineColor());
    D24v->SetFillStyle(4000); // 0% transparent
    E24v = new TGeoVolume("ITSsupCableTrayCoolTubeE24",E24,SUPss);
    E24v->SetVisibility(kTRUE);
    E24v->SetLineColor(6); //
    E24v->SetLineWidth(1);
    E24v->SetFillColor(E24v->GetLineColor());
    E24v->SetFillStyle(4000); // 0% transparent
    F24v = new TGeoVolume("ITSsupCableTrayBlockF24",F24,SUPal);
    F24v->SetVisibility(kTRUE);
    F24v->SetLineColor(6); //
    F24v->SetLineWidth(1);
    F24v->SetFillColor(F24v->GetLineColor());
    F24v->SetFillStyle(4000); // 0% transparent
    Gw24v = new TGeoVolume("ITSsupCableTrayCoolantWaterG24",G24,SUPwater);
    Gw24v->SetVisibility(kTRUE);
    Gw24v->SetLineColor(6); //
    Gw24v->SetLineWidth(1);
    Gw24v->SetFillColor(Gw24v->GetLineColor());
    Gw24v->SetFillStyle(4000); // 0% transparent
    Ga24v = new TGeoVolume("ITSsupCableTrayCoolantAirG24",G24,SUPair);
    Ga24v->SetVisibility(kTRUE);
    Ga24v->SetLineColor(6); //
    Ga24v->SetLineWidth(1);
    Ga24v->SetFillColor(Ga24v->GetLineColor());
    Ga24v->SetFillStyle(4000); // 0% transparent
    H24v = new TGeoVolume("ITSsupCableTrayUpperC24",H24,SUPair);
    H24v->SetVisibility(kTRUE);
    H24v->SetLineColor(6); //
    H24v->SetLineWidth(1);
    H24v->SetFillColor(H24v->GetLineColor());
    H24v->SetFillStyle(4000); // 0% transparent
    //
    tran = new TGeoTranslation("",-OutDcoolTub,OutDcoolTub+ThS24wall,0.0);
    F24v->AddNode(Gw24v,1,tran);
    D24v->AddNode(E24v,1,tran);
    tran = new TGeoTranslation("",0.0,OutDcoolTub+ThS24wall,0.0);
    F24v->AddNode(Gw24v,2,tran);
    D24v->AddNode(E24v,2,tran);
    tran = new TGeoTranslation("",+OutDcoolTub,OutDcoolTub+ThS24wall,0.0);
    F24v->AddNode(Gw24v,3,tran);
    D24v->AddNode(E24v,3,tran);
    tran = new TGeoTranslation("",0.0,0.0,0.5*LS24-0.5*BlkNozZS24);
    D24v->AddNode(F24v,1,tran);
    tran = new TGeoTranslation("",0.0,0.0,-(0.5*LS24-0.5*BlkNozZS24));
    D24v->AddNode(F24v,2,tran);
    C24v->AddNode(D24v,1,0);
    C24v->AddNode(H24v,1,0);
    //==================================================================
    //
    // RB 26 side
    const Double_t Z026         = -900*kmm;//SSup_203A.jpg
    const Double_t ThssFrame26  = 5.0*kmm;
    const Double_t R0ssFrame26  = 444.5*kmm-ThssFrame26; // SSup_204A.jpg
    const Double_t R1ssFrame26  = 601.6*kmm-ThssFrame26; // SSup_208A.jpg
    const Double_t WidthFrame26 = 10.0*kmm;
    //const Double_t HightFrame26 = 10.0*kmm;
    const Double_t Phi0Frame26  = 15.2*kDegree; // SSup_602A.jpg
    const Double_t Phi1Frame26  = (90.0-7.6)*kDegree; // SSup_802A.jpg
    const Double_t ZssFrameSection26 = (415.0-10.0)*kmm;
    const Int_t    NZsections26      = 4;
    const Int_t    NPhiSections26    = 4;
    const Int_t    NFramesPhi26      = 4;
    TGeoConeSeg *A26[NZsections26+1],*M26; // Cylinderial support structure
    TGeoArb8     *B26; // Cylinderial support structure
    Char_t name[100];
    Double_t r1,r2,m;

    M26 = new TGeoConeSeg("ITS sup Cable tray support frame mother volume M26",
                         0.5*(4.*ZssFrameSection26+5*WidthFrame26),
                          R1ssFrame26,R1ssFrame26+ThssFrame26,
                          R0ssFrame26,R0ssFrame26+ThssFrame26,
                          Phi0Frame26,Phi1Frame26);
    m = -((R1ssFrame26-R0ssFrame26)/
         (((Double_t)NZsections26)*(ZssFrameSection26+WidthFrame26)));
    for(i=0;i<NZsections26+1;i++){
        di = ((Double_t) i)*(ZssFrameSection26+WidthFrame26);
        sprintf(name,
                "ITS sup Cable tray support frame radial section A26[%d]",i);
        r1 = R1ssFrame26+m*di;
        r2 = R1ssFrame26+m*(di+WidthFrame26);
        A26[i] = new TGeoConeSeg(name,0.5*WidthFrame26,r2,r2+ThssFrame26,
                                 r1,r1+ThssFrame26,Phi0Frame26,Phi1Frame26);
    } // end for i
    B26 = new TGeoArb8("ITS sup Cable tray support frame Z section B26",
                       0.5*ZssFrameSection26);
    r = 0.25*(A26[0]->GetRmax1()+A26[0]->GetRmin1()+
              A26[1]->GetRmax2()+A26[1]->GetRmin2());
    B26->SetVertex(0,A26[0]->GetRmax2()-r,+0.5*WidthFrame26);
    B26->SetVertex(1,A26[0]->GetRmax2()-r,-0.5*WidthFrame26);
    B26->SetVertex(2,A26[0]->GetRmin2()-r,-0.5*WidthFrame26);
    B26->SetVertex(3,A26[0]->GetRmin2()-r,+0.5*WidthFrame26);
    B26->SetVertex(4,A26[1]->GetRmax1()-r,+0.5*WidthFrame26);
    B26->SetVertex(5,A26[1]->GetRmax1()-r,-0.5*WidthFrame26);
    B26->SetVertex(6,A26[1]->GetRmin1()-r,-0.5*WidthFrame26);
    B26->SetVertex(7,A26[1]->GetRmin1()-r,+0.5*WidthFrame26);
    for(i=0;i<NZsections26+1;i++) printConeSeg(A26[i]);
    printConeSeg(M26);
    printArb8(B26);
    TGeoVolume *A26v[NZsections26+1],*B26v,*M26v;
    //
    for(i=0;i<NZsections26+1;i++){
        sprintf(name,"ITSsupFrameA26[%d]",i);
        A26v[i] = new TGeoVolume(name,A26[i],SUPss);
        A26v[i]->SetVisibility(kTRUE);
        A26v[i]->SetLineColor(1); // black
        A26v[i]->SetLineWidth(1);
        A26v[i]->SetFillColor(A26v[i]->GetLineColor());
        A26v[i]->SetFillStyle(4000); // 0% transparent
    } // end for i
    B26v = new TGeoVolume("ITSsupFrameB26",B26,SUPss);
    B26v->SetVisibility(kTRUE);
    B26v->SetLineColor(1); // black
    B26v->SetLineWidth(1);
    B26v->SetFillColor(B26v->GetLineColor());
    B26v->SetFillStyle(4000); // 0% transparent
    M26v = new TGeoVolume("ITSsupFrameM26",M26,SUPair);
    M26v->SetVisibility(kTRUE);
    M26v->SetLineColor(7); // light blue
    M26v->SetLineWidth(1);
    M26v->SetFillColor(M26v->GetLineColor());
    M26v->SetFillStyle(4090); // 90% transparent
    //
    Int_t NcB26=1;
    t0 = Phi0Frame26;
    dt = (Phi1Frame26-Phi0Frame26)/((Double_t)NPhiSections26);
    for(i=0;i<=NZsections26;i++){
        di = ((Double_t) i)*(ZssFrameSection26+WidthFrame26);
        z = -M26->GetDz()+A26[i]->GetDz() + di;
        tran = new TGeoTranslation("",0.0,0.0,z);
        M26v->AddNode(A26v[i],1,tran);
        z = z+B26->GetDz();
        if(i<NZsections26)for(j=0;j<=NPhiSections26;j++){
            r = 0.25*(A26[i]->GetRmax1()+A26[i]->GetRmin1()+
                      A26[i+1]->GetRmax2()+A26[i+1]->GetRmin2());
            t = t0 + ((Double_t)j)*dt;
            rot = new TGeoRotation("",0.0,0.0,t);
            y = r*TMath::Sin(t*kRadian);
            x = r*TMath::Cos(t*kRadian);
            tranrot = new TGeoCombiTrans("",x,y,z,rot);
            delete rot; // rot not explicity used in AddNode functions.
            M26v->AddNode(B26v,NcB26++,tranrot);
        } // end for j
    } // end for i
    tran = new TGeoTranslation("",0.0,0.0,Z026-M26->GetDz());
    Moth->AddNode(M26v,1,tran);
    for(i=1;i<NFramesPhi26;i++){
        rot = new TGeoRotation("",0.0,0.0,90.0*((Double_t)i));
        tranrot = new TGeoCombiTrans(*tran,*rot);
        delete rot; // rot not explicity used in AddNode functions.
        Moth->AddNode(M26v,i+1,tranrot);
    } // end for i
    if(GetDebug()){
        for(i=0;i<NZsections26+1;i++) A26v[i]->PrintNodes();
        B26v->PrintNodes();
        M26v->PrintNodes();
    } // end if
}
//______________________________________________________________________
void AliITSv11::InitAliITSgeom(){
    // Based on the geometry tree defined in Geant 3.21, this
    // routine initilizes the Class AliITSgeom from the Geant 3.21 ITS 
    // geometry sturture.
    // Inputs:
    //   none.
    // Outputs:
    //   none.
    // Return
    //   none.
}
//______________________________________________________________________
void AliITSv11::Init(){
    // Initialise the ITS after it has been created.
    // Inputs:
    //   none.
    // Outputs:
    //   none.
    // Return
    //   none.
}
//______________________________________________________________________
void AliITSv11::SetDefaults(){
    // Sets the default segmentation, response, digit and raw cluster 
    // classes to be used. These defaults can be overwritten in the
    // macros that do these later steps. Defaults are give hear for the
    // general user.
    // Inputs:
    //   none.
    // Outputs:
    //   none.
    // Return
    //   none.
}
//______________________________________________________________________
void AliITSv11::DrawModule(){
    // Draw a standard set of shaded view of the ITS version 11.
    // Inputs:
    //   none.
    // Outputs:
    //   none.
    // Return
    //   none.
}
//______________________________________________________________________
void AliITSv11::StepManager(){
    // Called for every step in the ITS, then calles the AliITShit class
    // creator with the information to be recoreded about that hit.
    //  The value of the macro ALIITSPRINTGEOM if set to 1 will allow the
    // printing of information to a file which can be used to create a .det
    // file read in by the routine CreateGeometry(). If set to 0 or any other
    // value except 1, the default behavior, then no such file is created nor
    // is the extra variables and the like used in the printing allocated.
}
 
