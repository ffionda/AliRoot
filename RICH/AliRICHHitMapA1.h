#ifndef ALIRICHHITMAPA1_H
#define ALIRICHHITMAPA1_H


/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */
#include "AliHitMap.h"

class  TObjArray;
class  AliSegmentation;


class AliRICHHitMapA1 :
public AliHitMap 
{
    
 public:
    AliRICHHitMapA1();
    AliRICHHitMapA1(AliSegmentation *seg, TObjArray *dig);
    virtual ~AliRICHHitMapA1();
    virtual  void    FillHits();
    virtual  void    Clear(const char *opt = "");    
    virtual  void    SetHit(Int_t ix, Int_t iy, Int_t idigit);
    virtual  void    DeleteHit(Int_t ix, Int_t iy);
    virtual Int_t    GetHitIndex(Int_t ix, Int_t iy) const;
    virtual TObject* GetHit(Int_t ix, Int_t iy) const;
    virtual  void    FlagHit(Int_t ix, Int_t iy);
    virtual  Bool_t  ValidateHit(Int_t, Int_t) {return 1;}
    virtual FlagType TestHit(Int_t ix, Int_t iy);
 private:
    Int_t CheckedIndex(Int_t ix, Int_t iy) const;

 private:
    AliSegmentation *fSegmentation;                        //Segmentation model
    Int_t fNpx;                                            //Pads in x
    Int_t fNpy;                                            //Pads in y
    TObjArray *fDigits;                                    //List of digits
    Int_t fNdigits;                                        //Number of digits
    Int_t *fHitMap;                                        // !
    Int_t fMaxIndex;                                       //Index size

    ClassDef(AliRICHHitMapA1,2) // Implements HitMap as a 2-dim array
};
#endif	





