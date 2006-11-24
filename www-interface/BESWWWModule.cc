// BESWWWModule.cc

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
 
// (c) COPYRIGHT University Corporation for Atmostpheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>

#include <iostream>

using std::endl ;

#include "BESWWWModule.h"
#include "BESLog.h"

#include "BESWWWNames.h"
#include "BESResponseHandlerList.h"

#include "BESWWWResponseHandler.h"

#include "BESWWWTransmit.h"
#include "BESTransmitter.h"
#include "BESReturnManager.h"
#include "BESTransmitterNames.h"

#include "BESWWWGetCommand.h"

void
BESWWWModule::initialize( const string &modname )
{
    if( BESLog::TheLog()->is_verbose() )
	(*BESLog::TheLog()) << "Initializing OPeNDAP WWW module:" << endl;

    if( BESLog::TheLog()->is_verbose() )
	(*BESLog::TheLog()) << "    adding " << WWW_RESPONSE << " response handler" << endl;
    BESResponseHandlerList::TheList()->add_handler( WWW_RESPONSE, BESWWWResponseHandler::WWWResponseBuilder ) ;

    BESTransmitter *t = BESReturnManager::TheManager()->find_transmitter( BASIC_TRANSMITTER ) ;
    if( t )
    {
	if( BESLog::TheLog()->is_verbose() )
	    (*BESLog::TheLog()) << "    adding basic " << WWW_TRANSMITTER << " transmit function" << endl ;
	t->add_method( WWW_TRANSMITTER, BESWWWTransmit::send_basic_form ) ;
    }

    t = BESReturnManager::TheManager()->find_transmitter( HTTP_TRANSMITTER ) ;
    if( t )
    {
	if( BESLog::TheLog()->is_verbose() )
	    (*BESLog::TheLog()) << "    adding http " << WWW_TRANSMITTER << " transmit function" << endl ;
	t->add_method( WWW_TRANSMITTER, BESWWWTransmit::send_http_form ) ;
    }

    if( BESLog::TheLog()->is_verbose() )
	(*BESLog::TheLog()) << "    adding " << WWW_RESPONSE << " command" << endl;
    BESCommand *cmd = new BESWWWGetCommand( WWW_RESPONSE ) ;
    BESCommand::add_command( WWW_RESPONSE, cmd ) ;
}

void
BESWWWModule::terminate( const string &modname )
{
    if( BESLog::TheLog()->is_verbose() )
	(*BESLog::TheLog()) << "Removing OPeNDAP modules" << endl;

    BESResponseHandlerList::TheList()->remove_handler( WWW_RESPONSE ) ;
    BESCommand::del_command( WWW_RESPONSE ) ;
}

/** @brief dumps information about this object
 *
 * Displays the pointer value of this instance
 *
 * @param strm C++ i/o stream to dump the information to
 */
void
BESWWWModule::dump( ostream &strm ) const
{
    strm << BESIndent::LMarg << "BESWWWModule::dump - ("
			     << (void *)this << ")" << endl ;
}

extern "C"
{
    BESAbstractModule *maker()
    {
	return new BESWWWModule ;
    }
}

