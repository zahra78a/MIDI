﻿using System.Diagnostics;
using System.Text.Json;

namespace MidiService.Protocol
{
    public class MidiServiceConstants
    {
        private const string PipePrefix = "midi.";

        // May want to make this something which can be set in the config file

        public const string InitialConnectionPipeName = PipePrefix + "connect";
        public const string PingPipeName = PipePrefix + "ping";


        public const string EnumerationServicePipePrefix = PipePrefix + "enumerator";

        public const string SessionServicePipePrefix = PipePrefix + "session";  // this gets an ID added to it




    }
}