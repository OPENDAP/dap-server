// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1998
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Interface for AsciiStr type. See AsciiByte.h
//
// 3/12/98 jhrg

// $Log: AsciiStr.h,v $
// Revision 1.2  1999/04/30 17:06:54  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.1  1998/03/13 21:25:25  jimg
// Added
//

#ifndef _AsciiStr_h
#define _AsciiStr_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include "dods-limits.h"
#include "Str.h"

class AsciiStr: public Str {
public:
    AsciiStr(const string &n = (char *)0);
    virtual ~AsciiStr() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const string &dataset, int &error);

    virtual void print_val(ostream &os, string space = "", 
			   bool print_decl_p = true);
};

#endif

