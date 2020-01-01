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



local version = zmq.version()
--print(string.format("zmq version: %d.%d.%d", version[1], version[2], version[3]))

local ctx = zmq.context()

-- local skt = ctx:socket{ zmq.REQ, linger = 0, rcvtimeo = 1000, connect = "ipc:///tmp/zmqfeeds/CmdChannel" }
local skt = ctx:socket{ zmq.REQ, connect = "ipc:///tmp/zmqfeeds/CmdChannel" }


--    B2 is the LED:
--                    0 = LED0
--                    1 = LED1
--                    2 = LED2
--                    3 = LED3
--    B3 is the Value:
--                    0 = OFF
--                    1 = ON
--
--    B0   B1  B2  B3      -- Byte 0, Byte 1, Byte 2
X = "\002\000\000\001"     -- this will do the setled
skt:send(X)


SS = skt:recv()

if SS ~= nil then
    print("setled result: " .. SS)
else
    print("recv returned nil")
end

skt:close()
ctx:destroy()
