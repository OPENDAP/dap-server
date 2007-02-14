
	$Id:README.cgi 15847 2007-02-14 18:16:29Z jimg $

PREFACE

  NOTE

      This README is specifically for the CGI version of the OPeNDAP Data
      Server, which is being replaced by a server which uses the ESG/OPeNDAP
      data server back end (aka BES) along with a front-end component that
      uses Java Servlets. This new server is name 'Hyrax'. See opendap.org
      for more information.
      
  This is a general guide for the OPeNDAP data servers. There is a more 
  detailed guide available online at http://opendap.org/support at the link 
  'Server Installation Guide.'

  The server software is split between two functions; this part of the
  software provides the general capabilities of the server such as basic
  request handling. The server also uses one or more 'handlers' for each
  specific data format. Those handlers are available as separate items from
  our web site or from other sites.

  Version 3.5 and later of the server, described in this file, differs in
  some important ways from previous versions of the server. In version 3.5
  only one executable file is located in a web daemon's CGI binary directory.
  In addition, it's no longer necessary to locate the data in your web
  daemon's DocumentRoot directory. Both of these changes increase server
  robustness.

  Though not absolutely essential, it will be useful to understand the
  structure of the software. You can find general descriptions of the system
  and how the pieces fit together in the User's Guide, available from the
  OPeNDAP home page: http://www.opendap.org/.

  This file describes how to use the software once it has been built and
  installed. For instructions on the build and/or installation process, see
  the file INSTALL.

  See NEWS for recent changes to the server software.

  Updated for version 3.7.3 of the OPeNDAP Server Software.

-----------------------------------------------------------------------------

CONTENTS

Introduction

About WWW Servers

Setting Up a Data Server
Quick Instructions
Serving Compressed Files 
More Sophisticated Installation
  Usage information
  Logging accesses

About Data Security
Using A Secure OPeNDAP Server
Configuring a (Secure) Server

Server Installation Tips
  Tip 1: Which directory is the cgi-bin directory?
  Tip 2: Which directory is used by your WWW server as the document root?
  Tip 3: How can I avoid copying all my data into the document root?
  Tip 4: How do I write a URL for some archive's data?
  Tip 5: How do I add additional descriptive information about my dataset?
  Tip 6: How do I set up a server that limits access to only a few people?
  Tip 7: Is there a registry of datasets?
  Tip 8: Customizing the DODS Directory response (from Joe McLean)
  Tip 9: Secure servers
  Tip 10: Shorten URLs

More Questions...

-----------------------------------------------------------------------------

Introduction

  This is the base software for the OPeNDAP data server. It uses the CGI 1.1
  protocol and requires Apache or a similar web daemon.

  The software here includes three C++ programs which are installed in
  <prefix>/bin and a collection of Perl software install in
  <prefix>/share/dap-server. Finally, the Perl script nph-dods and it's
  configuration file dap-server.rc are installed in
  <prefix>/share/dap-server-cgi. These last two files are to be installed in
  one of the HTTP daemon's CGI binaries directory (you can use the default
  cgi-bin or set up a new one).
  
  We've added a sample configuration file which can be used with the
  Apache 2.x web server. Look at 'opendap_apache.conf.'

-----------------------------------------------------------------------------

About WWW Servers

  We have tested the servers extensively with the Apache HTTP daemon (httpd).
  Other servers will work as along as they support CGI 1.1 standard. However,
  the installation instructions will be slightly different. Don't let this
  dissuade you from installing the servers with another httpd - the process
  is very simple. Other groups have used the NCSA, CERN and Netscape web
  servers successfully.

  If you do not have a World Wide Web server running on the machine you want
  to use as your data server machine, the Apache daemon can be found
  at http://www.apache.org/.

-----------------------------------------------------------------------------

Setting Up a Data Server

