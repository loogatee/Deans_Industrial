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


local lf  = require("luafcgid")
local zmq = require("lzmq")


--
--   192.168.20.205/lua/d2.lua?F=ex2
--   192.168.20.205/lua/d2.lua?F=ex3
--
function main(env, con)
    local t = {}
	
    local ctx = zmq.context()
    local skt = ctx:socket{ zmq.PUSH, connect = "ipc:///tmp/zmqfeeds/0" }

    t[#t+1] = "\028"
    t[#t+1] = "\002"
    t[#t+1] = "\064"
    t[#t+1] = "\047"

    skt:send( table.concat(t) )

    skt:close()
    ctx:destroy()

    S1 = '{ "ADconfig: OK" }'
    con:header("Content-Type", "application/json")
    con:header("Connection",   "close")
    con:puts(S1)

end






