local cjson = require("cjson")

function CacheTest_logic_on_action(params_list)
  local params = cjson.decode(params_list)
  local retTable = {}

  if params["Power"] == 1 and params["CacheTestSwitch"] == 1 then
    retTable["_cache"] = os.time()
    retTable["_timer"] = true
    retTable["_value"] = 1
  else
    retTable["_timer"] = false
    retTable["_value"] = 0
  end

  return  cjson.encode(retTable)
end

function CacheTest_logic_on_time(params_list)
  local params = cjson.decode(params_list)
  local retTable = {}
  retTable["_value"] = (os.time() - params["_cache"]) * 1000 + params["_counter"]
  return  cjson.encode(retTable);
end