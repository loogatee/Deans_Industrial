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

local yesorno_TRANS =
{
    Yes = 1,
    yes = 1,
    No  = 0,
    no  = 0
}

--local THE_FILENAME="/usr/share/monkey/cdcX/ConfigData.lua"
--local THE_FILENAME="./ConfigData.lua"
local THE_FILENAME="/usr/local/bin/ConfigData.lua"

local AD_NAME_TRANS =
{
    ['I0.CDv'] = 1,
    ['I0.SYp'] = 2,
    ['I0.CSp'] = 3,
    ['I0.CSt'] = 4,
    ['I0.CBt'] = 5,
    ['I0.TWt'] = 6,
    ['I0.TWl'] = 7,
    ['I0.SYw'] = 8,
    ['I0.CPw'] = 9,
    ['I0.CLt'] = 10,
    ['I0.WSt'] = 11,
    ['A1.SYw'] = 12,
    ['A1.BLw'] = 13,
    ['A1.CPw'] = 14,
    ['A2.SYw'] = 15,
    ['A2.BLw'] = 16,
    ['A2.CPw'] = 17,
    ['5v_Ref'] = 18,
    ['GndRef'] = 19
}

local V_TYPES_TRANS =
{
    ['V_CORRECTED']           = 1,
    ['V_LINEAR_CORRECTION']   = 3,
    ['V_LINEAR_RATIO_METRIC'] = 5
}

--
-- { Line='AI1', Name='I0.CDv', Enbl='yes', Type='V_CORRECTED', DLog=1, Coefs={'1.0','0.0','0.0','1.0','1.0',} }
--
function lua_Get_AD_Config()
    local coef1,coef2
    local T = {}

    dofile(THE_FILENAME)

    for k,W in pairs({ConfigData.ANALOG_TO_DIGITAL_INPUTS_1,ConfigData.ANALOG_TO_DIGITAL_INPUTS_2}) do
        for i,V in ipairs(W) do
            T[#T+1] = AD_NAME_TRANS[V.Name]
            T[#T+1] = yesorno_TRANS[V.Enbl]
            T[#T+1] = V_TYPES_TRANS[V.Type]
            if V.Name == 'I0.SYw' then coef1=V.Coefs[1] end
            if V.Name == 'I0.CPw' then coef2=V.Coefs[1] end
        end
    end

    T[#T+1] = tonumber(coef1)*100
    T[#T+1] = tonumber(coef2)*100

    return T
end




function lua_Get_AD_Names()
    local T = {}

    dofile(THE_FILENAME)

    for k,W in pairs({ConfigData.ANALOG_TO_DIGITAL_INPUTS_1,ConfigData.ANALOG_TO_DIGITAL_INPUTS_2}) do
        for i,V in ipairs(W) do
            T[#T+1] = V.Name
        end
    end

    return T
end

--
--    Z={ 'dloop', 2, 3 }
--
function lua_Get_parms( Sval )
    local X = { "none", 0, 0 }
    local T = {}

    if #Sval == 0 then return X end

    local F  = loadstring( 'Z=' .. Sval )
    if type(F) ~= 'function' then return X end
    F()

    if Z[1] ~= nil and type(Z[1]) == 'string' then T[1] = Z[1] else T[1] = 'none' end
    if Z[2] ~= nil and type(Z[2]) == 'number' then T[2] = Z[2] else T[2] = -9999  end
    if Z[3] ~= nil and type(Z[3]) == 'number' then T[3] = Z[3] else T[3] = -9999  end

    Z=nil
    return T
end




