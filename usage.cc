
// (c) COPYRIGHT URI/MIT 1996
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// The Usage server. Arguments: two arguments; the dataset name and the
// pathname and `api prefix' of the data server. Returns a HTML document that
// describes what information this dataset contains, special characteristics
// of the server users might want to know and any special information that
// the dataset providers want to make available. 
// jhrg 12/9/96

// $Log: usage.cc,v $
// Revision 1.2  1996/12/18 18:41:33  jimg
// Added massive fixes for the processing of attributes to sort out the `global
// attributes'. Also added changes to the overall layout of the resulting
// document so that the hierarchy of the data is represented. Added some new
// utility functions.
//
// Revision 1.1  1996/12/11 19:55:18  jimg
// Created.
//

#include "config_dap.h"

static char rcsid[] __unused__ = {"$Id: usage.cc,v 1.2 1996/12/18 18:41:33 jimg Exp $"};

#include <stdio.h>
#include <assert.h>

#include <iostream.h>
#include <fstream.h>

#include <String.h>
#include <strstream.h>

#include "cgi_util.h"
#include "debug.h"

static void
usage(char *argv[])
{
    cerr << argv[0] << " <filename> <CGI directory>" << endl
	 << "      Takes two required arguments; the dataset filename" << endl
	 << "      and the directory and api prefix for the filter" << endl
	 << "      program" << endl; 
}

/// Look for the override file.
/** Look for the override file by taking the dataset name and appending
    `.ovr' to it. If such a file exists, then read it in and store the
    contents in #doc#. Note that the file contents are not checked to see if
    they are valid HTML (which they must be). 

    Returns: True if the `override file' is present, false otherwise. in the
    later case #doc#'s contents are undefined.
*/

bool
found_override(String name, String &doc)
{
    ifstream ifs(name + ".ovr");
    if (!ifs)
	return false;

    char tmp[256];
    doc = "";
    while (!ifs.eof()) {
	ifs.getline(tmp, 255);
	strcat(tmp, "\n");
	doc += tmp;
    }

    return true;
}

/// Read and discard the MIME header of the stream #in#.
/** Read the input stream #in# and discard the MIME header. The MIME header
    is separated from the body of the document by a single blank line. If no
    MIME header is found, then the input stream is `emptied' and will contain
    nothing.

    Returns: True if a MIME header is found, false otherwise.
*/

bool
remove_mime_header(FILE *in)
{
    char tmp[256];
    while (!feof(in)) {
	fgets(tmp, 255, in);
	if (tmp[0] == '\n')
	    return true;
    }

    return false;
}    

/// Look for the user supplied CGI- and dataset-specific HTML* documents.
/** Look in the CGI directory (given by #cgi#) for a per-cgi HTML* file. Also
    look for a dataset-specific HTML* document. Catenate the documents and
    return them in a single String variable.

    The #cgi# path must include the `API' prefix at the end of the path. For
    example, for the NetCDF server whose prefix is `nc' and resides in the
    DODS_ROOT/etc directory of my computer, #cgi# is
    `/home/dcz/jimg/src/DODS/etc/nc'. This function then looks for the file
    named #cgi#.html.

    Similarly, to locate the dataset-specific HTML* file it catenates `.html'
    to #name#, where #name# is the name of the dataset. If the filename part
    of #name# is of the form [A-Za-z]+[0-9]*.* then this function also looks
    for a file whose name is [A-Za-z].html For example, if #name# is
    .../data/fnoc1.nc this function first looks for .../data/fnoc1.nc.html.
    However, if that does not exist it will look for .../data/fnoc.html. This
    allows one `per-dataset' file to be used for a collection of files with
    the same root name.

    NB: An HTML* file contains HTML without the <html>, <head> or <body> tags
    (my own notation).

    Returns: A String which contains these two documents catenated. Documents
    that don't exist are treated as `empty'.
*/

