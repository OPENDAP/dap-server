// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1998
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Interface for AsciiFloat64 type. See AsciiByte.h
//
// 3/12/98 jhrg

// $Log: AsciiFloat64.h,v $
// Revision 1.2  1999/04/30 17:06:54  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.1  1998/03/13 21:25:22  jimg
// Added
//

#ifndef _AsciiFloat64_h
#define _AsciiFloat64_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include "Float64.h"

class AsciiFloat64: public Float64 {
public:
    AsciiFloat64(const string &n = (char *)0);
    virtual ~AsciiFloat64() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const string &dataset, int &error);

    virtual void print_val(ostream &os, string space = "", 
			   bool print_decl_p = true);
};

#endif

