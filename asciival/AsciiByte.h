
// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1998
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// his set of subclasses is used to build a simple client program
// which will dump a binary version of the DODS variables to a file
// (nominally stdout).
//
// 3/12/98 jhrg

// $Log: AsciiByte.h,v $
// Revision 1.1  1998/03/13 21:25:21  jimg
// Added
//

#ifndef _AsciiByte_h
#define _AsciiByte_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include "Byte.h"

class AsciiByte: public Byte {
public:
    AsciiByte(const String &n = (char *)0);
    virtual ~AsciiByte() {}

    virtual BaseType *ptr_duplicate();

    virtual bool read(const String &dataset, int &error);

    virtual void print_val(ostream &os, String space = "", 
			   bool print_decl_p = true);
};

#endif
