local lf = require("luafcgid")


HTML__0 = [[
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>test hello world</title>
</head>
<body>
    <h1>HelloWorld</h1>
</body>
</html>
]]


function main(env, con)
    con:header("X-Powered-By", "Lua")
    con:puts(HTML__0)

    con:puts("<h1>Environment</h1><pre>\n")
    for n, v in pairs(env) do
        con:puts(string.format("%s = %s\n", n, v))
    end
    con:puts("</pre>\n")

    if env.REQUEST_METHOD == "GET" then
        s = {}  -- string sets are faster then calling req:puts() all the time
        --params = lf.parse(env.QUERY_STRING)
        table.insert(s, "<h1>GET ParAms</h1><pre>\n")

        --for n, v in pairs(params) do
        --    table.insert(s, string.format("%s = %s\n", n, v))
        --end

        table.insert(s, "</pre>\n")
        con:puts(table.concat(s))
    end
end





























