local lf  = require("luafcgid")
local zmq = require("lzmq")

--
--  ADGetAllAvgs.lua
--
function main(env, con)
    local SS,S1

    local ADGETALL_CMD = "{ 'ADGetAll' }"

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

    con:header("Content-Type", "application/json")
    con:header("Connection",   "close")
    con:puts(S1)
end
