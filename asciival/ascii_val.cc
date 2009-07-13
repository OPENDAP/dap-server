
// -*- mode: c++; c-basic-offset:4 -*-

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
//
// This is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

// (c) COPYRIGHT URI/MIT 1998-2000
// Please read the full copyright statement in the file COPYRIGHT_URI.
//
// Authors:
//      jhrg,jimg       James Gallagher <jgallagher@gso.uri.edu>

/** Asciival is a simple filter program similar to geturl or writeval that
    reads a data object from a DAP server (either by dereferencing a URL,
    reading from a file or reading from stdin) and writes comma separated
    ASCII representation of the data values in that object.

    Past versions could read from several URLs. This version can read a
    single URL _or_ can read from a local file using a handler (run in a sub
    process). It cannot read from several URLs. That feature was never used
    and was hard to implement with the new file/handler mode. The
    file/handler mode makes server operation much more efficient since the
    data are not read by making another trip to the server. Instead, the
    handler, running in a subprocess, will return data directly to asciival.

    The -r 'per url' option has been removed. This was also not ever used.

    @todo The code in read_from_file should really be moved into DDS. Or
    DataDDS. That is, DataDDS should have the ability to read and parse not
    only the DDS part of a 'DODS' response, but also load those variables with
    data. It should also be smart enough to sort out Error objects and throw
    them, something the code here (and also in Connect::process_data()) does
    not do.

    @author: jhrg */

#include "config.h"

static char rcsid[] not_used =
    { "$Id$" };

#include <stdio.h>

#include <string>
#include <iostream>

using namespace std;

#include <GetOpt.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include <BaseType.h>
#include <Connect.h>
#include <PipeResponse.h>
#include <escaping.h>
#include <GNURegex.h>
#include <cgi_util.h>
#include <debug.h>

#include "AsciiByte.h"
#include "AsciiInt32.h"
#include "AsciiUInt32.h"
#include "AsciiFloat64.h"
#include "AsciiStr.h"
#include "AsciiUrl.h"
#include "AsciiArray.h"
#include "AsciiStructure.h"
#include "AsciiSequence.h"
#include "AsciiGrid.h"

#include "AsciiOutputFactory.h"

#include "get_ascii.h"

using namespace dap_asciival;

static void usage()
{
    cerr << "Usage: \n"
        << " [mnhwvruVt] -- [<url> | <file> ]\n"
        << "       m: Output a MIME header.\n"
        <<
        "       f <handler pathname>: Use a local handler instead of reading from a URL\n"
        << "          This assumes a local file, not a url.\n" <<
        "       w: Verbose (wordy) output.\n" <<
        "       v <string>: Read the server version information.\n" <<
        "       u <url>: URL to the referenced data source.\n" <<
        "       r <dir>: Path to the cache directory.\n" <<
        "       e <expr>: Constraint expression.\n" <<
        "       V: Print the version number and exit.\n" <<
        "       h: Print this information.\n" <<
        "<url> may be a true URL, which asciival will dereference,\n" <<
        "it may be a local file or it may be standard input.\n" <<
        "In the later case use `-' for <url>.\n";
}

static void
read_from_url(DataDDS & dds, const string & url, const string & expr)
{
    Connect *c = new Connect(url);

    c->set_cache_enabled(false);        // Server components should not cache...

    if (c->is_local()) {
    	delete c; c = 0;
        string msg = "Error: URL `";
        msg += string(url) + "' is local.\n";
        throw Error(msg);
    }

    c->request_data(dds, expr);

    delete c;
    c = 0;
}

static void
read_from_file(DataDDS & dds, const string & handler,
               const string & options, const string & file,
               const string & expr)
{
    Regex handler_allowed("[-a-zA-Z_]+");
    if (!handler_allowed.match(handler.c_str(), handler.length()))
	throw Error("Invalid input (1)");
    Regex options_allowed("[-a-zA-Z_]+");
    if (!options_allowed.match(options.c_str(), options.length()))
        throw Error("Invalid input (2)");

    // The file parameter (data source name, really) may have escape characters
    // (DODSFilter::initialize calls www2id()) so it's called here and the
    // resulting string is sanitized. I believe that the only escaped
    // character allowed is a space...
    Regex file_allowed("[-a-zA-Z0-9_%]+");
    string unesc_file = www2id(file, "%", "%20");
    if (!file_allowed.match(unesc_file.c_str(), unesc_file.length()))
        throw Error("Invalid input (3)");

    // The allowed set of characters for a constraint is fairly large. One way
    // to validate this input would be to build the DDS first (w/o the ce) and
    // then parse the CE to test it (a DDS is required to parse the DDS).
    // Finally, the validated CE would be used to get a DataDDS (the DDS
    // would not be used for anything other than validation). Instead this code
    // removes all the escaped characters except the spaces (%20) and then
    // filters the expression.
    Regex expr_allowed("[-+a-zA-Z0-9_/%.\\#:,(){}[\\]&<>=~]*");
    string unesc_expr = www2id(expr, "%", "%20");
    if (!expr_allowed.match(unesc_expr.c_str(), unesc_expr.length()))
        throw Error("Invalid input (4)");

    string command = handler + " -o dods " + options
        + " -e " + "\"" + unesc_expr + "\"" + " \"" + unesc_file + "\"";

    DBG(cerr << "DDS Command: " << command << endl);

    FILE *in = popen(command.c_str(), "r");

    try {
        PipeResponse pr(in);

        Connect c("local_pipe");
        c.read_data(dds, &pr);

        pclose(in);
    }
    catch (...) {
        pclose(in);
        throw;
    }

}

