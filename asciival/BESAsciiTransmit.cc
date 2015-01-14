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
#include <BESDapError.h>
#include <BESInternalFatalError.h>

#include <BESDebug.h>

#include "BESAsciiTransmit.h"
#include "get_ascii.h"
#include "get_ascii_dap4.h"

using namespace dap_asciival;

BESAsciiTransmit::BESAsciiTransmit() : BESBasicTransmitter() {

	add_method(DATA_SERVICE, BESAsciiTransmit::send_basic_ascii);
    add_method(DAP4DATA_SERVICE, BESAsciiTransmit::send_dap4_csv);

}

void BESAsciiTransmit::send_basic_ascii(BESResponseObject *obj, BESDataHandlerInterface &dhi)
{
    BESDEBUG("ascii", "BESAsciiTransmit::send_base_ascii" << endl);

    BESDataDDSResponse *bdds = dynamic_cast<BESDataDDSResponse *>(obj);
    DataDDS *dds = bdds->get_dds();
    ConstraintEvaluator & ce = bdds->get_ce();

    dhi.first_container();

    string constraint = www2id(dhi.data[POST_CONSTRAINT], "%", "%20%26");

    try {
        ce.parse_constraint(constraint, *dds);
    }
    catch (InternalErr &e) {
        throw BESDapError("Failed to parse the constraint expression: " + e.get_error_message(), true, e.get_error_code(), __FILE__, __LINE__);
    }
    catch (Error &e) {
        throw BESDapError("Failed to parse the constraint expression: " + e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
    }
    catch (...) {
        throw BESInternalFatalError("Failed to parse the constraint expression: Unknown exception caught", __FILE__, __LINE__);
    }

    dds->tag_nested_sequences(); // Tag Sequences as Parent or Leaf node.

    // string dataset_name = dhi.container->access();

	try {
		// Handle *functional* constraint expressions specially
		if (ce.function_clauses()) {
			BESDEBUG("ascii", "processing a functional constraint clause(s)." << endl);
			// This leaks the DDS on the LHS, I think. jhrg 7/29/14
			// Yes, eval_function_clauses() allocates a new DDS object which
			// it returns. This code was assigning that to 'dds' which is
			// a pointer to the DataDDS managed by the BES. The fix is to
			// store that in a temp, pass that new pointer into the BES object,
			// delete the old object held by the BES and then set the local pointer
			// so that the remaining code can use it. See ticket 2240. All of the
			// Transmitter functions in BES modules will need this fix if they
			// call eval_function_clauses(). jhrg 7/30/14
			DataDDS *new_dds = ce.eval_function_clauses(*dds);
			bdds->set_dds(new_dds);
			delete dds;
			dds = new_dds;
		}
        else {
            // Iterate through the variables in the DataDDS and read in the data
            // if the variable has the send flag set.
            for (DDS::Vars_iter i = dds->var_begin(); i != dds->var_end(); i++) {
                if ((*i)->send_p()) {
                    (**i).intern_data(ce, *dds);
                }
            }
        }
    }

    catch (InternalErr &e) {
        throw BESDapError("Failed to read data: " + e.get_error_message(), true, e.get_error_code(), __FILE__, __LINE__);
    }
    catch (Error & e) {
        throw BESDapError("Failed to read data: " + e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
    }
    catch (...) {
        throw BESInternalFatalError("Failed to read data: Unknown exception caught", __FILE__, __LINE__);
    }

    try {
        // Now that we have constrained the DataDDS and read in the data,
        // send it as ascii
        DataDDS *ascii_dds = datadds_to_ascii_datadds(dds);

        get_data_values_as_ascii(ascii_dds, dhi.get_output_stream());

        dhi.get_output_stream() << flush;
        delete ascii_dds;
    }
    catch (InternalErr &e) {
        throw BESDapError("Failed to get values as ascii: " + e.get_error_message(), true, e.get_error_code(), __FILE__, __LINE__);
    }
    catch (Error &e) {
        throw BESDapError("Failed to get values as ascii: " + e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
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
        throw BESDapError("Failed to return values as ascii: " + e.get_error_message(), true, e.get_error_code(), __FILE__, __LINE__);
    }
    catch (Error &e) {
        throw BESDapError("Failed to return values as ascii: " + e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
    }
    catch (...) {
        throw BESInternalFatalError("Failed to return values as ascii: Unknown exception caught", __FILE__, __LINE__);
    }

    BESDEBUG("ascii", "Done BESAsciiTransmit::send_dap4_csv" << endl);
}

