
// (c) COPYRIGHT URI/MIT 1998
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//	jhrg,jimg	James Gallagher (jgallagher@gso.uri.edu)

/** Asciival is a simple DODS client similar to geturl or writeval that reads
    a DODS data object (either by dereferencing a URL, reading from a file or
    reading from stdin) and writes comma separated ASCII representation of the
    data values in that object. 
    
    @author: jhrg */

// $Log: ascii_val.cc,v $
// Revision 1.2  1998/03/16 19:45:09  jimg
// Added mime header output. See -m.
//
// Revision 1.1  1998/03/16 18:30:19  jimg
// Added
//

#include "config_dap.h"

static char rcsid[] __unused__ = {"$Id: ascii_val.cc,v 1.2 1998/03/16 19:45:09 jimg Exp $"};

#include <stdio.h>
#include <assert.h>

#include <GetOpt.h>
#include <Pix.h>
#include <SLList.h>
#include <String.h>

#include "BaseType.h"
#include "Connect.h"

#include "AsciiByte.h"
#include "AsciiInt32.h"
#include "AsciiUInt32.h"
#include "AsciiFloat64.h"
#include "AsciiStr.h"
#include "AsciiUrl.h"
#include "AsciiList.h"
#include "AsciiArray.h"
#include "AsciiStructure.h"
#include "AsciiSequence.h"
#include "AsciiFunction.h"
#include "AsciiGrid.h"

#include "name_map.h"
#include "cgi_util.h"

name_map names;
bool translate = false;
const char *VERSION = "DODS asciival version: 1.0";

static void
usage(String name)
{
    cerr << "Usage: " << name 
	 << " [ngvVt] -- [<url> [-r <var>:<newvar> ...] ...]" << endl
	 << "       n: Turn on name canonicalization." << endl
	 << "       g: Use the GUI to show progress" << endl
	 << "       v: Verbose output." << endl
	 << "       V: Print the version number and exit" << endl
	 << "       t: Trace network I/O (HTTP, ...). See geturl" << endl
	 << "       r: Per-URL name mappings; var becomes newvar." << endl
	 << endl
	 << "<url> may be a true URL, which asciival will dereference," << endl
	 << "it may be a local file or it may be standard input." << endl
	 << "In the later case use `-' for <url>."
	 << endl;
}

static String
name_from_url(String url)
{
    // find the last part of the URL (after the last `/') and then strip off
    // the trailing extension (anything following the `.')
    int start = url.index("/", -1) + 1;
    int end = url.index(".", -1);
    
    String name = url.at(start, end-start);

    return name;
}

static void
process_per_url_options(int &i, int argc, char *argv[], bool verbose = false)
{
    names.delete_all();	// Clear the global name map for this URL.

    // Test for per-url option. Set variables accordingly.
    while (argv[i+1] && argv[i+1][0] == '-')
	switch (argv[++i][1]) {
	  case 'r':
	    ++i;	// Move past option to argument.
	    if (verbose)
		cerr << "  Renaming: " << argv[i] << endl;
	    // Note that NAMES is a global variable so that all the
	    // writeval() mfuncs can access it without having to pass
	    // it into each function call.
	    names.add(argv[i]);
	    break;
		
	  default:
	    cerr << "Unknown option `" << argv[i][1] 
		 << "' paired with URL has been ignored." << endl;
	    break;
	}

}

