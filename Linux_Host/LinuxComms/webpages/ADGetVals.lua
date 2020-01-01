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
--  ADGetVals.lua
--
function main(env, con)
    local SS,S1,S2,P,n
    local t = {}

    S2 = 5

    P = lf.parse(env.QUERY_STRING)
    if P.chan ~= nil then
        n = tonumber(P.chan)
        if n >= 1 and n <= 16 then
            S2 = n
        end
    end

	
    local ctx = zmq.context()
    local skt = ctx:socket{ zmq.REQ, linger=0, rcvtimeo=1500, connect = "ipc:///tmp/zmqfeeds/CmdChannel" }

    t[#t+1] = "\004"
    t[#t+1] = "\000"
    t[#t+1] = string.char(S2)

    skt:send( table.concat(t) )

    local SS = skt:recv()
    if SS == nil then
        S1 = '{ "ADGetVals": "skt:recv() returned nil" }'
    else
        S1 = '{ "ADGetVals": ' .. SS .. ' } '
    end

    skt:close()
    ctx:destroy()

    con:header("Content-Type", "application/json")
    con:header("Connection",   "close")
    con:puts(S1)
end






