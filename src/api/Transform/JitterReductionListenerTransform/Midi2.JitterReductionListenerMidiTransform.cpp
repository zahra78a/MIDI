// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================


#include "pch.h"



_Use_decl_annotations_
HRESULT
CMidi2JitterReductionListenerMidiTransform::Initialize(
    LPCWSTR deviceId,
    PTRANSFORMCREATIONPARAMS creationParams,
    DWORD* mmcssTaskId,
    IMidiCallback* callback,
    LONGLONG context,
    IUnknown* MidiDeviceManager
)
{
    UNREFERENCED_PARAMETER(deviceId);
    UNREFERENCED_PARAMETER(creationParams);
    UNREFERENCED_PARAMETER(mmcssTaskId);
    UNREFERENCED_PARAMETER(MidiDeviceManager);


    OutputDebugString(L"" __FUNCTION__ " Start up");

    TraceLoggingWrite(
        MidiJitterReductionListenerTransformTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingPointer(this, "this")
    );


    m_callback = callback;

    m_context = context;

    return S_OK;
}

HRESULT
CMidi2JitterReductionListenerMidiTransform::Cleanup()
{
    TraceLoggingWrite(
        MidiJitterReductionListenerTransformTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingPointer(this, "this")
    );

    try
    {
        OutputDebugString(L"" __FUNCTION__ " Shut down time");

        return S_OK;
    }
    catch (...)
    {
        OutputDebugString(L"" __FUNCTION__ " Exception cleaning up");

        // TODO: Log
        return S_OK;    // we don't care when cleaning up
    }
}


_Use_decl_annotations_
HRESULT
CMidi2JitterReductionListenerMidiTransform::SendMidiMessage(
    PVOID data,
    UINT size,
    LONGLONG timestamp)
{
    UNREFERENCED_PARAMETER(data);
    UNREFERENCED_PARAMETER(size);
    UNREFERENCED_PARAMETER(timestamp);



    return S_OK;
}

