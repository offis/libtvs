/* :external-source:
 *
 * Copyright (C) 2007--2010  Torsten Maehne
 * Copyright (c) 2011--2012  OFFIS Institute for Information Technology
 *                           Oldenburg, Germany
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file    sc_trace.h
 *
 * \brief   header to provide sc_trace() support for the
 *          boost::units::quantity<Unit, Y> type.
 *
 * This file is derived from the \c sc_trace_quantity.h
 * header of the \em SystemC \em AMS \em extensions \em
 * eXperiments library.
 *
 * \author    Torsten Maehne
 * \date      2011-01-13
 * \copyright Apache License, Version 2.0
 *            http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SYSX_UNITS_SC_TRACE_H_INCLUDED_
#define SYSX_UNITS_SC_TRACE_H_INCLUDED_

#include "tvs/utils/systemc.h"
#include "tvs/units/common.h"

/// SystemC core namespace
namespace sc_core {

/*!
 * \brief trace the passed Boost.Units quantity value
 *
 * This overload is added to the sc_core namespace to support tracing
 * of SystemC ports and signals using the Boost.Units quantity type via
 * SystemC's sc_trace() mechanism.
 *
 * \param tfp   Pointer to trace file.
 * \param value Quantity value.
 * \param name  Trace name.
 *
 * \author      Torsten Maehne
 */
template<typename Unit, typename T>
void
sc_trace(sc_trace_file* tfp,
         const boost::units::quantity<Unit, T>& value,
         const std::string& name)
{
  // forward plain value to SystemC tracing
  sc_trace(tfp, value.value(), name);
}

} // namespace sc_core

#endif // SYSX_UNITS_SC_TRACE_H_INCLUDED_
/* Taf!
 * :tag: (units,h)
 */
