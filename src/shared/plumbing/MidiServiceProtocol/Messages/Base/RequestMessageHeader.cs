﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MidiService.Protocol.Messages.Base
{
    [ProtoContract]
    public struct RequestMessageHeader
    {
        [ProtoMember(11)] 
        public Guid ClientId;         // this may not be necessary
        [ProtoMember(12)] 
        public Guid ClientRequestId;  // generated by client for a request, parroted back by server in response

        [ProtoMember(13)] 
        public string ClientVersion;  // Version class can't be serialized/deserialized, so using string

    }
}
