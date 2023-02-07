local cjson = require("cjson")

function LoopTest_logic_on_action(params_list)
  local params = cjson.decode(params_list)
  
  local retTable = {}

  if params["Power"] == 1 and params["LoopTestSwitch"] == 1 then
    retTable["_loop"] = {"red","green","yellow"}
  else
    retTable["_value"] = 0
  end

  return  cjson.encode(retTable);
end

function LoopTest_logic_on_time(params_list)
end
