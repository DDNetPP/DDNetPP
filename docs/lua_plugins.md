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
end
```

```lua
-- main.lua

Game:call_plugin("lib_func", "some argument")
```

It searches for "lib_func" in all plugins and only runs the first it finds.
If it finds no function with that name in any plugin it will throw and error.
So you need to use load and pcall to do this safely.