static void
process_trace_options(char *tcode) 
{
    while (tcode++)
	switch (*tcode) {
	  case 'a': WWWTRACE |= SHOW_ANCHOR_TRACE; break;
	  case 'A': WWWTRACE |= SHOW_APP_TRACE; break;
	  case 'b': WWWTRACE |= SHOW_BIND_TRACE; break;
	  case 'c': WWWTRACE |= SHOW_CACHE_TRACE; break;
	  case 'h': WWWTRACE |= SHOW_AUTH_TRACE; break;
	  case 'i': WWWTRACE |= SHOW_PICS_TRACE; break;
	  case 'k': WWWTRACE |= SHOW_CORE_TRACE; break;
	  case 'l': WWWTRACE |= SHOW_SGML_TRACE; break;
	  case 'm': WWWTRACE |= SHOW_MEM_TRACE; break;
	  case 'p': WWWTRACE |= SHOW_PROTOCOL_TRACE; break;
	  case 's': WWWTRACE |= SHOW_STREAM_TRACE; break;
	  case 't': WWWTRACE |= SHOW_THREAD_TRACE; break;
	  case 'u': WWWTRACE |= SHOW_URI_TRACE; break;
	  case 'U': WWWTRACE |= SHOW_UTIL_TRACE; break;
	  case 'x': WWWTRACE |= SHOW_MUX_TRACE; break;
	  case 'z': WWWTRACE = SHOW_ALL_TRACE; break;
	  default:
	    cerr << "Unrecognized trace option: `" << *tcode << "'" 
		 << endl;
	    break;
	}
}

static void
process_data(XDR *src, DDS *dds)
{
    for (Pix q = dds->first_var(); q; dds->next_var(q)) {
	if (dds->var(q)->type() == dods_sequence_c)
	    ((AsciiSequence *)dds->var(q))->print_all_vals(cout, src, dds);
	else
	    dds->var(q)->print_val(cout);
	cout << endl;
    }
}

// Read a DODS data object. The object maybe specified by a URL (which will
// be dereferenceed using Connect, it maybe read from a file or it maybe read
// from stdin. Use `-' in the command line to indicate the next input should
// be read from standard input.

int
main(int argc, char * argv[])
{
    GetOpt getopt (argc, argv, "ngmvVh?t:");
    int option_char;
    bool verbose = false;
    bool trace = false;
    bool translate = false;
    bool gui = false;
    bool mime_header = false;
    String expr = "";
    char *tcode = NULL;
    int topts = 0;

    putenv("_POSIX_OPTION_ORDER=1"); // Suppress GetOpt's argv[] permutation.

    while ((option_char = getopt()) != EOF)
	switch (option_char) {
	  case 'n': translate = true; break;
	  case 'g': gui = true; break;
	  case 'm': mime_header = true; break;
	  case 'v': verbose = true; break;
	  case 'V': {cerr << VERSION << endl; exit(0);}
	  case 't':
	    trace = true;
	    topts = strlen(getopt.optarg);
	    if (topts) {
		tcode = new char[topts + 1];
		strcpy(tcode, getopt.optarg); 
		process_trace_options(tcode);
		delete tcode;
	    }
	    break;
	  case 'h':
	  case '?':
	  default:
	    usage(argv[0]); exit(1); break;
	}

    Connect *url = 0;

    for (int i = getopt.optind; i < argc; ++i) {
	if (url)
	    delete url;
	
	if (strcmp(argv[i], "-") == 0)
	    url = new Connect("stdin", trace, false);
	else
	    url = new Connect(argv[i], trace);

	DBG2(cerr << "argv[" << i << "] (of " << argc << "): " << argv[i] \
	     << endl);

	if (verbose) {
	    String source_name;
	    if (url->is_local() && (strcmp(argv[i], "-") == 0))
		source_name = "standard input";
	    else if (url->is_local())
		source_name = argv[i];
	    else
		source_name = url->URL(false);

	    cerr << endl << "Reading: " << source_name << endl;
	}

	process_per_url_options(i, argc, argv, verbose);

	DDS *dds;

	if (url->is_local() && (strcmp(argv[i], "-") == 0))
	    dds = url->read_data(stdin, gui, false);
	else if (url->is_local())
	    dds = url->read_data(fopen(argv[i], "r"), gui, false);
	else
	    dds = url->request_data(expr, gui, false);

	if (dds) {
	    if (mime_header)
		set_mime_text(dods_data);
	    process_data(url->source(), dds);
	}
    }

    cout.flush();
    cerr.flush();

    return 0;
}
