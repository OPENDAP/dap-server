// BESAsciiModule.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004,2005 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.edu>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact University Corporation for Atmospheric Research at
// 3080 Center Green Drive, Boulder, CO 80301
 
// (c) COPYRIGHT University Corporation for Atmospheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>

#include <iostream>

using std::endl ;

#include "BESAsciiModule.h"
#include "BESDebug.h"

#include "BESAsciiNames.h"
#include "BESResponseNames.h"
#include "BESResponseHandlerList.h"
#include "BESAsciiResponseHandler.h"

#include "BESAsciiRequestHandler.h"
#include "BESRequestHandlerList.h"

#include "BESDapService.h"

#include "BESAsciiTransmit.h"
#include "BESTransmitter.h"
#include "BESReturnManager.h"
#include "BESTransmitterNames.h"


void
BESAsciiModule::initialize( const string &modname )
{
    BESDEBUG( "ascii", "Initializing OPeNDAP Ascii module " << modname << endl )

    BESDEBUG( "ascii", "    adding " << modname << " request handler" << endl )
    BESRequestHandler *handler = new BESAsciiRequestHandler( modname ) ;
    BESRequestHandlerList::TheList()->add_handler( modname, handler ) ;

    BESDEBUG( "ascii", "    adding " << ASCII_RESPONSE << " response handler" << endl )
    BESResponseHandlerList::TheList()->add_handler( ASCII_RESPONSE, BESAsciiResponseHandler::AsciiResponseBuilder ) ;

    BESDEBUG( "ascii", "Adding to dap services" << endl )
    BESDapService::add_to_dap_service( ASCII_SERVICE,
				       "OPeNDAP ascii data representation" ) ;

    BESTransmitter *t =
	BESReturnManager::TheManager()->find_transmitter( DAP2_FORMAT ) ;
    if( t )
    {
	BESDEBUG( "ascii", "    adding basic " << ASCII_TRANSMITTER << " transmit function" << endl )
	t->add_method( ASCII_TRANSMITTER, BESAsciiTransmit::send_basic_ascii ) ;
    }

    BESDEBUG( "ascii", "    adding ascii debug context" << endl )
    BESDebug::Register( "ascii" ) ;

    BESDEBUG( "ascii", "Done Initializing OPeNDAP Ascii module " << modname << endl )
}

void
BESAsciiModule::terminate( const string &modname )
{
    BESDEBUG( "ascii", "Cleaning OPeNDAP Ascii module " << modname << endl )

    BESDEBUG( "ascii", "    removing " << modname << " request handler " << endl )
    BESRequestHandler *rh = BESRequestHandlerList::TheList()->remove_handler( modname ) ;
    if( rh ) delete rh ;

    BESDEBUG( "ascii", "    removing " << ASCII_RESPONSE << " response handler" << endl )
    BESResponseHandlerList::TheList()->remove_handler( ASCII_RESPONSE ) ;

    BESTransmitter *t = BESReturnManager::TheManager()->find_transmitter( BASIC_TRANSMITTER ) ;
    if( t )
    {
	BESDEBUG( "ascii", "    removing basic " << ASCII_TRANSMITTER << " transmit function" << endl )
	t->remove_method( ASCII_TRANSMITTER ) ;
    }

    t = BESReturnManager::TheManager()->find_transmitter( HTTP_TRANSMITTER ) ;
    if( t )
    {
	BESDEBUG( "ascii", "    removing http " << ASCII_TRANSMITTER << " transmit function" << endl )
	t->remove_method( ASCII_TRANSMITTER ) ;
    }

    BESDEBUG( "ascii", "Done Cleaning OPeNDAP Ascii module " << modname << endl )
}

/** @brief dumps information about this object
 *
 * Displays the pointer value of this instance
 *
 * @param strm C++ i/o stream to dump the information to
 */
void
BESAsciiModule::dump( ostream &strm ) const
{
    strm << BESIndent::LMarg << "BESAsciiModule::dump - ("
			     << (void *)this << ")" << endl ;
}

extern "C"
{
    BESAbstractModule *maker()
    {
	return new BESAsciiModule ;
    }
}

