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

    t[#t+1] = "\004"                              -- 0x0004 is the command
    t[#t+1] = "\000"
    t[#t+1] = string.char(S2)                     -- this is the channel number

    skt:send( table.concat(t) )

    local SS = skt:recv()
    if SS == nil then
        S1 = string.format('{ "ADGetVals": ["error","%d","skt:recv() returned nil"] }',S2)
    else
        S1 = '{ "ADGetVals": ' .. SS .. ' } '
    end

    skt:close()
    ctx:destroy()

    con:header("Content-Type", "application/json")
    con:header("Connection",   "close")
    con:puts(S1)
end






