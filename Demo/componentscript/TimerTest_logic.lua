local cjson = require("cjson")

function TimerTest_logic_on_action(params_list)
  local params = cjson.decode(params_list)
  
  local retTable = {}
  

  if params["Power"] == 1 and params["TimerTestSwitch"] == 1 then
    retTable["_value"] = 1

    local tblTimes = {}
    tblTimes.times = 100
    tblTimes.interval = 500

    retTable["_timer"] = tblTimes
  else
    retTable["_value"] = 0
    retTable["_timer"] = false
  end

  return  cjson.encode(retTable);
end

function TimerTest_logic_on_time(params_list)
  local params = cjson.decode(params_list)

  local retTable = {}

  retTable["_value"] = params["_value"] + params["_counter"] 

  return  cjson.encode(retTable);

end