/** Write out the given error object. If the Error object #e# is empty, don't
    write anything out (since that will confuse loaddods).

    @author jhrg */

static void output_error_object(const Error & e)
{
    if (e.OK())
        cout << "Error: " << e.get_error_message() << endl;
}

/*
 *     } elsif ( $self->ext() eq "ascii" || $self->ext() eq "asc" ) {
        $options    = "-v " . $self->caller_revision() . " ";
        if ( $self->cache_dir() ne "" ) {
            $options .= "-r " . $self->cache_dir();
        }

        @command = ( $self->asciival(), $options, "-m",
                     "-u", $self->url_text(),
                     "-f", $self->handler(),
                     "--", $self->filename() ); #. "?" . $self->query() );
 */

// Read a DODS data object. The object maybe specified by a URL (which will
// be dereferenceed using Connect, it maybe read from a file or it maybe read
// from stdin. Use `-' in the command line to indicate the next input should
// be read from standard input.

int main(int argc, char *argv[])
{
    GetOpt getopt(argc, argv, "nf:mv:r:u:e:wVh?");
    int option_char;
    bool verbose = false;
    bool handler = false;
    bool mime_header = false;
    bool version = false;
    bool cache = false;
    bool url_given = false;
    bool expr_given = false;

    string expr = "";
    string handler_name = "";
    string server_version = "";
    string cache_dir = "/usr/tmp";
    string url = "";
    string file = "";           // Local file; used with the handler option

#ifdef WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    putenv("_POSIX_OPTION_ORDER=1");    // Suppress GetOpt's argv[] permutation.

    while ((option_char = getopt()) != EOF)
        switch (option_char) {
        case 'f':
            handler = true;
            handler_name = getopt.optarg;
            break;
        case 'm':
            mime_header = true;
            break;
        case 'w':
            verbose = true;
            break;
        case 'v':
            version = true;
            server_version = getopt.optarg;
            break;
        case 'r':
            cache = true;
            cache_dir = getopt.optarg;
            break;
        case 'u':
            url_given = true;
            url = getopt.optarg;
            break;
        case 'e':
            expr_given = true;
            expr = getopt.optarg;
            break;
        case 'V':{
                cerr << "asciival: " << PACKAGE_VERSION << endl;
                exit(0);
            }
        case 'h':
        case '?':
        default:
            usage();
            exit(1);
            break;
        }

        try {
        // After processing options, test for errors. There must be a single
        // argument in addition to any given with the options. This will be
        // either a file or a URL, depending on the options supplied and will
        // be the source from which to read the data.
        if (getopt.optind >= argc) {
            usage();
            throw Error("Expected a file or URL argument.");
        }

        if (handler)
            file = argv[getopt.optind];

        // Normally, this filter is called using either a URL _or_ a handler,
        // a file and a URL (the latter is needed for the form which builds
        // the CE and appends it to the URL). However, it's possible to call
        // the filter w/o a handler and pass the URL in explicitly using -u.
        // In that case url_given will be true and the URL will already be
        // assigned to url.
        if (!handler && !url_given)
            url = argv[getopt.optind];

        // Remove the expression from the URL if no expression was given
        // explicitly and the URL is not empty and contains a '?'
        if (!expr_given && !url.empty() && url.find('?') != string::npos) {
            expr = url.substr(url.find('?') + 1);
            url = url.substr(0, url.find('?'));
        }

        AsciiOutputFactory aof;

        // The version should be read from the handler! jhrg 10/18/05
        DataDDS dds(&aof, "Ascii Data", "DAP2/3.5");

        if (handler) {
            if (verbose)
                cerr << "Reading: " << file << endl;
            string options = "";
            if (version)
                options += string(" -v ") + server_version;
            if (cache)
                options += string(" -r ") + cache_dir;
            read_from_file(dds, handler_name, options, file, expr);
        } else {
            if (verbose)
                cerr << "Reading: " << url << endl;
            read_from_url(dds, url, expr);
        }

        if (mime_header)
            set_mime_text(stdout, dods_data);

        get_data_values_as_ascii(&dds, cout);
    }
    catch(Error & e) {
        DBG(cerr << "Caught an Error object." << endl);
        output_error_object(e);
    }

    catch(exception & e) {
        cerr << "Caught an exception: " << e.what() << endl;
    }

    cout.flush();
    cerr.flush();

    return 0;
}
