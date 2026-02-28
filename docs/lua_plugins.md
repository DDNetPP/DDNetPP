# lua plugin api

THE API IS IN SUPER EARLY STATE!!!!!!! NOT READY TO BE USED YET!

## register hooks

There are a few functions you can define in your lua script
that will be called by the server. For example `on_init` and `on_tick`

```lua
-- your_plugin.lua

function on_init()
    print("this is called only on init")
end

function on_tick()
    print("this is called every tick")
end
```

## trigger actions

There is the global `Game` instance that lets you trigger some actions on the server side.
For example `Game:send_chat()`

```lua
-- your_plugin.lua

function on_player_connect()
    Game:send_chat("someone joined the game")
end
```

## plugin to plugin api

All plugins have their own lua state and are isolated.
But there is a way to attempt calling functions defined in other plugins.


Let's say we have a plugin called `lib.lua` and a plugin called `main.lua`
and main wants to call a function defined in lib it works like this:

```lua
-- lib.lua

function lib_func(arg)
    print("hello from lib. got arg: " .. arg)
    return "frog"
end
```

```lua
-- main.lua

-- call_plugin(func_name, args..)
ok, result = Game:call_plugin("lib_func", "some argument")

if ok then
    print("lib_func found and it returned: " .. result)
end
```

The method `call_plugin` always returns two values where the first
is a boolean that is set to true if the call worked and the second is the return
value from the called function.


In this case it searches for "lib_func" in all plugins and only runs the first it finds.
If it finds no function with that name in any plugin it will return `false` and `nil`.

## register rcon commands

You can register a lua function to be called if a rcon command is executed.
Your callback should take as first parameter the client id of the player
that executed the command as integer and as second argument the raw arguments string.

```lua
-- myplugin.lua

Game:register_rcon("custom_command", function (client_id, args)
	Game:send_chat("cid=" .. client_id  .. " ran command with args: '" .. args .. "'")
end)
```

The arguments do not get parsed by the teeworlds console code so you have to do that your self.
But you can also let other lua plugins do the argument parsing for you.

There is an official [argparse.lua](https://github.com/DDNetPP/plugins/blob/43ba783577bcab7913fb0b699c828856500a3685/argparser.lua) plugin
you can install by downloading the lua file and placing it into your plugins directory next to your plugin.
Then you can use it like this:

```lua
-- myplugin.lua

Game:register_rcon("yellow", function (_, args)
	ok, data = Game:call_plugin("parse_args", "s[name]i[seconds]", args)
	if ok == false then
		Game:send_chat("please install arg parser plugin")
		return
	end
   if data.error ~= nil then
		Game:send_chat(data.error)
		return
   end
   args = data.args

   Game:send_chat("player '" .. args.name .. "' is now yellow for " .. args.seconds .. " seconds")
end)
```
