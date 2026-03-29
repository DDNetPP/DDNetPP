# lua plugin api

THE API IS IN SUPER EARLY STATE!!!!!!! NOT READY TO BE USED YET!

## enable lua plugins

You need to compile DDNet++ with lua support for example like this

```
mkdir build
cd build
cmake .. -DLUA=ON
make
```

You need lua 5.4 installed for this to work properly. On debian based systems you can install it like this

```
apt install liblua5.4-dev lua5.4
```

## register hooks

There are a few functions you can define in your lua script
that will be called by the server. For example `ddnetpp.on_init` and `ddnetpp.on_tick`

```lua
-- your_plugin.lua

function ddnetpp.on_init()
	print("this is called only on init")
end

function ddnetpp.on_tick()
	print("this is called every tick")
end

function ddnetpp.on_chat(client_id, msg)
	-- append LOL to all messages sent to chat
	-- unless its a chat command
	if msg.message:sub(1, 1) ~= "/" then
		msg.message = msg.message .. " LOL"
	end
	return msg
end

function ddnetpp.on_snap()
	chr = ddnetpp.get_character(0)
	if chr then
		pos = chr:pos()
		-- WARNING: this is a simplified example do not use "id = 2"
		--          checkout ddnetpp.snap.new_id() and ddnetpp.snap.free_id()
		--          for correct snap ids
		ddnetpp.snap.new_laser({
			id = 2,
			x = pos.x,
			y = pos.y - 2,
			from_x = pos.x,
			from_y = pos.y - 2,
			start_tick = 0,
		})
	end
end

function ddnetpp.on_character_take_damage(chr, weapon, from, dmg)
	if chr:id() == 0 then
		-- weapons of client id 0 will not cause any knockback
		return false
	end
	-- all other hammers are explosion boosted
	if weapon == ddnetpp.weapon.HAMMER then
		local pos = chr:pos()
		pos.y = pos.y + 0.1
		ddnetpp.create_explosion(pos)
	end
end
```

## change snap of existing characters

```lua
-- snapping_client is the client id of the snapshot receiver
-- character is the character instance of the tee that is being snapped
-- snap_item is whatever ddnet++ would include in the snap normally
function ddnetpp.on_snap_character(snapping_client, character, snap_item)
	-- render all tees without weapon
	snap_item.weapon = -1
	return snap_item
end
```

## trigger actions

There is the global `ddnetpp` instance that lets you trigger some actions on the server side.
For example `ddnetpp.send_chat()`

```lua
-- your_plugin.lua

function ddnetpp.on_player_connect(client_id)
	player = ddnetpp.get_player(client_id)
	ddnetpp.send_chat("'" .. player:name() .. "' connected!")
end
```

## edit chat message

In the on_chat event you can edit the msg.message and msg.team
before returning it again. This gets called before it runs chat commands.
So you can alter chat commands. Here an example of a chat command typo
auto correct plugin

```lua
-- autocorrect.lua

ddnetpp.register_chat("foo", "", "", function (client_id, args)
	ddnetpp.send_chat("hello from foo chat command")
end)

function ddnetpp.on_chat(client_id, msg)
	if msg.message == "/doo" then
		ddnetpp.send_chat("/doo typo corrected to /foo")
		-- this will run the foo chat command defined above
		msg.message = "/foo"
	end
	return msg
end
```

## implement custom tiles

```lua
function ddnetpp.on_character_game_tile_change(chr, tile)
	if tile == ddnetpp.tile.FREEZE then
		-- this only gets called once when touching freeze
		ddnetpp.send_chat_target(chr, "woah you suck xd")
	end
end

function ddnetpp.on_character_tile(chr, tile)
	if tile == 159 then
		-- assuming index 159 is not used in the gamelayer
		-- you can implement your own tile here
		-- this will be called every tick
	end
end
```

## set custom spawn points

```lua
function ddnetpp.on_pick_spawn_pos(player)
	-- player with client id 1 (second joining player on fresh server)
	-- will always spawn at the top right of the map
	if player:id() == 1 then
		return {
			x = 0,
			y = 0,
		}
	end
	-- other players spawn at default spawn
	return nil
end
```

## override existing tiles

```lua
function ddnetpp.on_skip_game_tile(chr, tile)
	-- if a player uses the name "hacker" they become immune to freeze tiles
	if tile == ddnetpp.tile.FREEZE and chr:player():name() == "hacker" then
		return true
	end
	return false
end
```

## objects with gravity and collision

Have a look at the [move_box_blob.lua](./examples/move_box_blob.lua) example if
you want to create your own entity that collides with the world.