String
get_user_supplied_docs(String name, String cgi)
{
    char tmp[256];
    ostrstream oss;
    ifstream ifs(cgi + ".html");

    if (ifs) {
	while (!ifs.eof()) {
	    ifs.getline(tmp, 255);
	    oss << tmp << "\n";
	}
	ifs.close();
	
	oss << "<hr>";
    }

    ifs.open(name + ".html");

    // If name.html cannot be opened, look for basename.html
    if (!ifs) {
	int slash = name.index("/", -1);
	String pathanme = name.before(slash);
	String filename = name.after(slash);
	filename = filename.at(RXalpha);
	String new_name = pathanme + "/" + filename + ".html";
	ifs.open(new_name);
    }

    if (ifs) {
	while (!ifs.eof()) {
	    ifs.getline(tmp, 255);
	    oss << tmp << "\n";
	}
	ifs.close();
    }

    oss << ends;
    String html = oss.str();
    oss.freeze(0);

    return html;
}

static bool
name_in_variable(BaseType *btp, const String &name)
{
    switch (btp->type()) {
      case dods_byte_c:
      case dods_int32_c:
      case dods_uint32_c:
      case dods_float64_c:
      case dods_str_c:
      case dods_url_c:
      case dods_array_c:
      case dods_list_c:
	return (btp->name() == name);

      case dods_structure_c: {
	  Structure *sp = (Structure *)btp;
	  for (Pix p = sp->first_var(); p; sp->next_var(p)) {
	      if (name_in_variable(sp->var(p), name))
		  return true;
	  }
	  break;
      }

      case dods_sequence_c: {
	  Sequence *sp = (Sequence *)btp;
	  for (Pix p = sp->first_var(); p; sp->next_var(p)) {
	      if (name_in_variable(sp->var(p), name))
		  return true;
	  }
	  break;
      }

      case dods_function_c: {
	  Function *fp = (Function *)btp;
	  for (Pix p = fp->first_indep_var(); p; fp->next_indep_var(p)) {
	      if (name_in_variable(fp->indep_var(p), name))
		  return true;
	  }
	  for (Pix p = fp->first_dep_var(); p; fp->next_dep_var(p)) {
	      if (name_in_variable(fp->dep_var(p), name))
		  return true;
	  }
	  break;
      }

      case dods_grid_c: {
	  Grid *gp = (Grid *)btp;
	  if (gp->array_var()->name() == name)
	      return true;
	  for (Pix p = gp->first_map_var(); p; gp->next_map_var(p)) {
	      if (name_in_variable(gp->map_var(p), name))
		  return true;
	  }
	  break;
      }

      default:
	assert("Unknown type" && false);
	return false;
    }

    return false;
}


static bool
name_in_dds(DDS &dds, const String &name)
{
    BaseType *btp = dds.var(name);
    
    if (btp)
	return true;
    
    for (Pix q = dds.first_var(); q; dds.next_var(q)) {
	btp = dds.var(q);

	switch (btp->type()) {
	  case dods_byte_c:
	  case dods_int32_c:
	  case dods_uint32_c:
	  case dods_float64_c:
	  case dods_str_c:
	  case dods_url_c:
	  case dods_array_c:
	  case dods_list_c:
	    break;

	  case dods_structure_c: {
	      Structure *sp = (Structure *)btp;
	      for (Pix p = sp->first_var(); p; sp->next_var(p)) {
		  if (name_in_variable(sp->var(p), name))
		      return true;
	      }
	      break;
	  }

	  case dods_sequence_c: {
	      Sequence *sp = (Sequence *)btp;
	      for (Pix p = sp->first_var(); p; sp->next_var(p)) {
		  if (name_in_variable(sp->var(p), name))
		      return true;
	      }
	      break;
	  }

	  case dods_function_c: {
	      Function *fp = (Function *)btp;
	      for (Pix p = fp->first_indep_var(); p; fp->next_indep_var(p)) {
		  if (name_in_variable(fp->indep_var(p), name))
		      return true;
	      }
	      for (Pix p = fp->first_dep_var(); p; fp->next_dep_var(p)) {
		  if (name_in_variable(fp->dep_var(p), name))
		      return true;
	      }
	      break;
	  }

	  case dods_grid_c: {
	      Grid *gp = (Grid *)btp;
	      if (gp->array_var()->name() == name)
		  return true;
	      for (Pix p = gp->first_map_var(); p; gp->next_map_var(p)) {
		  if (name_in_variable(gp->map_var(p), name))
		      return true;
	      }
	      break;
	  }

	  default:
	    assert("Unknown type" && false);
	}
    }

    return false;
}

// This code could use a real `kill-file' some day - about the same time that
// the rest of the server gets a `rc' file... For the present just see if a
// small collection of regexs match the name.

