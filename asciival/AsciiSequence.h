
// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1998,2000
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Interface for the class AsciiSequence. See AsciiByte.h
//
// 3/12/98 jhrg

#ifndef _asciisequence_h
#define _asciisequence_h 1

#ifdef _GNUG_
#pragma interface
#endif

#include "Sequence.h"

class AsciiSequence: public Sequence {
private:
    bool is_simple_sequence();
    void print_header(ostream &os);

public:
    AsciiSequence(const string &n = (char *)0);
    virtual ~AsciiSequence();

    virtual BaseType *ptr_duplicate();

    virtual bool read(const string &dataset);

    virtual int length();

    virtual void print_val(ostream &os, string space = "", 
			   bool print_decl_p = true);

    virtual void print_all_vals(ostream &os, XDR *src, DDS *dds, 
				string space = "", bool print_decl_p = true);
};

// $Log: AsciiSequence.h,v $
// Revision 1.3  2000/10/02 20:09:52  jimg
// Moved Log entries to the end of the files
//
// Revision 1.2  1999/04/30 17:06:54  jimg
// Merged with no-gnu and release-2-24
//
// Revision 1.1  1998/03/13 21:25:24  jimg
// Added
//

#endif
