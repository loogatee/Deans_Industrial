--
-- Copyright 2019  john reed
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not  use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
-- http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations  under the License.
--

zmq = require "lzmq"

-- #define ZMQPORT_BACKCHANNEL_3     "ipc:///tmp/zmqfeeds/backchannel3"
--ctx = zmq.context()
--skt = ctx:socket{ zmq.PUSH, connect = "ipc:///tmp/zmqfeeds/0" }
--TheCmd = "\028\000\000"     -- B0,B1,B2:  
--TheCmd = "\199\000\000"     -- B0,B1,B2:  
--skt:send(TheCmd)            -- Send it


ctx = zmq.context()
skt = ctx:socket{ zmq.REQ, connect = "ipc:///tmp/zmqfeeds/backchannel3" }

TheCmd = "\109\001"

skt:send(TheCmd)

SS = skt:recv()

if SS ~= nil then
    print("result: " .. SS)
else
    print("recv returned nil")
end



skt:close()
ctx:destroy()

--what healtcheck does:
--
--   buffer[0] = 0x6d;                                     // 6d is the Magic Header Byte
--   buffer[1] = 0x01;                                     //    01:  "ok, TotBytes, zmqsend_errors"
--   rc = zmq_send( requester, buffer,  2, ZMQ_NOBLOCK );
--   len = zmq_recv( requester,  buffer, 40, 0 );

