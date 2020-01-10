local lf  = require("luafcgid")
local zmq = require("lzmq")

--
--
--
function main(env, con)
    local SS,S1
    local t = {}

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

    con:header("Content-Type", "application/json")
    con:header("Connection",   "close")
    con:puts(S1)
end