## explosions with client mask

In C++ there is a `CClientMask` which in lua is represented as a table
with integer keys representing client ids. All keys set will be included in the mask.
You can use this to show for example explosions only to two players with id 0 and 2.

```lua
function ddnetpp.on_tick()
	local mask = {}
	-- only players with the client id 0 and 2 will
	-- see the explosion
	mask[0] = true
	mask[2] = true
	ddnetpp.create_explosion(
		{ x = 20, y = 20 }, -- pos
		-1, -- owner
		ddnetpp.weapon.GRENADE, -- weapon
		false, -- no damage
		0, -- team
		mask -- client mask
	)
end
```

## read and write player inputs

You can call `:input()` on a character instance to get their current inputs.
The aimed at position target_x and target_y is relative to the tee and 32 as precise
as the coordinates used for positions in the world. So for example if you place the
cursor on your own tee the target_x and target_y will be 0, 0 and if you aim
to the top right it will be something like target_x = 200 and target_y = -200
So if you want to get the position in the world you need to add it to your own tees position
and divide it by 32. Here is an example where every player sees a heart under their own cursor.

```lua
function ddnetpp.on_snap(snapping_client_id)
	local chr = ddnetpp.get_character(snapping_client_id)
	if chr then
		local inp = chr:input()
		local pos = chr:pos()
		pos.x = pos.x + inp.target_x / 32
		pos.y = pos.y + inp.target_y / 32
		-- WARNING: the id 128 might already be used it is on you to avoid snap item id collisions
		--          this is just a naive example
		ddnetpp.snap.new_pickup({
			id = 128,
			pos = pos,
			type = ddnetpp.protocol.POWERUP_HEALTH,
			sub_type = 0,
		})
	end
end
```

To set inputs you can use `:set_input()` on any character instance.
And pass the inputs as keys in your table. All inputs are optional it will only
update the inputs you explicitly set. **But setting inputs only works if you are in the
`on_character_pre_tick()` event!** If you try to set inputs on tick or on snap or another place
things will break. Here is a small example where we invert the players direction.
So when they try to walk left we force them to walk right.

```lua
function ddnetpp.on_character_pre_tick(character)
	character:set_input({
		direction = character:input().direction * -1
	})
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
ok, result = ddnetpp.call_plugin("lib_func", "some argument")

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

ddnetpp.register_rcon("yellow", "s[foo]", "description of command", function (client_id, args)
	ddnetpp.send_chat("cid=" .. client_id  .. " ran command with arg: '" .. args.foo .. "'")
end)
```

```lua
-- myplugin.lua

ddnetpp.register_rcon("add", "i[num1] i[num2]", "adds two numbers", function (client_id, args)
	ddnetpp.send_chat("result: " .. args.num1 + args.num2)
end)

ddnetpp.register_rcon("foo", "?s[might be nil]", "adds two numbers", function (client_id, args)
	if args["might be nil"] then
		ddnetpp.send_chat("got optional argument: " .. args["might be nil"])
	end
end)
```

## multi file plugins

All lua files in the project directory will be loaded as one plugin.
If you are working on a bigger plugin that has multiple files you can also place an entire
directory in the plugins directory. It will be loaded if it contains a src/main.lua inside of it
all other files and folders will be ignored. So it is on you to require other files if you need them.
Ideally you should use require locations relative to the current lua script in the filesystem
because you should make no assumptions about the current working directory.

```
plugins/
├── accounts                    # <= multi file plugin will be loaded as one plugin
│   └── src
│       ├── chat.lua
│       ├── database.lua
│       ├── main.lua            # <= the only entry point loaded by the server
│       └── ratelimits.lua
├── rainbow.lua                 # <= single file plugin will be loaded as one plugin
└── simple.lua                  # <= single file plugin will be loaded as one plugin
```

If you place your plugins directory in the teeworlds storage location `$CURRENTDIR` so next to the server binary.
Then in your main.lua you might get away with a require like this


```lua
-- main.lua

-- WARNING: this relative require CAN and WILL break
local db = require("plugins/accounts/src/database")
```

But this breaks for users that run the server from a different path than the executable.
Or install the plugin in the `$USERDIR`. It will also break if the user renames the plugin directory.


So a way more robust way and the recommended way is doing it like this:

```lua
function script_path()
   local str = debug.getinfo(2, "S").source:sub(2)
   return str:match("(.*/)") or "./"
end

local db = require(script_path() .. "database")
```


## type hints

You can install type hints for your lsp using `luarocks install lls-addon-ddnetpp`


For more details check out https://github.com/DDNetPP/lls-addon-ddnetpp
