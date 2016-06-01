// BESAsciiTransmit.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004,2005 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.edu> and Jose Garcia <jgarcia@ucar.edu>
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact University Corporation for Atmospheric Research at
// 3080 Center Green Drive, Boulder, CO 80301

// (c) COPYRIGHT University Corporation for Atmospheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#include <memory>

#include <BaseType.h>
#include <Sequence.h>
#include <ConstraintEvaluator.h>
#include <D4Group.h>
#include <DMR.h>
//#include <D4CEDriver.h>
#include <D4ConstraintEvaluator.h>
#include <crc.h>
#include <InternalErr.h>
#include <util.h>
#include <escaping.h>
#include <mime_util.h>

#include <BESDapNames.h>
#include <BESDataNames.h>
#include <BESDapTransmit.h>
#include <BESContainer.h>
#include <BESDataDDSResponse.h>
#include <BESDMRResponse.h>
#include <BESDapResponseBuilder.h>

#include <BESError.h>
#include <BESDapError.h>
#include <BESForbiddenError.h>
#include <BESInternalFatalError.h>
#include <DapFunctionUtils.h>

#include <BESDebug.h>

#include "BESAsciiTransmit.h"
#include "get_ascii.h"
#include "get_ascii_dap4.h"

using namespace dap_asciival;

BESAsciiTransmit::BESAsciiTransmit() :
        BESBasicTransmitter()
{

    add_method(DATA_SERVICE, BESAsciiTransmit::send_basic_ascii);
    add_method(DAP4DATA_SERVICE, BESAsciiTransmit::send_dap4_csv);

}

