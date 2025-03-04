// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include <windows.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Enumeration.h>

#include <wrl\module.h>
#include <wrl\event.h>
#include <avrt.h>
#include <wil\com.h>
#include <wil\resource.h>
#include <wil\result_macros.h>
#include <wil\tracelogging.h>
#include <ppltasks.h>

#include <SDKDDKVer.h>

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // some CString constructors will be explicit
#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlcoll.h>
#include <atlsync.h>

#include <winmeta.h>
#include <TraceLoggingProvider.h>

// AbstractionUtilities
#include "endpoint_data_helpers.h"
#include "swd_property_builders.h"
#include "json_helpers.h"

#include "MidiDefs.h"
#include "MidiDataFormat.h"
#include "MidiFlow.h"
#include "MidiAbstraction.h"

#include "MidiXProc.h"

namespace internal = ::Windows::Devices::Midi2::Internal;

#include "Midi2SampleAbstraction_i.c"
#include "Midi2SampleAbstraction.h"

#include "dllmain.h"

#include "Midi2.SampleAbstraction.h"
#include "Midi2.SampleMidiIn.h"
#include "Midi2.SampleMidiOut.h"
#include "Midi2.SampleMidiBiDi.h"

