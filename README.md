# ActionSimulation

#### 介绍

基于QT的动作-现象 仿真平台，仿真逻辑使用Lua编写。

#### 开发环境

QT 5.15.2

boost asio 1.80

luajit 2.1.0 beta 3

#### 使用说明

    项目编译后会生成2个程序ActionSimulationServer 和 ActionSimulationEditor，ActionSimulationEditor 用于创建项目。ActionSimulationServer 用于运行项目。ActionSimulationServer有二种运行方式一种是将ActionSimulationServer注册为服务，以window服务的方式运行，一种是以控制台的方式运行。具体信息可以在控制台下使用 ActionSimulationServer -h 查看。ActionSimulationServer 运行后使用tcp协议与客户端进行通讯。ActionSimulationServer.exe 从 ActionSimulationServer.json 文件中读取配置。ActionSimulationServer.json说明见下图:

![](D:\Jimmy\Pictures\配置文件.png)

###### 创建项目

    运行 ActionSimulationEditor，点击新建 创建项目 然后在项目名称上点击鼠标右键创建类别用于组织组件(设备)，然后在组件下单击鼠标右键添加组件(设备)。

![](D:\Jimmy\Pictures\创建项目.png)

![](D:\Jimmy\Pictures\添加类别.png)

![](D:\Jimmy\Pictures\添加设备.png)

###### 组件设备属性

    ActionSimulation 仿真的对象是组件，组件有如下属性

- ID：唯一的标识每个组件。

- 名称: 便于记忆的名字

- 类别：组件所在位置

- 设备类型：所有组件分5种类型，输入设备，输出设备，内部设备，设备组(主，从)。
  
  - 输入设备是唯一从客户端改变状态的设备，由于输入设备的状态只能从客户端改变且行为要和改变一致，所以其行为不能是脚本。系统收到某个连接改变输出设备的请求后会转发给该用户所有其他连接
  
  - 输出设备和内部设备的区别是，输出设备的状态改变后系统会将改变后的值推送到该用户的所有客户端连接。内部设备只会推送给管理员用户（用于调试）
  
  - 设备组用于处理业务关系比较紧密的一组设备，同一个设备组只能有一个主设备，可以有多个从设备，主设备的行为只能是脚本，且从设备的状态只能在主设备脚本中改变，主设备的值为数字，初始为0.每次调用脚本+1

- 行为：设备有4种行为。脚本。等于输入(类似拨动开关设备)。等于输入，复位不发信号(类似键盘设备)。置位反转，复位不发信号(很多自复位按键开关)。**本系统中，置位指使设备状态不等于默认值的行为。复位指使设备状态等于默认值的行为**

- 设备组：仅用于设备组(主，从)设备

- 角色：lua 脚本名称，多个行为相同的设备可以使用同一脚本

- 置位信号保持时间：仅用于输入设备。用于处理一些延迟发送信号的情况

- 默认值：设备的初始值，如果指定初始值为 _calculate_default_value 则表示该设备的初始值需要在脚本加载后动态计算，这时候行为必须为脚本，且脚本中必须实现on_initialize函数

- 订阅设备：设备可以订阅其他设备，当订阅的设备状态值改变后，该设备收到信号，按定义的行为改变自己的值。内部设备，输出设备的脚本中只能改变自己的值无法改变其他设备的值

- 引用设备：只用于行为是脚本的设备。在设备执行脚本时系统会把引用设备的当前值传到脚本

- 响应的广播：一般用来定义设备故障信息

#### Lua 脚本编写规则

     ActionSimulation 中行为为脚本设备，其行为定义在 角色.lua  文件中 组织方式如下图所示

![](D:\Jimmy\Pictures\项目组织.png) 

    每个脚本中有一个必须的 on_action函数处理设备动作事件，一个可选的 on_initialize 函数计算初始值，一个可选的on_time函数处理计时器事件，函数的参数都是json字符串。