Quick Instructions

  First build and install the Server Base Software. See the file INSTALL. You
  may be able to install a binary package and skip the build step.

  Once the Base has been installed, you're ready to configure the software
  and add one or more data handlers. The base software does not include any
  handlers (each is available as a separate package) so we'll use the NetCDF
  File handler as an example.

  To configure the server you can copy the files nph-dods and dap-server.rc
  from the directory $prefix/share/dap-server-cgi to a CGI Binary directory
  for your web daemon. Or, you can edit your web server's configuration file
  and make that directory a CGI bin directory. See SERVER INSTALLATION TIPS 
  section below for help on how to locate your web server's CGI Binary 
  diretcory. Make sure that the nph-dods script is exectuable. Also see the
  opendap_apache.conf. This can be used in lieu of editing your httpd.conf
  file in many cases, should you want to run your OPeNDAP server in its own
  CGI bin directory.

  Previous versions of the OPeNDAP server required that data either be stored
  in your web daemon's DocumentRoot, one of its subdirectories or be
  symbolically linked so that it appeared so (and in the later case a special
  option had to be set for the Apache web daemon). However, the version 3.5
  server provides an option where the data location can be set using the
  data_root parameter of the dap_server.rc file. See below under "The
  dap_server.rc Configuration File" for details. Note that the old behavior
  where data are located in or under DocumentRoot is still supported. Also
  note that the OPeNDAP server's directory browsing functions don't currently
  work when using data_root.

