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

function cmd_get_value()
    local a,b,c,d,e,f,g

    local ctx1 = zmq.context()
    local skt1 = ctx1:socket{ zmq.PUSH, connect = "ipc:///tmp/zmqfeeds/1" }

    local ctx2 = zmq.context()
    local skt2 = ctx2:socket{ zmq.SUB,  subscribe = { "\031" },  rcvtimeo=1500, connect = "ipc:///tmp/zmqfeeds/CdcResponse" }

    
    skt1:send( "\001\031\000" )

    local SS = skt2:recv()
    if SS == nil then
        S1 = '{ "cmd_get_value": ["error","skt2:recv() returned nil"] } '
    else
        S1 = 'ok'
    end


    skt1:close()
    ctx1:destroy()

    skt2:close()
    ctx2:destroy()

    if SS == nil then
        print(S1)
        return
    end

    ii=3
    for j=1,1 do

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


cmd_get_value()


