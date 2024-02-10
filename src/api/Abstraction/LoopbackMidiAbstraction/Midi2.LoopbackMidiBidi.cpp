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
CMidi2LoopbackMidiBiDi::Initialize(
    LPCWSTR endpointId,
    PABSTRACTIONCREATIONPARAMS,
    DWORD *,
    IMidiCallback * Callback,
    LONGLONG Context,
    GUID /* SessionId */
)
{
    TraceLoggingWrite(
        MidiLoopbackMidiAbstractionTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingPointer(this, "this"),
        TraceLoggingWideString(endpointId, "endpoint id")
        );

    m_callbackContext = Context;
    m_endpointId = internal::NormalizeEndpointInterfaceIdWStringCopy(endpointId);
  
    //if (Context != MIDI_PROTOCOL_MANAGER_ENDPOINT_CREATION_CONTEXT)
    {
        HRESULT hr = S_OK;

        // TODO: This should use SWD properties and not a string search

        if (internal::EndpointInterfaceIdContainsString(m_endpointId, MIDI_VIRT_INSTANCE_ID_DEVICE_PREFIX))
        {
            TraceLoggingWrite(
                MidiLoopbackMidiAbstractionTelemetryProvider::Provider(),
                __FUNCTION__,
                TraceLoggingLevel(WINEVENT_LEVEL_INFO),
                TraceLoggingPointer(this, "this"),
                TraceLoggingWideString(L"Initializing device-side BiDi", "message"),
                TraceLoggingWideString(m_endpointId.c_str(), "endpoint id")
            );

            m_callback = Callback;

 //           LOG_IF_FAILED(hr = AbstractionState::Current().GetEndpointTable()->OnDeviceConnected(m_endpointId, this));
        }
        else if (internal::EndpointInterfaceIdContainsString(m_endpointId, MIDI_VIRT_INSTANCE_ID_CLIENT_PREFIX))
        {
            TraceLoggingWrite(
                MidiLoopbackMidiAbstractionTelemetryProvider::Provider(),
                __FUNCTION__,
                TraceLoggingLevel(WINEVENT_LEVEL_INFO),
                TraceLoggingPointer(this, "this"),
                TraceLoggingWideString(L"Initializing client-side BiDi", "message"),
                TraceLoggingWideString(m_endpointId.c_str(), "endpoint id")
            );

            m_callback = Callback;

 //           LOG_IF_FAILED(hr = AbstractionState::Current().GetEndpointTable()->OnClientConnected(m_endpointId, this));
        }
        else
        {
            TraceLoggingWrite(
                MidiLoopbackMidiAbstractionTelemetryProvider::Provider(),
                __FUNCTION__,
                TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
                TraceLoggingPointer(this, "this"),
                TraceLoggingWideString(L"We don't understand the endpoint Id", "message"),
                TraceLoggingWideString(m_endpointId.c_str(), "endpoint id")
                );

            // we don't understand this endpoint id

            hr = E_FAIL;
        }

        return hr;
    }
    //else
    //{
    //    // we're in protocol negotiation
    //    m_isDeviceSide = false;
    //    return S_OK;
    //}
}

HRESULT
CMidi2LoopbackMidiBiDi::Cleanup()
{
    TraceLoggingWrite(
        MidiLoopbackMidiAbstractionTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingPointer(this, "this"),
        TraceLoggingWideString(m_endpointId.c_str(), "endpoint id")
        );

    m_callback = nullptr;
    m_callbackContext = 0;

    return S_OK;
}

_Use_decl_annotations_
HRESULT
CMidi2LoopbackMidiBiDi::SendMidiMessage(
    PVOID Message,
    UINT Size,
    LONGLONG /*Position*/
)
{
    TraceLoggingWrite(
        MidiLoopbackMidiAbstractionTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingPointer(this, "this"),
        TraceLoggingWideString(m_endpointId.c_str(), "endpoint id")
    );

    // message received from the device

    RETURN_HR_IF_NULL(E_INVALIDARG, Message);
    RETURN_HR_IF(E_INVALIDARG, Size < sizeof(uint32_t));

    //for (auto connection : m_linkedBiDiConnections)
    //{
    //    auto cb = connection->GetCallback();
    //    
    //    if (cb != nullptr)
    //    {
    //        return cb->Callback(Message, Size, Position, m_callbackContext);
    //    }
    //}


    //// if there's no linked bidi, it's not a failure. We just lose the message
    //if (m_linkedBiDiCallback != nullptr)
    //{
    //    //return m_linkedBiDi->SendMidiMessage(Message, Size, Position);
    //    return m_linkedBiDiCallback->Callback(Message, Size, Position, m_callbackContext);
    //}

    return S_OK;

}

_Use_decl_annotations_
HRESULT
CMidi2LoopbackMidiBiDi::Callback(
    PVOID Message,
    UINT Size,
    LONGLONG Position,
    LONGLONG Context
)
{
    TraceLoggingWrite(
        MidiLoopbackMidiAbstractionTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingPointer(this, "this"),
        TraceLoggingWideString(m_endpointId.c_str(), "endpoint id")
    );

    // message received from the client

    if (m_callback != nullptr)
    {
        return m_callback->Callback(Message, Size, Position, Context);
    }

    return S_OK;
}

