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

local zmq = require("lzmq")
bit = require("bit")


function S2()
    local a,b,c,d,e,f,g

    local ctx1 = zmq.context()
    local skt1 = ctx1:socket{ zmq.PUSH, connect = "ipc:///tmp/zmqfeeds/1" }

    
    skt1:send( "\002\026\004\000" )

    skt1:close()
    ctx1:destroy()
end

S2()