// This version of send_basic_ascii() should work for server functions,
// including ones that are used in combination with a selection expression.
// This functionality has not yet been added to the DAP4 version of the
// method, however.
//
// I have some questions regarding how caching will work in this function.
//
// Since this 'transmitter' pattern is pretty common in our code, I think
// it would be good if BESDapResponseBuilder supported it with a set of
// methods that could be used to implement the logic uniformly.
void BESAsciiTransmit::send_basic_ascii(BESResponseObject *obj, BESDataHandlerInterface &dhi)
{
    BESDEBUG("ascii", "BESAsciiTransmit::send_basic_ascii() - BEGIN" << endl);

    BESDataDDSResponse *bdds = dynamic_cast<BESDataDDSResponse *>(obj);
    if (!bdds) throw BESInternalFatalError("Expected a BESDataDDSResponse instance", __FILE__, __LINE__);

    try { // Expanded try block so all DAP errors are caught. ndp 12/23/2015

        DataDDS *dds = bdds->get_dds();
        ConstraintEvaluator &eval = bdds->get_ce();
        ostream &o_strm = dhi.get_output_stream();

        if (!o_strm)
            throw BESInternalError("Output stream is not set, can not return as ASCII", __FILE__, __LINE__);

        // ticket 1248 jhrg 2/23/09
        string ce = www2id(dhi.data[POST_CONSTRAINT], "%", "%20%26");
        eval.parse_constraint(ce, *dds);

        dds->tag_nested_sequences(); // Tag Sequences as Parent or Leaf node.

        // Is the requested stuff too big?
        int response_size_limit = dds->get_response_limit(); // use the original DDS
        if (response_size_limit != 0 && dds->get_request_size(true) > response_size_limit) {
            string msg = "The Request for " + long_to_string(dds->get_request_size(true) / 1024)
                    + "KB is too large; requests for this user are limited to "
                    + long_to_string(response_size_limit / 1024) + "KB.";
            throw Error(msg);
        }

        // now we need to read the data
        BESDEBUG("ascii", "BESAsciiTransmit::send_basic_ascii() - Reading data into DataDDS" << endl);

        // Handle *functional* constraint expressions specially
        if (eval.function_clauses()) {
            BESDEBUG("ascii", "BESAsciiTransmit::send_basic_ascii() - Processing functional constraint clause(s)." << endl);
            DataDDS *tmp_dds = eval.eval_function_clauses(*dds);
            delete dds;
            dds = tmp_dds;
            bdds->set_dds(dds);
            // This next step utilizes a well known function, promote_function_output_structures()
            // to look for one or more top level Structures whose name indicates (by way of ending
            // with "_uwrap") that their contents should be promoted (aka moved) to the top level.
            // This is in support of a hack around the current API where server side functions
            // may only return a single DAP object and not a collection of objects. The name suffix
            // "_unwrap" is used as a signal from the function to the the various response
            // builders and transmitters that the representation needs to be altered before
            // transmission, and that in fact is what happens in our friend
            // promote_function_output_structures()
            promote_function_output_structures(dds);
        }
        else {
            // Iterate through the variables in the DataDDS and read
            // in the data if the variable has the send flag set.
            for (DDS::Vars_iter i = dds->var_begin(); i != dds->var_end(); i++) {
                if ((*i)->send_p()) {
                    (*i)->intern_data(eval, *dds);
                }
            }
        }
        // Now that we have constrained the DataDDS and read in the data,
        // send it as ascii
        DataDDS *ascii_dds = datadds_to_ascii_datadds(dds);
        get_data_values_as_ascii(ascii_dds, dhi.get_output_stream());
        dhi.get_output_stream() << flush;
        delete ascii_dds;
    }
    catch (Error &e) {
        throw BESDapError("Failed to get values as ascii: " + e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
    }
    catch (BESError &e) {
        throw;
    }
    catch (...) {
        throw BESInternalError("Failed to get values as ascii: Unknown exception caught", __FILE__, __LINE__);
    }
}

/**
 * Transmits DAP4 Data as Comma Separated Values
 */
void BESAsciiTransmit::send_dap4_csv(BESResponseObject *obj, BESDataHandlerInterface &dhi)
{
    BESDEBUG("ascii", "BESAsciiTransmit::send_dap4_csv" << endl);

    BESDMRResponse *bdmr = dynamic_cast<BESDMRResponse *>(obj);
    if (!bdmr) throw BESInternalFatalError("Expected a BESDMRResponse instance.", __FILE__, __LINE__);

    DMR *dmr = bdmr->get_dmr();

    string dap4Constraint = www2id(dhi.data[DAP4_CONSTRAINT], "%", "%20%26");
    string dap4Function = www2id(dhi.data[DAP4_FUNCTION], "%", "%20%26");

    // Not sure we need this...
    dhi.first_container();

    try {
        // @TODO Handle *functional* constraint expressions specially
        // Use the D4FunctionDriver class and evaluate the functions, building
        // an new DMR, then evaluate the D4CE in the context of that DMR.
        // This might be coded as "if (there's a function) do this else process the CE".
        // Or it might be coded as "if (there's a function) build the new DMR, then fall
        // through and process the CE but on the new DMR". jhrg 9/3/14

        if (!dap4Constraint.empty()) {
            D4ConstraintEvaluator d4ce(dmr);
            bool parse_ok = d4ce.parse(dap4Constraint);
            if (!parse_ok) throw Error(malformed_expr, "Constraint Expression (" + dap4Constraint + ") failed to parse.");
        }
        else {
            dmr->root()->set_send_p(true);
        }

        print_values_as_ascii(dmr, dhi.get_output_stream());
        dhi.get_output_stream() << flush;
    }
    catch (Error &e) {
        throw BESDapError("Failed to return values as ascii: " + e.get_error_message(), false, e.get_error_code(),__FILE__, __LINE__);
    }
    catch (BESError &e){
        throw;
    }
    catch (...) {
        throw BESInternalError("Failed to return values as ascii: Unknown exception caught", __FILE__, __LINE__);
    }

    BESDEBUG("ascii", "Done BESAsciiTransmit::send_dap4_csv" << endl);
}

