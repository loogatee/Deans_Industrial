local lf  = require("luafcgid")
local zmq = require("lzmq")
local bit = require("bit")




hAD_AI1   =  0
hAD_AI2   =  1
hAD_AI3   =  2
hAD_AI4   =  3
hAD_AI5   =  4
hAD_AI6   =  5
hAD_AI7   =  6
hAD_AI8   =  7
hAD_AI9   =  8
hAD_AI10  =  9
hAD_AI11  = 10
hAD_AI12  = 11
hAD_AI13  = 12
hAD_AI14  = 13
hAD_AI15  = 14
hAD_AI16  = 15
hI0CBt    = 16

--
--  cmd_get_value.lua
--
function main(env, con)

    local ctx1 = zmq.context()
    local skt1 = ctx1:socket{ zmq.PUSH, connect = "ipc:///tmp/zmqfeeds/1" }

    local ctx2 = zmq.context()
    local skt2 = ctx2:socket{ zmq.SUB,  subscribe = { "\031" },  rcvtimeo=1500, connect = "ipc:///tmp/zmqfeeds/CdcResponse" }

    -- 
    -- /lua/get_cmd_value.lua?parm=5
    --
    local n = 0
    local P = lf.parse(env.QUERY_STRING)
    if P.parm ~= nil then
        n = tonumber(P.parm)
    end

    local T = {}
    T[#T+1] = "\001"
    T[#T+1] = "\031"
    T[#T+1] = string.char(n)                    -- index for value you want to retrieve is here

    skt1:send( table.concat(T) )                -- send on one queue
    local SS = skt2:recv()                      -- receive on another

    local S1,S2
    if SS == nil then
        S1 = '{ "cmd_get_value": ["error","skt2:recv() returned nil"] } '
    else

        local a = string.byte(SS,3)
        local b = string.byte(SS,4)
        local c = string.byte(SS,5)
        local d = string.byte(SS,6)

        b = bit.lshift(b,8)
        c = bit.lshift(c,16)
        d = bit.lshift(d,24)

        local e = bit.bor( a, b )
        local f = bit.bor( c, d )
        local g = bit.bor( e, f )

        ---------------------------------

         -- Init, then may overwrite
         S2 = string.format("%d",g)

         if n >= hAD_AI1 and n <= hAD_AI16 then
             S2 = string.format("%.2f",g/1000)           -- floating point numbers
         elseif n == hI0CBt then
             S2 = string.format("%.2f",g/1000)
         end

         --------------------------------

         S1 = '{ "cmd_get_value": ' .. S2 .. ' } '
    end

    skt1:close()
    ctx1:destroy()
    skt2:close()
    ctx2:destroy()

    con:header("Content-Type", "application/json")
    con:header("Connection",   "close")
    con:puts(S1)
end
