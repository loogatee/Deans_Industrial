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

ctx = zmq.context()
skt = ctx:socket{ zmq.REQ, connect = "ipc:///tmp/zmqfeeds/CmdChannel" }

TheCmd = "\003\000\000"     -- B0,B1,B2:  this will do the reset
skt:send(TheCmd)            -- Send it
TheResp = skt:recv()        -- Get the God-Damned response, God-Dammit!!   ;-)

if TheResp ~= nil then
    print("resetdev (03,00,00)  result: " .. TheResp)
else
    print("skt:recv() returned nil")
end

skt:close()
ctx:destroy()

