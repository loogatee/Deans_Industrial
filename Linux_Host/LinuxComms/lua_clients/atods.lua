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




function AD2()
    local a,b,c,d,e,f,g

    local ctx1 = zmq.context()
    local skt1 = ctx1:socket{ zmq.PUSH, connect = "ipc:///tmp/zmqfeeds/1" }

    local ctx2 = zmq.context()
    local skt2 = ctx2:socket{ zmq.SUB,  subscribe = { "\030" },  rcvtimeo=1500, connect = "ipc:///tmp/zmqfeeds/CdcResponse" }

    
    skt1:send( "\001\030\000" )

    local SS = skt2:recv()
    if SS == nil then
        S1 = '{ "AD2": ["error","skt2:recv() returned nil"] } '
    else
        S1 = '{ "AD2": ' .. SS .. ' } '
    end

    skt1:close()
    ctx1:destroy()

    skt2:close()
    ctx2:destroy()

    ii=6
    for j=1,16 do

        a = string.byte(SS,ii)
        b = string.byte(SS,ii+1)
        c = string.byte(SS,ii+2)
        d = string.byte(SS,ii+3)

        b = bit.lshift(b,8)
        c = bit.lshift(c,16)
        d = bit.lshift(d,24)

        e = bit.bor( a, b )
        f = bit.bor( c, d )
        g = bit.bor( e, f )

        print( g / 1000 )

        ii=ii+4
    end
    
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


AD2()
--AD_GetAll()
--AD_GetNames()


