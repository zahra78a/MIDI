// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#include "stdafx.h"


HRESULT
CMidiSessionTracker::Initialize()
{
    return S_OK;
}

_Use_decl_annotations_
HRESULT
CMidiSessionTracker::AddClientSession(
    GUID SessionId,
    LPCWSTR SessionName,
    DWORD ClientProcessId,
    LPCWSTR ClientProcessName 
)
{
    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingPointer(this, "this"),
        TraceLoggingGuid(SessionId),
        TraceLoggingWideString(SessionName),
        TraceLoggingWideString(ClientProcessName)
        );

    MidiSessionEntry entry;

    entry.SessionId = SessionId;
    entry.SessionName = internal::TrimmedWStringCopy(SessionName);
    entry.ClientProcessId = ClientProcessId;
    entry.ProcessName = ClientProcessName;

    entry.StartTime = std::chrono::system_clock::now();

//    GetSystemTime(&entry.StartTime);

    m_sessions[SessionId] = entry;

    return S_OK;
}

_Use_decl_annotations_
HRESULT
CMidiSessionTracker::RemoveClientSession(
    GUID SessionId
)
{
    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingPointer(this, "this"),
        TraceLoggingGuid(SessionId)
    );

    if (m_sessions.find(SessionId) != m_sessions.end())
    {
        m_sessions.erase(SessionId);
    }

    return S_OK;
}

_Use_decl_annotations_
HRESULT
CMidiSessionTracker::AddClientEndpointConnection(
    GUID SessionId, 
    LPCWSTR ConnectionEndpointInterfaceId
)
{
    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingPointer(this, "this"),
        TraceLoggingGuid(SessionId),
        TraceLoggingWideString(ConnectionEndpointInterfaceId)
    );

    MidiSessionConnectionEntry newConnection;

    auto cleanEndpointId = internal::NormalizeEndpointInterfaceIdWStringCopy(ConnectionEndpointInterfaceId);

    newConnection.ConnectedEndpointInterfaceId = cleanEndpointId;
    newConnection.InstanceCount = 1;
    newConnection.EarliestStartTime = std::chrono::system_clock::now();

    if (auto sessionEntry = m_sessions.find(SessionId); sessionEntry != m_sessions.end())
    {
        if (auto connection = sessionEntry->second.Connections.find(cleanEndpointId); connection != sessionEntry->second.Connections.end())
        {
            // connection already exists. Increment it
            connection->second.InstanceCount++;
        }
        else
        {
            sessionEntry->second.Connections[cleanEndpointId] = newConnection;
        }
    }
    else
    {
        // we have a connection and no session. Create a session to hold it and
        // then call this again
        RETURN_IF_FAILED(AddClientSession(SessionId, L"Unknown Session", 0, L"Unknown Process"));
        RETURN_IF_FAILED(AddClientEndpointConnection(SessionId, ConnectionEndpointInterfaceId));    // interface id will get cleaned up on second pass
    }

    return S_OK;
}

_Use_decl_annotations_
HRESULT
CMidiSessionTracker::RemoveClientEndpointConnection(
    GUID SessionId, 
    LPCWSTR ConnectionEndpointInterfaceId
)
{
    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingPointer(this, "this"),
        TraceLoggingGuid(SessionId),
        TraceLoggingWideString(ConnectionEndpointInterfaceId)
    );

    auto cleanEndpointId = internal::NormalizeEndpointInterfaceIdWStringCopy(ConnectionEndpointInterfaceId);

    if (auto sessionEntry = m_sessions.find(SessionId); sessionEntry != m_sessions.end())
    {
        if (auto connection = sessionEntry->second.Connections.find(cleanEndpointId); connection != sessionEntry->second.Connections.end())
        {
            // connection already exists. Decrement it
            connection->second.InstanceCount--;

            if (connection->second.InstanceCount <= 0)
            {
                sessionEntry->second.Connections.erase(cleanEndpointId);
            }
        }
    }

    return S_OK;

}



_Use_decl_annotations_
HRESULT 
CMidiSessionTracker::GetSessionListJson(
    BSTR* SessionList
)
{

    json::JsonObject rootObject;
    json::JsonArray sessionsArray{};



    for (auto sessionIter = m_sessions.begin(); sessionIter != m_sessions.end(); sessionIter++)
    {
        json::JsonObject sessionObject;

        internal::JsonSetGuidProperty(sessionObject, MIDI_SESSION_TRACKER_JSON_RESULT_SESSION_ID_PROPERTY_KEY, sessionIter->second.SessionId);
        internal::JsonSetWStringProperty(sessionObject, MIDI_SESSION_TRACKER_JSON_RESULT_SESSION_NAME_PROPERTY_KEY, sessionIter->second.SessionName);
        internal::JsonSetWStringProperty(sessionObject, MIDI_SESSION_TRACKER_JSON_RESULT_PROCESS_NAME_PROPERTY_KEY, sessionIter->second.ProcessName);
        internal::JsonSetWStringProperty(sessionObject, MIDI_SESSION_TRACKER_JSON_RESULT_PROCESS_ID_PROPERTY_KEY, std::to_wstring(sessionIter->second.ClientProcessId));
        internal::JsonSetDateTimeProperty(sessionObject, MIDI_SESSION_TRACKER_JSON_RESULT_SESSION_TIME_PROPERTY_KEY, sessionIter->second.StartTime);

        // now add all the client connections

        json::JsonArray connectionsArray{};

        for (auto connectionIter = sessionIter->second.Connections.begin(); connectionIter != sessionIter->second.Connections.end(); connectionIter++)
        {
            json::JsonObject connectionObject;

            internal::JsonSetDateTimeProperty(sessionObject, MIDI_SESSION_TRACKER_JSON_RESULT_CONNECTION_TIME_PROPERTY_KEY, connectionIter->second.EarliestStartTime);
            internal::JsonSetDoubleProperty(connectionObject, MIDI_SESSION_TRACKER_JSON_RESULT_CONNECTION_COUNT_PROPERTY_KEY, connectionIter->second.InstanceCount);
            internal::JsonSetWStringProperty(connectionObject, MIDI_SESSION_TRACKER_JSON_RESULT_CONNECTION_ENDPOINT_ID_PROPERTY_KEY, connectionIter->second.ConnectedEndpointInterfaceId);
            
            connectionsArray.Append(connectionObject);
        }

        // append the connections array only if we have some
        if (connectionsArray.Size() > 0)
        {
            internal::JsonSetArrayProperty(sessionObject, MIDI_SESSION_TRACKER_JSON_RESULT_CONNECTION_ARRAY_PROPERTY_KEY, connectionsArray);
        }

        sessionsArray.Append(sessionObject);
    }

    rootObject.SetNamedValue(MIDI_SESSION_TRACKER_JSON_RESULT_SESSION_ARRAY_PROPERTY_KEY, sessionsArray);

    CComBSTR ccbstr;
    ccbstr.Empty();

    // we do this conversion step so we can trace log it
    std::wstring wstr = rootObject.Stringify().c_str();

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingPointer(this, "this"),
        TraceLoggingWideString(wstr.c_str())
    );

    ccbstr = wstr.c_str();

    RETURN_IF_FAILED(ccbstr.CopyTo(SessionList));

    return S_OK;
}

HRESULT
CMidiSessionTracker::Cleanup()
{
    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingPointer(this, "this")
    );



    return S_OK;
}