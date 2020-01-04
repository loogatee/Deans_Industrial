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

--
--  ADGetAllAvgs.lua
--
function AD_GetAll()
    local SS,S1
    local t = {}

    ADGETALL_CMD = "{ 'ADGetAll' }"

    local ctx = zmq.context()
    local skt = ctx:socket{ zmq.REQ, linger=0, rcvtimeo=1500, connect = "ipc:///tmp/zmqfeeds/CmdChannel" }

    skt:send( ADGETALL_CMD )

    SS = skt:recv()
    if SS == nil then
        S1 = '{ "ADGetAllAvgs": ["error","skt:recv() returned nil"] } '
    else
        S1 = '{ "ADGetAllAvgs": ' .. SS .. ' } '
    end

    skt:close()
    ctx:destroy()

    print(S1)
end


--
--  ADGetAllAvgs.lua
--
--#define ZMQPORT_BACKCHANNEL_PR    "ipc:///tmp/zmqfeeds/backchannelpaktrecv"
function AD_GetNames()
    local SS,S1

    local ctx = zmq.context()
    local skt = ctx:socket{ zmq.REQ, linger=0, rcvtimeo=1500, connect = "ipc:///tmp/zmqfeeds/backchannelpaktrecv" }

    skt:send( "ADGetNames" )

    SS = skt:recv()
    if SS == nil then
        S1 = '{ "ADGetNames": ["error","skt:recv() returned nil"] } '
    else
        S1 = '{ "ADGetNames": ' .. SS .. ' } '
    end

    skt:close()
    ctx:destroy()

    print(S1)
end



AD_GetAll()
AD_GetNames()