The dap-server.rc Configuration File

  The dap-server.rc configuration file is used to tailor the server to your
  site. The file contains a handful of parameters plus the mappings between
  different data sources (typically files, although that doesn't have to be
  the case) and hander-programs. The format of the configuration files is:
  
     <parameter> <value> [ <value> ... ]

  The configuration file is line-oriented, with each parameter appearing on
  its own line. Blank lines are ignored and the '#' character is used to
  begin comment lines. Comments have to appear on lines by themselves (you
  cannot tack a comment on to the end of a line that contains a parameter).

  The parameters recognized are:

  data_root <path>

  Define this if you do not want the server to assume data are located under
  DocumentRoot. The value <path> should be the fully qualified path to the
  directory which you want to use as the root of your data tree. For example,
  we have a collection of netcdf, hdf, et c., files that we store in
  directories named /usr/local/test_data/data/nc,
  /usr/local/test_data/data/hdf, et c., and we set data_root to
  '/usr/local/test_data'. The value of <path> should _not_ end in a slash.

  Notes for 'data_root': 

  The OPeNDAP server's directory browsing functions do not work when using
  the data_root option. They do still work when locating data under
  DocumentRoot.

  It's not an absolute requirement that this path be a real directory or that
  your data are in files. Data can reside in a relational database, for
  example. In that case the base software will use <path> as a prefix to the
  path part of the URL it receives from the client.).

  timeout <seconds>

  This sets the OPeNDAP server timeout value, in seconds. This is different
  from the httpd timeout. OPeNDAP servers run independently of httpd once the
  initial work of httpd is complete. This ensures that your OPeNDAP server
  does not continue indefinitely if something goes wrong (i.e., a user makes
  a huge request to a database). Default is 0 which means no time out.

  cache_dir <directory>

  When data files are stored in a compressed format such as gzip or UNIX
  compress, the OPeNDAP server first decompresses them and then serves the
  decompressed file. The files are cached as they are decompressed. This
  parameter tells the server where to put that cache. Default: /usr/tmp. In
  the RPM package, this defaults to /var/cache/dap-server.

  cache_size <size in MB>

  How much space can the cached files occupy? This value is given in
  mega bytes. When the total size of all the decompressed files exceeds this
  value, the oldest remaining file will be removed until the size drops below
  the parameter value. If you are serving large files, make sure this value
  is at least as large as the largest file. 

  maintainer <email address>

  The email address of the person responsible for this server. This email
  address will be included in many error messages returned by the server.
  Default: support@unidata.ucar.edu.

  curl <path>

  This parameter is used to set the path to the curl executable. The curl
  command line tool is used to dereference URLs when the server needs to do
  so. In some cases the curl exectuable might not be found be the server when
  checks default directories. This can be a source of considerable confusion
  because the CGI program run from a web daemon used a very restricted PATH
  environment variable, much more restricted than a typical user's PATH.
  Thus, even if you, as teh server installer have curl on your PATH, nph-dods
  may not be able to find the program unless you tell it exactly where to
  look.

  exclude <handler> [ <handler> ... ]

  This is a list of handlers whose regular expressions should *not* be used
  when building the HTML form interface for this server. In general, this
  list should be empty. However, if you have a handler that is bound to a
  regular expression that is very general (such as .* which will match all
  files), then you should list that handler here, enclosing the name in
  double quotes. See the next item about the 'handler' parameter. Default: by
  default, no handlers are excluded.

  handler <regular expression> <handler name>

  The handler parameter is used to match data sources with particular handler
  programs used by the server. In a typical OPeNDAP server setup, the data
  sources are files and the regular expressions choose handlers based on the
  data file's extension. However, this need not be that case. The OPeNDAP
  server actually matches the entire pathname (starting at the HTTP Document
  Root) of the data source when searching for the correct handler to use. The
  regular expressions are Perl regular expressions; the examples in the
  sample dap-server.rc file should get you going.

  Here are the default values for 'handler:'

    handler .*\.(HDF|hdf|EOS|eos)(.Z|.gz|bz2)*$ /usr/local/bin/dap_hdf_handler
    handler .*\.(NC|nc|cdf|CDF)(.Z|.gz|bz2)*$ /usr/local/bin/dap_nc_handler
    handler .*\.(dat|bin)$ /usr/local/bin/dap_ff_handler

  "Regular expressions", advanced pattern-matching languages, are a powerful
  feature of Perl and many other computer programs. Powerful enough, in fact,
  to warrant at least one book about them (Mastering Regular Expressions by
  Jeffrey Friedl, O'Reilly, 1997). For a complete reference online--not a
  particularly good place to learn about them for the first time--see
  http://www.perldoc.com/perl5.6/pod/perlre.html.

  Briefly, however, the above patterns test whether a filename is of the form
  'file.ext.comp', where 'comp' (if present) is Z or gz, and 'ext' is one of
  several possible filename extensions that might indicate a specific storage
  format. file.

  If these default rules will not work for your installation, you can rewrite
  them. For example, if all your files are HDF files, you could replace the
  default configuration file with one that looks like this:

    handler .* dap_hdf_handler

  The .* pattern matches all possible names, and indicates that whatever the
  name of the file sought, the HDF handler is the one to use.

  If you have a situation where all the files in a particular directory
  (whatever its extension) are to be handled by the DSP service programs,
  and all other files served are JGOFS files, try this:

    handler \/dsp_data\/.* dap_dsp_handler
    handler .* dap_jg_handler

  The rules are applied in order, and the first rule with a successful match
  returns the handler that will be applied. The above set of rules implies
  that everything in the dsp_data directory will be handled with the DSP
  service programs, and everything else will be handled with the JGOFS
  programs.

Serving Compressed Files 

  Here is some important configuration information for people serving data
  from compressed files. If your server is not handling compressed files
  correctly, changing the options as described here will likely fix the
  problem.

  When the OPeNDAP software is used to serve compressed files (e.g., files
  compressed using gzip), the files are first decompressed and then stored in
  a cache; data served are read from those cached files. The location of the
  cache directory is /usr/tmp by default. This can be changed by editing
  dap-server.rc and changing the value of cache_dir. The software is set by
  default to limit the size of this directory to 500 MB. However, if you're
  serving large files, or are experiencing a large volume of traffic, you
  should increase this. To do so, edit the value of of the cache_size, also
  in dap-server.rc. The cache size is given in MB, so changing the 500 to
  1000 would increase the cache size from 500MB to 1GB.

  Finally, the decompression software uses the gzip program to do its work.
  If your computer does not have the gzip program in its /bin directory
  you'll need to edit the DODS_Cache.pm so that the correct version of gzip
  is used. Look in that file for "/bin/gzip" and replace that text with the
  correct pathname. To figure out where gzip is on you computer, type 'which
  gzip' in a shell. As of version 3.5.2 the server will also decompress files
  compressed using bzip2. If the bizip2 utility cannot be found by the
  DOSD_Cache.pm module, look for /bin/bzip2 and replace that with the correct
  pathname for your server host.

Logging accesses

  Since an HTTP daemon logs all accesses, every access to data through your
  OPeNDAP server is logged. However, to collect information about
  project-wide data use, that information needs to be collated. We have built
  an optional module that scans your daemon's logs for just the information
  about OPeNDAP activity. This feature is disabled by default.

  This optional module provides for remote access to this OPeNDAP-only
  information using the URL 'http://<your host>/<your server>/stats'. The
  information can be accessed by your computer and some computers at Unidata,
  URI and OPeNDAP, using tools like a browser or wget.

  You can configure the remote stat access module by hand fairly easily. In
  the distributions etc directory, edit the file called
  EXAMPLE_OPENDAP_STATISTICS. This file holds the pathname to you server's
  access and error logs as well as its name. Once you have edited the file,
  save is as 'OPENDAP_STATISTICS' and copy that file to the directory that
  contains the OPeNDAP server (i.e., nph-dods).

  If you are using virtual hosts with the web deamon that runs the OPeNDAP
  server, the remote stats access module can only be configured to provide
  access to one set of log files.

ABOUT DATA SECURITY

There are two levels of security which the OPeNDAP server supports: Domain
restrictions and user restrictions. In conjunction with a World Wide Web
server, access to the OPeNDAP server can be limited to a specific group of
users (authenticated by password), specific machine(s) or a group of machines
within a given domain or domains.

Notes

    DAP 3.4 onward includes support for Digest authentication, which
    significantly increases the robustness of password access.

    DAP versions 3.2 and greater software contains significant improvements
    in the way password authentication is handled. Older versions of the DODS
    clients prompted for the password with each and every interaction between
    client and server. Now credentials may be embedded in URLs and are
    remembered and reused for the duration of a session.

The security features of the OPeNDAP server depends heavily on the underlying
WWW daemon because we felt this was the best way to solve the thorny problem
of ensuring only authorized users accessed data. By using the daemon's
authorization software we are ensuring that the security checks used by the
server have been tested by many many sites. In addition, WWW daemons already
support a very full set of security features and many system administrators
are comfortable and confidant with them. The tradeoff with using the web
daemon's security system for our servers is that two security settings must
be made for each group of data to be configured and more than one copy of the
nph-dods CGI program and dap-server.rc configuration file may be needed even
if you're serving only one type of data.

Because the security features rely almost entirely on the host machine's WWW
server, the steps required to install a secure the server will vary depending
on the WWW server used. Thus, before installing a secure server, check over
your WWW server's documentation to make sure it provides the following
security features: Access limits to files in the document root on a per user
and/or per machine basis, and; The ability to place CGI scripts.

IMPORTANT

    Because security features are used to protect sensitive or otherwise
    important information, once set-up they should be tested until you are
    comfortable that they work. You should try accessing from at least one
    machine that is not allowed to access your data. If you would like, we
    will try to access your data, assuming that our machines are among those
    not allowed, to help you evaluate your set-up.

    Since the security features are provided by a WWW server, it is highly
    likely that they are functional and extensively tested. While problems
    with these features have shown up in the past (e.g., the Netscape SSL
    server bug) they are generally fixed quickly. Thus there is good reason
    to assume that your data are safe if you choose to set-up your server as
    a secure one. However, *there is a chance* that a defect in the WWW
    server software will allow unauthorized people access; how big that
    chance is depends on the WWW server software you use and how extensively
    its security features are tested. That level of testing is completely
    beyond our control.

It is important to distinguish securing a server from securing data. If data
are served, then those data may also be accessible through a web browser (see
the note about the new data_root configuration parameter). If so the data
themselves need to be stored in directories that have limited access. If all
data access will take place through the server this limitation can exclude
all access *except* the local machine. This is the case because some the
server's directory function requires being able to read the data through the
local host's web server.

It bears repeating: If you're serving sensitive information with the OPeNDAP
server and those data are located under the WWW daemon's DocumentRoot, that
information is accessible two ways: via the OPeNDAP server and through the
WWW server. You need to make sure *both* are protected.

About the 'data_root' parameter

With version 3.5 of the OPeNDAP server, you can use the 'data_root' parameter
in the dap-server.rc configuration file to serve data that are *not*
accessible using your WWW server. This simplifies securing the data but has
the drawback that the directory response is not supported by the OPeNDAP
server. 

About serving both limited- and open-access data from the same server

In the past it was possible to install two or more OPeNDAP servers on a
computer and assign different protections to each one. However, in practice
this has proven to be very hard for to configure correctly. In many cases
where this feature was used, a secure server was setup up for one group of
data while an open server was set up for another. It was often the case that
all the data were accessible using the open server! Thus, if you need to
secure some data, it is best to host all the sensitive information on one
machine and put other data on a second machine with an open-access server. If
you *must* run two or more servers from the same physical host, we suggest
that you configure your web server to see two (or more) virtual hosts. This
will provide the needed separation between the groups of data.

USING A SECURE OPeNDAP SERVER

Using a secure sever is transparent if the server is configured to allow
access based on hosts or domains. Give the URL to a client; the server will
respond by answering the request if allowed or with an error message if
access is not allowed.

Accessing a server which requires password authentication is a little
different and varies depending on the type of client being used. All OPeNDAP
clients support passing the authentication information along with the URL. To
do this add `<username>:<password>@' before the machine name in a URL. For
example, suppose I have a secure server set up on `test.opendap.org' and the
user `guest' is allowed access with the password `demo'. A URL for that
server would start out:

    http://guest:demo@test.opendap.org/...

For example,

    http://guest:demo@test.opendap.org/secure/nph-dods/sdata/nc/fnoc1.nc.info

will return the info on the data set fnoc1.nc from a secure server. You
cannot access the data without including the username and password `guest'
and `demo'.

Some clients will pop up a dialog box and prompt for the username and
password. Netscape, and some other web browsers, for example, will do this.
Similarly, some DODS clients may also popup a dialog.

CONFIGURING A SERVER

In the following I'll use the Apache 1.3.12 server as an example (also tested
on Apache 2.0.40, 07/25/03 jhrg) and describe how to install a server which
limits access to a set of users. While this example is limited to the Apache
server, it should be simple to perform the equivalent steps for any other WWW
server that supports the set of required security features (See ABOUT DATA
SECURITY).