static bool
name_in_kill_file(const String &name)
{
    static Regex dim(".*_dim_[0-9]*", 1); // HDF `dimension' attributes.

    return name.matches(dim);
}

/// Build the global attribute HTML* document.
/** Given the DAS and DDS, build the HTML* document which contains all the
    global attributes for this dataset. A global attribute is defined here as
    any attribute not bound to variable in the dataset. Thus the attributes
    of `NC_GLOBAL', `global', etc. will be called global attributes if there
    are no variables `NC_GLOBAL', ... in the dataset. If there are variable
    with those names the attributes will NOT be considered `global
    attributes'.

    Returns: A String object containing the global attributes in human
    readable form (as an HTML* document).
*/

String
build_global_attributes(DAS &das, DDS &dds)
{
    bool found = false;
    ostrstream ga;

    ga << "<h3>Dataset Information</h3>\n<center>\n<table>\n";

    for (Pix p = das.first_var(); p; das.next_var(p)) {
	String name = das.get_name(p);

	if (!name_in_kill_file(name) && !name_in_dds(dds, name)) {
	    AttrTable *attr = das.get_table(p);

	    if (attr) {
		for (Pix a = attr->first_attr(); a; attr->next_attr(a)) {
		    int num_attr = attr->get_attr_num(a);

		    found = true;
		    ga << "\n<tr><td align=right valign=top><b>" 
		       << attr->get_name(a) << "</b>:</td>\n";
		    ga << "<td align=left>";
		    for (int i = 0; i < num_attr; ++i)
			ga << attr->get_attr(a, i) << "<br>";
		    ga << "</td></tr>\n";
		}
	    }
	}
    }

    ga << "</table>\n</center><p>\n" << ends;

    if (found) {
	String global_attrs = ga.str();
	ga.freeze(0);

	return global_attrs;
    }

    return "";
}

static String
fancy_typename(BaseType *v)
{
    String fancy;
    switch (v->type()) {
      case dods_byte_c:
	return "Byte";
      case dods_int32_c:
	return "32 bit Integer";
      case dods_uint32_c:
	return "32 bit Unsigned integer";
      case dods_float64_c:
	return "64 bit Real";
      case dods_str_c:
	return "String";
      case dods_url_c:
	return "URL";
      case dods_array_c: {
	  ostrstream type;
	  Array *a = (Array *)v;
	  type << "Array of " << fancy_typename(a->var()) <<"s ";
	  for (Pix p = a->first_dim(); p; a->next_dim(p))
	      type << "[" << a->dimension_name(p) << " = 0.." 
		   << a->dimension_size(p, false)-1 << "]";
	  type << ends;
	  String fancy = type.str();
	  type.freeze(0);
	  return fancy;
      }
      case dods_list_c: {
	  ostrstream type;
	  List *l = (List *)v;
	  type << "List of " << fancy_typename(l->var()) <<"s " << ends;
	  String fancy = type.str();
	  type.freeze(0);
	  return fancy;
      }
      case dods_structure_c:
	return "Structure";
      case dods_sequence_c:
	return "Sequence";
      case dods_function_c:
	return "Function";
      case dods_grid_c:
	return "Grid";
      default:
	return "Unknown";
    }
}

