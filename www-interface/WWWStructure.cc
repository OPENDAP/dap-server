
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of www_int, software which returns an HTML form which
// can be used to build a URL to access data from a DAP data server.

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// www_int is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2, or (at your option) any later
// version.
// 
// www_int is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
// 
// You should have received a copy of the GNU General Public License along
// with GCC; see the file COPYING. If not, write to the Free Software
// Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// 
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

// (c) COPYRIGHT URI/MIT 1999
// Please read the full copyright statement in the file COPYRIGHT_URI.
//
// Authors:
//      jhrg,jimg       James Gallagher <jgallagher@gso.uri.edu>

// Implementation for the class WWWStructure. See WWWByte.cc
//
// 4/7/99 jhrg

#include "config.h"

static char rcsid[] not_used = {"$Id$"};

#include <iostream>
#include <string>

#include "DAS.h"
#include "InternalErr.h"

#include "WWWStructure.h"
#include "WWWSequence.h"
#include "WWWOutput.h"
#include "get_html_form.h"

using namespace dap_html_form;

BaseType *
WWWStructure::ptr_duplicate()
{
    return new WWWStructure(*this);
}

WWWStructure::WWWStructure(const string &n) : Structure(n)
{
}

WWWStructure::WWWStructure( Structure *bt ) : Structure( bt->name() )
{
    Vars_iter p = bt->var_begin();
    while( p != bt->var_end() )
    {
        BaseType *new_bt = basetype_to_wwwtype( *p ) ;
        add_var( new_bt ) ;
        delete new_bt ;
        p++ ;
    }
}

WWWStructure::~WWWStructure()
{
}

// For this `WWW' class, run the read mfunc for each of variables which
// comprise the structure. 

// As is the case with geturl, use print_all_vals to print all the values of
// a sequence. 

void 
WWWStructure::print_val(FILE *os, string /*space*/, bool print_decls)
{
    fprintf(os, "<b>Structure %s </b><br>\n", name().c_str());
    fprintf(os, "<dl><dd>\n");

    for (Vars_iter i = var_begin(); i != var_end(); ++i) {
        (*i)->print_val(os, "", print_decls);
        wo->write_variable_attributes(*i, *(wo->get_das()));
        fprintf(os, "<p><p>\n");
    }

    fprintf(os, "</dd></dl>\n");
}

// Is this a simple WWWStructure? Simple WWWStructures are composed of
// only simple type elements *or* other structures which are simple.

bool
WWWStructure::is_simple_structure()
{
    for (Vars_iter i = var_begin(); i != var_end(); ++i) {
	if ((*i)->type() == dods_structure_c) {
	    if (!dynamic_cast<WWWStructure *>(*i)->is_simple_structure())
		return false;
	}
	else {
	    if (!(*i)->is_simple_type())
		return false;
	}
    }


    return true;
}
