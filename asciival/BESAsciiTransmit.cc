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
    BESDEBUG("ascii", "BESAsciiTransmit::send_base_ascii; modified" << endl);

    BESDataDDSResponse *bdds = dynamic_cast<BESDataDDSResponse *>(obj);
    if (!bdds) throw BESInternalFatalError("Expected a BESDataDDSResponse instance", __FILE__, __LINE__);

    try { // Expanded try block so DAP all DAP errors are caught. ndp 12/23/2015

        DataDDS *dds = bdds->get_dds();
        ConstraintEvaluator &ce = bdds->get_ce();

        dhi.first_container();

        string constraint = www2id(dhi.data[POST_CONSTRAINT], "%", "%20%26");
        BESDEBUG("ascii", "parsing constraint: " << constraint << endl);

        BESDapResponseBuilder rb;

        rb.split_ce(ce, constraint);

        // If there are functions, parse them and eval.
        // Use that DDS and parse the non-function ce
        // Serialize using the second ce and the second dds
        if (!rb.get_btp_func_ce().empty()) {
            BESDEBUG("ascii", "BESAsciiTransmit::send_base_ascii - Found function(s) in CE: " << rb.get_btp_func_ce() << endl);

            // Define a local ce evaluator so that the clause(s) from the function parse
            // won't get treated like selection clauses later on when serialize is called
            // on the DDS (fdds)
            ConstraintEvaluator func_eval;

            // FIXME Does caching work outside of the DAP module?
#if 0
            if (responseCache()) {
                BESDEBUG("ascii", "BESAsciiTransmit::send_base_ascii - Using the cache for the server function CE" << endl);
                fdds = rb.responseCache()->cache_dataset(dds, get_btp_func_ce(), this, &func_eval, cache_token);
            }
            else {
                BESDEBUG("ascii", "BESAsciiTransmit::send_base_ascii - Cache not found; (re)calculating" << endl);
                func_eval.parse_constraint(get_btp_func_ce(), dds);
                fdds = func_eval.eval_function_clauses(dds);
            }
#endif
            func_eval.parse_constraint(rb.get_btp_func_ce(), *dds);
            DataDDS *fdds = func_eval.eval_function_clauses(*dds);

            // Server functions might mark variables to use their read()
            // methods. Clear that so the CE in d_dap2ce will control what is
            // sent. If that is empty (there was only a function call) all
            // of the variables in the intermediate DDS (i.e., the function
            // result) will be sent.
            fdds->mark_all(false);

            ce.parse_constraint(rb.get_ce(), *fdds);

            fdds->tag_nested_sequences(); // Tag Sequences as Parent or Leaf node.

            int response_size_limit = dds->get_response_limit(); // use the original DDS
            if (response_size_limit != 0 && fdds->get_request_size(true) > response_size_limit) {
                string msg = "The Request for " + long_to_string(fdds->get_request_size(true) / 1024)
                        + "KB is too large; requests for this user are limited to "
                        + long_to_string(response_size_limit / 1024) + "KB.";
                throw Error(msg);
            }

            // Now we have the correct values in fdds, so set the BESResponseObject so
            // that it will reference that. At a minimum, its dtor will free the object.
            // So, delete the initial DataDDS*
            bdds->set_dds(fdds);
            delete dds;
            dds = fdds;

            // FIXME Caching broken outside of the DAP module?
#if 0
            if (!store_dap2_result(data_stream, dds, eval)) {
                serialize_dap2_data_dds(data_stream, *fdds, eval, true /* was 'false'. jhrg 3/10/15 */);
            }

            if (responseCache()) responseCache()->unlock_and_close(cache_token);
#endif
        }
        else {
            BESDEBUG("ascii", "BESAsciiTransmit::send_base_ascii - Simple constraint" << endl);

            ce.parse_constraint(rb.get_ce(), *dds); // Throws Error if the ce doesn't parse.

            dds->tag_nested_sequences(); // Tag Sequences as Parent or Leaf node.

            if (dds->get_response_limit() != 0 && dds->get_request_size(true) > dds->get_response_limit()) {
                string msg = "The Request for " + long_to_string(dds->get_request_size(true) / 1024)
                        + "KB is too large; requests for this user are limited to "
                        + long_to_string(dds->get_response_limit() / 1024) + "KB.";
                throw Error(msg);
            }

            // FIXME Caching...
#if 0
            if (!store_dap2_result(data_stream, dds, eval)) {
                serialize_dap2_data_dds(data_stream, dds, eval);
            }
#endif
        }

        for (DDS::Vars_iter i = dds->var_begin(); i != dds->var_end(); i++) {
            if ((*i)->send_p()) {
                BESDEBUG("ascii", "BESAsciiTransmit::send_base_ascii; call to intern_data() for '" << (*i)->name() << "'" << endl);
                (*i)->intern_data(ce, *dds);
            }
        }

        // Now that we have constrained the DataDDS and read in the data,
        // send it as ascii
        DataDDS *ascii_dds = datadds_to_ascii_datadds(dds);

        get_data_values_as_ascii(ascii_dds, dhi.get_output_stream());

        dhi.get_output_stream() << flush;
        delete ascii_dds;
    }
    catch (InternalErr &e) {
        throw BESDapError("Failed to get values as ascii: " + e.get_error_message(), true, e.get_error_code(), __FILE__,  __LINE__);
    }
    catch (Error &e) {
        throw BESDapError("Failed to get values as ascii: " + e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
    }
    catch (BESError &e){
        throw BESError("Failed to get values as ascii: " + e.get_message(), e.get_error_type(), __FILE__, __LINE__);
    }
    catch (...) {
        throw BESInternalFatalError("Failed to get values as ascii: Unknown exception caught", __FILE__, __LINE__);
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
        // Handle *functional* constraint expressions specially
        // Use the D4FunctionDriver class and evaluate the functions, building
        // an new DMR, then evaluate the D4CE in the context of that DMR.
        // This might be coded as "if (there's a function) do this else process the CE".
        // Or it might be coded as "if (there's a function) build the new DMR, then fall
        // through and process the CE but on the new DMR". jhrg 9/3/14

        if (!dap4Constraint.empty()) {
            D4ConstraintEvaluator d4ce(dmr);
            d4ce.parse(dap4Constraint);
        }
        else {
            dmr->root()->set_send_p(true);
        }

        print_values_as_ascii(dmr, dhi.get_output_stream());
        dhi.get_output_stream() << flush;
    }
    catch (InternalErr &e) {
        throw BESDapError("Failed to return values as ascii: " + e.get_error_message(), true, e.get_error_code(),
                __FILE__, __LINE__);
    }
    catch (Error &e) {
        throw BESDapError("Failed to return values as ascii: " + e.get_error_message(), false, e.get_error_code(),
                __FILE__, __LINE__);
    }
    catch (...) {
        throw BESInternalFatalError("Failed to return values as ascii: Unknown exception caught", __FILE__, __LINE__);
    }

    BESDEBUG("ascii", "Done BESAsciiTransmit::send_dap4_csv" << endl);
}

