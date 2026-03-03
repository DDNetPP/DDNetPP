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
that executed the command as integer and as second argument a table which will be filled
with all the arguments passed to the rcon command.


The signature of the register_rcon command looks like this:
`register_rcon(name: string, params: string, description: string, callback: function)`

The `name` will be the rcon command that is being registered and that has to match
the first word of the rcon line the user sends. The `params` are regular teeworlds parameters.
If you named them with brackets the name will be used as the key in the args table.
If you do not name them it will be just an array.

```lua
-- myplugin.lua

Game:register_rcon("yellow", "s[foo]", "description of command", function (client_id, args)
	Game:send_chat("cid=" .. client_id  .. " ran command with arg: '" .. args.foo .. "'")
end)
```

```lua
-- myplugin.lua

Game:register_rcon("add", "i[num1] i[num2]", "adds two numbers", function (client_id, args)
	Game:send_chat("result: " .. args.num1 + args.num2)
end)

Game:register_rcon("foo", "?s[might be nil]", "adds two numbers", function (client_id, args)
	if args["might be nil"] then
		Game:send_chat("got optional argument: " .. args["might be nil"])
	end
end)
```

## type hints

You can install type hints for your lsp using `luarocks install lls-addon-ddnetpp`


For more details check out https://github.com/DDNetPP/lls-addon-ddnetpp