Note: 
    I have installed a secured server using dap-server 3.5 and Apache
    2.0.54 and have found that installing the dap-server.rc and nph-dods
    files in their own directory, making that a ScriptAlias directory and
    using the directives shown below works. That is, there's no need with
    Apache 2.x to place the CGI under the document root. Two addition bits of
    information regarding this option: 1. The 'Options' directive is not
    needed since the directory is named as a ScriptAlias; and 2. There's no
    need to user the '.cgi' extension on the nph-dods executable.

    I also have found that using the ScriptAliasMatch directive can be used
    to shorten the URL quite a bit. I included the following in httpd.conf:

      ScriptAliasMatch ^/secure(.*) "/usr/local/secure-3.5/nph-dods$1

    so that a url like http://test.opendap.org/secure/data/nc/fnoc1.nc will
    access the fnoc1.nc dataset using the secured server.

I. Create a directory for the server.

To limit access to a  dataset to particular machine, begin by creating a
special directory for the server. This maybe either an additional CGI bin
directory or a directory within the web server's document root. In this
example, I chose the latter.

    cd /var/www/html/
    mkdir secure

II. Establish access limitations for that directory.

Establish the access limitations for this directory. For the Apache server,
this is done either by adding lines to the server's httpd.conf file or by
using a per-directory file. Note: The use of per-directory access limit files
is a configurable feature of the Apache server; look in the server's
httpd.conf file for the value of the AccessFileName resource.

