local cjson = require("cjson")

function TeamTest_logic_on_action(params_list)
  local params = cjson.decode(params_list)
  local retTable = {}

  if params["Power"] == 1 and params["TeamTestSwitch"] == 1 then
    retTable["_cache"] = os.time()
    retTable["_timer"] = true
    retTable["TeamSlaveBool"] = false
    retTable["TeamSlaveNumber"] = 1
    retTable["TeamSlaveString"] = 'Hello world' .. retTable["TeamSlaveNumber"]

    local TeamSlaveObject = {}
    TeamSlaveObject["Key"] = retTable["TeamSlaveString"]
    retTable["TeamSlaveObject"] = TeamSlaveObject
  else
    retTable["_timer"] = false
  end

  return  cjson.encode(retTable)
end

function TeamTest_logic_on_time(params_list)
    local params = cjson.decode(params_list)
    local retTable = {}
  
    retTable["TeamSlaveBool"] = not params["TeamSlaveBool"]
    retTable["TeamSlaveNumber"] = params["TeamSlaveNumber"] + 1
    retTable["TeamSlaveString"] = 'Hello world ' .. params["_cache"] .. ' ' .. retTable["TeamSlaveNumber"]
  
    local TeamSlaveObject = {}
    TeamSlaveObject["Key"] = retTable["TeamSlaveString"] 
    retTable["TeamSlaveObject"] = TeamSlaveObject
  
    return  cjson.encode(retTable);
end