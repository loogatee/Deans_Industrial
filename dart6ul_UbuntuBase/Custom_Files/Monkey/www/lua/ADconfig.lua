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






