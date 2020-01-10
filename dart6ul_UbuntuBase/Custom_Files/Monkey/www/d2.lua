local lf  = require("luafcgid")
local zmq = require("lzmq")


HTML__err = [[
<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8">
	<title>Error Message</title>
	<link rel="stylesheet" href="/css/dojodemo.css">
</head>
<body>
	<h1>$$ZZ1</h1>
</body>
</html>
]]




function dloop(env, con, Parms)
    local SS,S1,ctx,skt
    local t = {}

    t[#t+1] = "\001"
    t[#t+1] = "\000"
    t[#t+1] = "\004"
    t[#t+1] = "\001"

    ctx = zmq.context()
    skt = ctx:socket{ zmq.REQ, connect = "ipc:///tmp/zmqfeeds/CmdChannel" }

    skt:send( table.concat(t) )

    SS = skt:recv()
    if SS == nil then
         S1 = '{ "dloop": "skt:recv() returned nil" }'
     else
         S1 = '{ "dloop": "' .. SS .. '" }'
    end

    skt:close()
    ctx:destroy()

    con:header("Content-Type", "application/json")
    con:header("Connection",   "close")
    con:puts(S1)
end



function doreset(env, con, Parms)
    local SS,S1,ctx,skt
    local t = {}

    t[#t+1] = "\003"
    t[#t+1] = "\000"
    t[#t+1] = "\000"

    ctx = zmq.context()
    skt = ctx:socket{ zmq.REQ, connect = "ipc:///tmp/zmqfeeds/CmdChannel" }

    skt:send( table.concat(t) )

    SS = skt:recv()

    skt:close()
    ctx:destroy()
end


function setled(env, con, Parms)
    local SS,S1,s1,s2,theled,W
    local got_one = 1
    local t={}


    t[#t+1] = "\002"
    t[#t+1] = "\000"
    s2      = "\000"
    W       = "off"


    if Parms.led0 ~= nil then
        theled  = "led0"
        s1      = "\000"
        if Parms.led0 == "on" then W="on"; s2 = "\001" end

    elseif Parms.led1 ~= nil then
        theled  = "led1"
        s1      = "\001"
        if Parms.led1 == "on" then W="on"; s2 = "\001" end

    elseif Parms.led2 ~= nil then
        theled  = "led2"
        s1      = "\002"
        if Parms.led2 == "on" then W="on"; s2 = "\001" end

    elseif Parms.led3 ~= nil then
        theled  = "led3"
        s1      = "\003"
        if Parms.led3 == "on" then W="on"; s2 = "\001" end
    else
        got_one = 0
    end

    t[#t+1] = s1
    t[#t+1] = s2

    if got_one == 1 then

        local ctx = zmq.context()
        local skt = ctx:socket{ zmq.REQ, linger = 0, rcvtimeo = 1000, connect = "ipc:///tmp/zmqfeeds/CmdChannel" }

        skt:send(table.concat(t))
        SS = skt:recv()
        if SS == nil then
            S1 = '{ "setled": "skt:recv() returned nil" }'
        else
            S1 = '{ "' .. theled .. '": "' .. W .. '" }'
        end

        skt:close()
        ctx:destroy()

    else
        S1 = '{ "setled": "No Valid Led parm" }'
    end

    con:header("Content-Type", "application/json")
    con:header("Connection", "close")
    con:puts(S1)
end


--
--   192.168.20.205/lua/d2.lua?F=ex2
--   192.168.20.205/lua/d2.lua?F=ex3
--
function main(env, con)
    local P, S
	
    P = lf.parse(env.QUERY_STRING)

    if P.F == nil then
        S = string.gsub(HTML__err, '$$ZZ1', "Error! 'F' parm not found")
	con:puts(S)
    elseif _G[P.F] == nil then
        S = string.gsub(HTML__err, '$$ZZ1', "Error!  Function '" .. P.F .. "' not found")
	con:puts(S)
    else
        _G[P.F](env, con, P)
    end
end






