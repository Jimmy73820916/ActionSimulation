local common = require "_common"
local cjson = require "cjson"

function FaultTest_logic_on_action(params_list)
  local params = cjson.decode(params_list)
  
  local retTable = {}

  if (params["Power"] == 0) or (params["FaultTestSwitch"] == 0) or common.HasBoardcode(params["_boardcast"],"fault1") then
    retTable["_value"] = 0
  else
    if params["Power"] == 1 and params["FaultTestSwitch"] == 1 then
      retTable["_value"] = 1
    else
      retTable["_value"] = 0
    end
  end
  
  return  cjson.encode(retTable)
end