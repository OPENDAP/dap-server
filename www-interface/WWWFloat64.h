// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1999
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Interface for WWWFloat64 type. See WWWByte.h
//
// 4/7/99 jhrg

#ifndef _wwwfloat64_h
#define _wwwfloat64_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include "Float64.h"
#include "WWWOutput.h"

class WWWFloat64: public Float64 {
public:
    WWWFloat64(const string &n = (char *)0);
    virtual ~WWWFloat64() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const string &dataset);

    virtual void print_val(ostream &os, string space = "", 
			   bool print_decl_p = true);
};

// $Log: WWWFloat64.h,v $
// Revision 1.4  2000/10/03 20:07:20  jimg
// Moved Logs to the end of each file.
//
// Revision 1.3  2000/10/02 22:42:44  jimg
// Modified the read method to match the new definition in the dap
//
// Revision 1.2  1999/05/09 04:14:51  jimg
// String --> string
//
// Revision 1.1  1999/04/20 00:21:03  jimg
// First version
//

#endif