I modified Apache's httpd.conf file so that it contains the following:

    # Only valid users can use the server in secure. 7/6/2000 jhrg
    <Directory /var/www/html/secure>
	Options ExecCGI Indexes FollowSymLinks

	Order deny,allow
	Deny from all
	# ALLOW SERVER (IP OF SERVER) MACHINE TO REQUEST DATA ITSELF
	Allow from __YOUR_SERVER_HERE__ 
	Require valid-user
	# ALL VISITORS NEED USERNAME AND PASS BUT NOT SERVER
	Satisfy any

	AuthType Basic 
	AuthUserFile /etc/httpd/conf/htpasswd.users 
	AuthGroupFile /etc/httpd/conf/htpasswd.groups
	AuthName "Secure server"
    </Directory>

    # Protect the directory used to hold the secure data.
    <Directory /var/www/html/sdata>
	Options Indexes

	Order deny,allow
	Deny from all
	# ALLOW SERVER (IP OF SERVER) MACHINE TO REQUEST DATA ITSELF
	Allow from __YOUR_SERVER_HERE__ 
	Require valid-user
	# ALL VISITORS NEED USERNAME AND PASS BUT NOT SERVER
	Satisfy any

	AuthType Basic 
	AuthUserFile /etc/httpd/conf/htpasswd.users 
	AuthGroupFile /etc/httpd/conf/htpasswd.groups
	AuthName "Secure data"
    </Directory>