static void
write_variable(BaseType *btp, DAS &das, ostrstream &vs)
{
    vs << "<td align=right valign=top><b>" << btp->name() 
	<< "</b>:</td>\n"
	<< "<td align=left valign=top>" << fancy_typename(btp)
	    << "<br>";
    //	    << "</td>\n<td align=left valign=top>";
    AttrTable *attr = das.get_table(btp->name());
	    
    if (attr)			// Not all variables have attributes!
	for (Pix a = attr->first_attr(); a; attr->next_attr(a)) {
	    int num = attr->get_attr_num(a);
	    
	    vs << attr->get_name(a) << ": ";
	    for (int i = 0; i < num; ++i, (void)(i<num && vs << ", "))
		vs << attr->get_attr(a, i);
	    vs << "<br>\n";
	}

    switch (btp->type()) {
      case dods_byte_c:
      case dods_int32_c:
      case dods_uint32_c:
      case dods_float64_c:
      case dods_str_c:
      case dods_url_c:
      case dods_array_c:
      case dods_list_c:
	vs << "</td>\n";
	break;

      case dods_structure_c: {
	vs << "<table>\n";
	Structure *sp = (Structure *)btp;
	for (Pix p = sp->first_var(); p; sp->next_var(p)) {
	    vs << "<tr>";
	    write_variable(sp->var(p), das, vs);
	    vs << "</tr>";
	}
	vs << "</table>\n";
	break;
      }

      case dods_sequence_c: {
	vs << "<table>\n";
	Sequence *sp = (Sequence *)btp;
	for (Pix p = sp->first_var(); p; sp->next_var(p)) {
	    vs << "<tr>";
	    write_variable(sp->var(p), das, vs);
	    vs << "</tr>";
	}
	vs << "</table>\n";
	break;
      }

      case dods_function_c: {
	vs << "<table>\n";
	Function *fp = (Function *)btp;
	for (Pix p = fp->first_indep_var(); p; fp->next_indep_var(p)) {
	    vs << "<tr>";
	    write_variable(fp->indep_var(p), das, vs);
	    vs << "</tr>";
	}
	for (Pix p = fp->first_dep_var(); p; fp->next_dep_var(p)) {
	    vs << "<tr>";
	    write_variable(fp->dep_var(p), das, vs);
	    vs << "</tr>";
	}
	vs << "</table>\n";
	break;
      }

      case dods_grid_c: {
	vs << "<table>\n";
	Grid *gp = (Grid *)btp;
	write_variable(gp->array_var(), das, vs);
	for (Pix p = gp->first_map_var(); p; gp->next_map_var(p)) {
	    vs << "<tr>";
	    write_variable(gp->map_var(p), das, vs);
	    vs << "</tr>";
	}
	vs << "</table>\n";
	break;
      }

      default:
	assert("Unknown type" && false);
    }
}

/// Build the variable summaries.
/** Given the DAS and the DDS build an HTML table which describes each one of
    the variables by listing its name, datatype and all of its attriutes.

    Returns: A String object containing the variable summary information in
    human readable form (as an HTML* document).
*/

String
build_variable_summaries(DAS &das, DDS &dds)
{
    ostrstream vs;
    vs << "<h3>Variables in this Dataset</h3>\n<center>\n<table>\n";
    //    vs << "<tr><th>Variable</th><th>Information</th></tr>\n";

    for (Pix p = dds.first_var(); p; dds.next_var(p)) {
	vs << "<tr>";
	write_variable(dds.var(p), das, vs);
	vs << "</tr>";
    }

    vs << "</table>\n</center><p>\n" << ends;

    String html = vs.str();
    vs.freeze(0);

    return html;
}

static void
html_header()
{
    cout << "HTTP/1.0 200 OK" << endl;
    cout << "Server: " << DVR << endl;
    cout << "Content-type: text/html" << endl; 
    cout << "Content-Description: dods_description" << endl;
    cout << endl;			// MIME header ends with a blank line
}

int 
main(int argc, char *argv[])
{
    if (argc != 3) {
	usage(argv);
	exit(1);
    }

    String name = argv[1];
    String doc;

    if (found_override(name, doc)) {
	html_header();
	cout << doc;
	exit(0);
    }

    // The user is not overriding the DAS/DDS generated information, so read
    // the DAS, DDS and user supplied documents. 

    String cgi = argv[2];

    DAS das;
    String command = cgi + "_das '" + name + "'";
    DBG(cerr << "DAS Command: " << command << endl);

    FILE *in = popen(command, "r");
    if (in && remove_mime_header(in)) {
	das.parse(in);
	pclose(in);
    }

    DDS dds;
    command = cgi + "_dds '" + name + "'";
    DBG(cerr << "DDS Command: " << command << endl);

    in = popen(cgi + "_dds '" + name + "'", "r");
    if (in && remove_mime_header(in)) {
	dds.parse(in);
	pclose(in);
    }

    // Build the HTML* documents.

    String user_html = get_user_supplied_docs(name, cgi);

    String global_attrs = build_global_attributes(das, dds);

    String variable_sum = build_variable_summaries(das, dds);

    // Write out the HTML document.

    html_header();

    if (global_attrs) {
	cout << "<html><head><title>Dataset Information</title></head>" 
	     << endl 
	     << "<html>" << endl 
	     << global_attrs << endl 
	     << "<hr>" << endl;
    }

    cout << variable_sum << endl;

    cout << "<hr>" << endl;

    cout << user_html << endl;

    cout << "</html>" << endl;

    exit(0);
}