- on_action 函数参数说明：json字符串格式 (%d表示json数字，%s表示json字符串，%r 表示任意合法的json值，%o 表示json对象，%a 表示json数组)
  
  对于设备组（主）：
  
  {"_userid":%d,"_trigger":%s,"_cid":"%s,"Slave1":%r,.."SlaveN":%r,"_Slave1_default_value":%r,.."_SlaveN_default_value":%r,"Cp1":%r,..,"CpN":%r,"_cache":%o,"_boardcast":%a}
  
  对于其他设备
  
  {"_userid":%d,"_trigger":%s,"_cid":"%s,"_value":%r","_default_value":%r,"Cp1":%r,..,"CpN":%r,"_cache":%o,"_boardcast":%a}
  
  _userid ： 当前用户名。单用户模式下为0
  
  _trigger：触发事件的订阅设备id,如果是 “_boardcast” 则为广播触发，如果是 “_calculate_default_value” 则是需要计算默认值
  
  _cid：当前设备ID
  
  _value：设备当前值
  
  _default_value：设备默认值
  
  Cp1 .. CpN：该设备所有订阅设备和引用设备的当前值
  
  Slave1 .. SlaveN：从设备当前值
  
  _Slave1_default_value.. _SlaveN_default_value：从设备默认值
  
  _cache：缓存的值.(脚本中有一些信息需要存以便以后使用，则放在_cache中)
  
  _boardcast：响应的广播

- on_initialize 函数参数说明：
  
  - 对于设备组(主)
    
    - {"Slave1":%r,...,"SlaveN":"%r,"Cp1":%r,..,"CpN":%r}
  
  - 对于其他设备
    
    - {“Cp1":%r,..,"CpN":%r}

- on_time 函数参数说明：{"_userid":%d,_cid":"%s,"_value":%r","_default_value":%r,"Cp1":%r,..,"CpN":%r,"_cache":%o,"_counter":%d,"_boardcast":%a}
  
  _counter：计时器调用次数

- 返回值说明：
  
  - 对于内部设备或外部设备脚本的返回值：
    
    - {} 不改变现有状态
    
    - {"_value":%r[,"_cache":%o]} 正常返回
    
    - {“_value":%r,"_timer":{"times":val1,"interval":500}},返回并启动计时器 "times" 调用次数,-1不限制次数,"interval"调用间隔时间(毫秒)，如果 "interval" 小于最小间隔时间则使用默认间隔时间
    
    - {“_value":%r,"_timer":true},返回并启动计时器 "times" = -1,"interval" = 默认间隔时间
    
    - {"_value":%r,"_timer":false} 返回并停止计时器
    
    - {"_loop":[%d,%d,%d,%d],"_times":val1,"_interval":500 } value 按 loop 中给定的序列按 "interval" 间隔(毫秒) 依次循环取值，"times" 循环次数,-1不限制次数(value 值每次更改 times 都会变化)，当下次调用 on_action 或 on_time 时且返回非空时停止 当前loop.
    
    - {"_loop":[%d,%d,%d,%d]} "times" = -1,"interval" = 默认间隔时间
    
    - on_initialize 函数的返回值中 _value 指缺省值
  
  - 对于设备组(主)设备的返回值
    
    - {} 不改变现有状态
    
    - {"cid1":%r,"cid2":%r,.."cidN":%r[,"_cache":%o]} 正常返回,cid1..cidN 表示team组内的TeamSlave类型组件id
    
    - {"cid1":%r,"cid2":%r,.."cidN":%r,"_timer":{"times":val1,"interval":500}},返回并启动计时器 "times" 调用次数,-1不限制次数,"interval"调用间隔时间(毫秒)，如果 "interval" 小于最小间隔时间则使用默认间隔时间
    
    - {"cid1":%r,"cid2":%r,.."cidN":%r,"_timer":true},返回并启动计时器 "times" = -1,"interval" = 默认间隔时间
    
    - {"cid1":%r,"cid2":%r,.."cidN":%r,"_timer":false} 返回并停止计时器
      
      {_timer":true|false} 单纯的启动或停止计时器
    
    - on_initialize 函数的返回值中 "cid1":%r,"cid2":%r,.."cidN":%r,指缺省值

#### 通讯协议

    ActionSimulationServer 运行后与客户端使用Tcp协议   进行通讯。协议如下

- 设置日志级别：
  
  - 发送:{"action":"set_log","log_level":%d}
  
  - 回复:{"action":"set_log","log_level":%d,"result":"succeed|failed"[,"reason":%s]}

- 设置广播：
  
  - 发送:{"action":"set_boardcast","value":%s}
  
  - 回复:{"action":"set_boardcast","value":%s,"result":"succeed|failed"[,"reason":%s]}
  
  - 同时收到因设置导致的设备状态变化

- 取消广播：
  
  - 发送:{"action":"cancel_boardcast","value":%s}
  
  - 回复:{"action":"cancel_boardcast","value":%s,"result":"succeed|failed"[,"reason":%s]}
  
  - 同时收到因取消导致的设备状态变化

- 获取广播：
  
  - 发送:{"action":"get_boardcast"}
  
  - 回复:{"action":"get_boardcast",value":%a}
    
      - 失败 answer:{"action":"get_boardcast","result":"failed","reason":%s}

- 加载项目：
  
  - 发送:{"action":"load_project"}
  
  - 回复:{"action":"load_project","result":"succeed|failed"[,"reason":%s ][,"name":"%s","status":%d,"components":%o,"categories":%o,"relation":%o,"dashboards":%o]}
  
  - 项目在运行后会自动加载如果运行后对项目做了修改或对脚本作了修改需要 先停止项目-加载项目-运行项目

- 运行项目：
  
  - 发送:{"action":"run"}
  
  - 回复:{"action":"run","result":"succeed|failed"[,"reason":%s]}

- 停止项目：
  
  - 发送:{"action":"stop"}
  
  - 回复:{"action":"stop","result":"succeed|failed"[,"reason":%s]}

- 获取项目状态：
  
  - 发送:{"action":"get_project_status"}
  
  - 回复:{"action":"get_project_status","name":"%s","status":“%s”}

- 用户注册：
  
  - 发送:{"action":"login"[,"userid":%d]["role":0]}  
  
  - 回复:{"action":"login","userid":%d,"result":"succeed|failed"[,"reason":%s ]}，userid 是用户标识,项目配置文件中 user_type = 0 时userid无效
    
    role 省略时为0,表示一般用户,1表示管理员,大于1表示自定义用户类型

- 发送通知：
  
  - 发送:{"action":"notify"[,"role":%d],[,"userid":%r],...}
  
  - 默认会把命令转发给所有连接，如果指定了 role 或 userid 则会按role 或 userid的规则进一步进行筛选，如果同时指定则 userid 优先

- 获取所有组件值：
  
  - 发送:{"action":"query_all_value"}
  
  - 回复:{"cid":"component",value":%r}

- 新加载脚本：
  
  - 发送:{"action":"reload_script","role":"%s"}
  
  - 回复:{"action":"reload_script","result":"succeed|failed"[,"reason":%s]}
  
  - 此处的role是设备属性中的角色，此命令用于热更新

- 重置项目：
  
  - 发送:{"action":"reset"}
  
  - 应答一组{"cid":"component",”value":%r}命令集合

- 查询组件值：
  
  - 发送:{"action":"query_value","cid":%s}
  
  - 回复:{"cid":"component",value":%r}

- 组件状态变化：
  
  - 发送:{"cid":"component_id","value":%d}
  
  - 回复(0个或多个):{"cid":"value_changed_device_name",value":%r}

#### 后续开发

- ActionSimulationEditor 添加 订阅,引用关系图，以方便查看设备间关系

- 写一个 ActionSimulationRunner 用于测试，现在测试项目可以使用使用NetAssist网络调试助手

- 订阅关系动态变更功能

#### 联系作者

    作者：Jimmy Song

    如果在使用过程中你有什么想法，建议或问题，欢迎联系我，我会乐于与你探讨

    邮箱: 52333676@qq.com

 