and

    ScriptAlias /secure/ "/var/www/html/secure/"

The first group of lines establishes the options allowed for the `secure'
directory, including that it can contain CGI programs. The lines following
that establish that only users in the Apache password file can access the
contents of the directory, with the exception that this server is allowed to
access the directory without authentication. This last bit is important
because OPeNDAP servers sometimes make requests to themselves (e.g., when
generating the directory response) but don't pass on the authentication
information.*

Regarding the 'Satisfy any" directive, Brock Murch says: 

    I thought that one needed an "Allow from all" since I want my users to
    connect from anywhere, which would have necessitated a "satisfy all"
    since I needed the passwd authentication as well. I didn't know that the
    "Deny from all" would still allow anyone in so long as the AuthType etc
    was included and authentication took place. Since this is the case a
    "satisfy any" will do as I have denied all ip access except for the
    server itself. The second group of lines secure the data itself from
    accesses which bypass the DODS server.

The ScriptAlias line tells Apache that executable files in the directory are
CGIs. You can also do this by renaming the nph-dods script to nph-dods.cgi
and making sure httpd.conf contains the line:

    AddHandler cgi-script .cgi

The AuthType directive selects the type of authentication used. Apache 2.0
supports 'Basic' and 'Digest' while other servers may also support
GSS-Negotiate and NTLM. Version 3.4 onward of the DAP software supports all
these authentication schemes, although only Basic and Digest have been
thoroughly tested. Configuration of Apache 2.0 for Digest authentication is
slightly different then for Basic authentication, but is explained well in
Apache's on line documentation.

III. Copy the server into the new directory.

Copy the CGI program (nph-dods) and the server configuration file to the
newly created directory. Note that if you're using the extension `.cgi' to
tell Apache that nph-dods is a CGI you must rename nph-dods to nph-dods.cgi.
If you forget to do that then you will get a Not Found (404) error from the
server and debugging information generated by the DODS server won't appear in
Apache's error_log even if it has been turned on.

Server Installation Tips

  The following tips are mostly specific to the Apache server. For other web
  servers, this advice may be only of general, or possibly little, help.

