
// (c) COPYRIGHT URI/MIT 1999
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Implementation for WWWArray. See WWWByte.cc
//
//  4/7/99 jhrg

// $Log: WWWArray.cc,v $
// Revision 1.1  1999/04/20 00:21:02  jimg
// First version
//

#ifdef __GNUG__
#pragma implementation
#endif

#include "config_dap.h"

#include <assert.h>

#include <iostream.h>

#include <Pix.h>
#include <SLList.h>
#include <String.h>

#include "WWWArray.h"
#include "WWWOutput.h"

#ifdef TRACE_NEW
#include "trace_new.h"
#endif

Array *
NewArray(const String &n, BaseType *v)
{
    return new WWWArray(n, v);
}

BaseType *
WWWArray::ptr_duplicate()
{
    return new WWWArray(*this);
}

WWWArray::WWWArray(const String &n, BaseType *v) : Array(n, v)
{
}

WWWArray::~WWWArray()
{
}

bool
WWWArray::read(const String &, int &)
{
    assert(false);
    return false;
}

void 
WWWArray::print_val(ostream &os, String, bool print_decl_p)
{
    os << "<script type=\"text/javascript\">\n"
       << "<!--\n"
       << name() << " = new dods_var(\"" << name() << "\", 1);\n"
       << "DODS_URL.add_dods_var(" << name() << ");\n"
       << "// -->\n"
       << "</script>\n";

    os << "<b>" 
       << "<input type=\"checkbox\" name=\"get_" << name() << "\"\n"
       << "onclick=\"" << name() << ".handle_projection_change(get_"
       << name() << ")\">\n" 
       << "<font size=\"+1\">" << name() << "</font>"
       << ": " << fancy_typename(this) << "</b><br>\n\n";

    Pix p = first_dim();
    for (int i = 0; p; ++i, next_dim(p)) {
	int size = dimension_size(p, true);
	string n = (const char *)dimension_name(p);
	if (n != "")
	    os << n << ":";
	os << "<input type=\"text\" name=\"" << name() << "_" << i 
	   << "\" size=8 onfocus=\"describe_index()\""
	   << " onChange=\"DODS_URL.update_url()\">\n";
	os << "<script type=\"text/javascript\">\n"
	   << "<!--\n"
	   << name() << ".add_dim(" << size << ");\n"
	   << "// -->\n"
	   << "</script>\n";
    }
    
    os << "<br>\n\n";
}