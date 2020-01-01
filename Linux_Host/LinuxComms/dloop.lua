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

local zmq = require "lzmq"

--DLOOP_CMD = "\001\000\164\010"     -- this will do the dloop     0x0100        CDCCMD_DLOOP = 0x0001 
DLOOP_CMD = "\001\000\009\010"     -- this will do the dloop     0x0100        CDCCMD_DLOOP = 0x0001 


local version = zmq.version()
--print(string.format("zmq version: %d.%d.%d", version[1], version[2], version[3]))

local ctx = zmq.context()

-- local skt = ctx:socket{ zmq.REQ, linger = 0, rcvtimeo = 1000, connect = "ipc:///tmp/zmqfeeds/CmdChannel" }
   local skt = ctx:socket{ zmq.REQ,                              connect = "ipc:///tmp/zmqfeeds/CmdChannel" }

for i=1,10000 do

    skt:send( DLOOP_CMD )

    SS = skt:recv()
    if SS ~= nil then
        print("dloop result: " .. SS .. "     " .. i )
    else
        print("recv returned nil")
    end

end

skt:close()
ctx:destroy()