Tip 1: Which directory is the cgi-bin directory?

  Look in the server's `httpd.conf' file for a line like:

    ScriptAlias /cgi-bin/ /var/www/cgi-bin/

  The option ScriptAlias defines where CGI programs may reside. In this case
  they are in the directory /var/www/cgi-bin/. With this script alias in
  effect, URLs with `cgi-bin' in their path will automatically refer to
  programs in the specified directory. You might want to put the OPeNDAP
  server in its own CGI directory named 'opendap' or 'dap', although that's
  not a requirement.

Tip 2: Which directory is used by your WWW server as the document root?

  Look in the http.conf file for a line like:

    DocumentRoot /var/www/html

  This says that the document root directory is /var/www/html. URLs that
  contain only the machine name and a file name, such as:
  
    http://test.opendap.org/info.txt

  refer to files in the document root directory.  For example, with the
  above DocumentRoot declaration, and the above URL, the file specified is:

    /var/www/html/info.txt

Tip 3: How can I avoid copying all my data into the document root?  Can't I
  link to it?

  This tip is old and, while still true, the 3.5 server provides a better
  solution for some sites: Use the data_root configuration parameter in
  dap-server.rc. See the section on the Configuration File for more
  information...

  By default many servers disable soft (symbolic) links that point out of the
  directories that fall under the web server's document root. This is not
  done for inconvenience, it is a security feature (really). To disable this
  feature, and allow links to other directories, you'll have to edit the
  httpd.conf file, and add the `FollowSymLinks' option to the directory trees
  which will contain the links to data. This might sound more complex than it
  is.

  For example, suppose that you have a large dataset stored in a directory
  named `/archive1'. Under that directory are many files and taken together
  they are far too large to copy into the WWW server's document root. First
  look in access.conf and see that FollowSymLinks is set for the WWW server's
  document root. It will look like:

    <Directory /usr/local/spool/http>
    Options Indexes FollowSymLinks

  There may be some blank lines and comments between the two.  If
  `FollowSymLinks' is not on the `Options' line, add it, just like in the
  example above. Once that is done, create a symbolic link from within your
  WWW server's document root to `/archive1' (use `ln -s'). You're done.

Tip 4: How do I write a URL for some archive's data?

  One question about your server you will likely be asked is "How do I
  write one of those URLs for your data?". Here's how to answer that
  question. The URLs have the form:

    http://<machine name>/<cgi dir name>/<pathname>

  Suppose that you have copied the HDF server to /var/www/opendap
  and soft linked a set of files in /var/www/html/data/hdf on the machine
  `test.opendap.org'. Also suppose a partial directory listing of that is:

    S2000415.HDF
    S1700101.HDF.gz
    S3096277.HDF.Z

  A URL that references the first file would be:
  
    http://test.opendap.org/opendap/nph-dods/data/hdf/S2000415.HDF

  The part of the URL that reads `opendap/nph-dods' selects the server and
  the section `data/hdf/S2000415.HDF' chooses the file. Note that the WWW
  server's document root is /var/www/html/ and the files are stored in a
  subdirectory within that named `data/hdf'. On the Web URLs, are rooted at
  the DocumentRoot directory so the `/var/www/html/' part of the pathname is
  implicit and should not be part of the URL. Since the directory `data/hdf'
  is under the document root directory it *is* included in the URL.

Tip 5: How do I add additional descriptive information about my dataset?

  The best way is to use the `usage' service.  This service returns
  information to the client about the server, but it can also return an HTML
  document you write to describe your server or any dataset served.

  To describe any special features of a particular data server, write an
  HTML document describing that server, and put it in a file named
  <root>.html in the cgi-bin directory that holds the server programs. The
  only special thing about this file is that you should include only those
  HTML tags that would fall between the <body> and </body> tags. Thus it
  should not contain the <html> <head> <body> or their matching
  counterparts.

  To provide HTML for a class of files you'd create an HTML file whose name
  is based on the names of the data files you want to describe. For example,
  a file that would be used for all the HDF files used in the previous
  section's example would be S.html. This file should be located in the
  directory where the data is located.  You could also create an S20004.html
  and an S20005.html to distinguish between the different sets of HDF files.

  Users access this information by appending `.html' to a URL. For example to
  get the HTML page for the URL used in the previous section, you'd type:

    http://test.opendap.org/opendap/nph-hdf/data/hdf/S2000415.HDF.html

  Note: the Usage server will return important information about your data
    set even if you do not write custom HTML files for it. IF you do write
    those files they will be concatenated with the default information
    returned by the usage server.

Tip 6: How do I set up a server that limits access to only a few people?

  Briefly, it is fairly easy in practice to set up servers so that only a
  particular group can access data using them. The specific mechanisms you
  have at your disposal will depend on the WWW server you use. See the file
  README-security (found in the DODS/etc directory) for information on
  installing `secure'servers with Apache.

Tip 7: Is there a registry of datasets?

  You can register your data set(s) with the us using our web page. Goto
  http://www.opendap.org/data/addtolist.html. Our list of available datasets
  can be found under the 'support' heading of our web page by following the
  'Sources of data' link in the left-hand panel. There's also a link to the
  list of data sources on the front-page left-hand panel.

  You can also register your dataset with the Global Change Master Directory
  (http://gcmd.gsfc.nasa.gov/); they maintain a list registered
  DODS/NVODS/OPeNDAP data sets.

  The GCMD has a special portal dedicated to datasets accessible using OPeNDAP
  servers at http://gcmd.gsfc.nasa.gov/Data/portals/dods/.

Tip 8: Customizing the Directory response (from Joe McLean)

  After messing around with Apache for a few hours, I have found some
  configuration directives that may be of use in beautifying the
  appearance of a directory on your user's Browser.

  example: http://ferret.pmel.noaa.gov/cgi-bin/nph-dods/data/PMEL/

  In your Apache httpd.conf file add the following:

  <Directory /path-to-dods(without cgi-bin or nph-dods)/data>
      Options +Indexes +FollowSymLinks
      IndexOptions FancyIndexing
      IndexOptions +FoldersFirst +NameWidth=* +IgnoreCase
      IndexOptions +SuppressDescription
      DefaultIcon /path-to-icons/dods.gif
      IndexOptions +IconHeight=20 +IconWidth=20
  </Directory>

  I copied the dods logo from upper left hand corner of
  http://www.unidata.ucar.edu/packages/dods/home/getStarted/
  called it dods.gif and stuck it in my apache icons directory
  (in httpd.conf grep for Icon - something should show you the path)

  Information for Apache
  IndexOptions: http://httpd.apache.org/docs/mod/mod_autoindex.html#indexoptions

  Options: http://httpd.apache.org/docs/mod/core.html#options

  DefaultIcon: http://httpd.apache.org/docs/mod/mod_autoindex.html#defaulticon

Tip 9: Secure servers

  Using the per-directory limit files makes changing limits easier since the
  server reads those every time it accesses the directory, while changes made
  to the httpd.conf file are not read until the server is restarted or sent the
  HUP signal. However, using httpd.conf for your security configuration seems
  more straightforward since all the information is in one place.

  If the protections are set up so that it is impossible for the server host to
  access the data and/or the OPeNDAP server itself, then an infinite loop can
  result. This can be frustrating to debug, but if you see that accesses
  generate an endless series of entries in the access_log file, it is likely
  that is the problem. Make sure that you have `allow from <server host name>'
  set for both the directory that holds the OPeNDAP server and that holds the
  data. Also make sure that the server's name is set to the full name of the
  host.

  Configuring a secure server can be frustrating if you're testing the server
  using a web browser that remembers passwords. You can turn this feature off
  in some browers. Also, the getdap tool supplied with OPeNDAP's libdap (which
  is required to build the server) can be useful to test the server since it
  will not remember passwords between runs.

Tip 10: Shorten URLs

  Use the ScriptAliasMatch directive in Apache 2.x to shorten URLs. Suppose
  you have a server installed in /usr/local/opendap-3.5/share/dap-server-cgi/.
  The following:

    ScriptAliasMatch ^/dap(.*) "/usr/local/opendap-3.5/share/dap-server-cgi/
    nph-dods$1
 
  (without the line break) will cause a URL like:

  http://test.opendap.org/dap/data/nc/fnoc1.nc to work where with just the
  ScriptAlias 

    ScriptAlias /opendap-3.5/ "/usr/local/opendap-3.5/share/dap-server-cgi/"

  users would need the more cumbersome:

    http://test.opendap.org/opendap-3.5/nph-dods/data/nc/fnoc1.nc      

More Questions...

  There are several sources of further information about DODS/NVODS/OPeNDAP:

    o The OPeNDAP home page: http://www.opendap.org/.

    o The users manual, available in HTML and PDF, on the home page.

    o User support by email: support@unidata.ucar.edu

Thanks

* Brock Murch <bmurch@marine.usf.edu> worked out some thorny configuration
  details for securing the Apache/DODS/OPeNDAP combination. 
    