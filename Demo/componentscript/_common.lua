local M = {}

function M.HasBoardcode(tbl,boardcode)
  for _,v in pairs(tbl) do
    if v == boardcode then 
      return true
    end
  end
  return false
end

return M