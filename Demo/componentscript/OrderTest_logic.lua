local cjson = require("cjson")

function OrderTest_logic_on_action(params_list)
  local params = cjson.decode(params_list)
  
  local retTable = {}

  if params["Power"] == 1 and params["OrderTestSwitch"] == 1 then
    retTable["_order"] = {0,100,3}
  else
    retTable["_value"] = 0
  end

  return  cjson.encode(retTable);
end

function OrderTest_logic_on_time(params_list)
end
