-- a blob is just a blue laser dot that has gravity and collision
-- you can spawn some using the chat command /blob i[amount]

---@class Blob
---@field snap_id integer
---@field pos Position
---@field vel Velocity

---@type Blob[]
local blobs = {}

---@return Blob
function new_blob()
	return {
		snap_id = ddnetpp.snap.new_id(),
		vel = {
			x = 5,
			y = 5,
		},
		pos = {
			x = 10,
			y = 10,
		}
	}
end

---@param blob Blob
function snap_blob(blob)
	ddnetpp.snap.new_laser({
		id = blob.snap_id,
		x = blob.pos.x,
		y = blob.pos.y,
	})
end

---@param blob Blob
function tick_blob(blob)
	-- gravity
	blob.vel.y = blob.vel.y + 0.5

	blob.pos, blob.vel, grounded = ddnetpp.collision.move_box(blob.pos, blob.vel)
	if grounded then
		blob.vel.y = 0
	end
end

ddnetpp.register_chat("blob", "?i[amount]", "spawn a new blob", function (client_id, args)
	local chr = ddnetpp.get_character(client_id)
	if chr == nil then
		ddnetpp.send_chat_target(client_id, "You need to be alive to use this command")
		return
	end
	local num = 1
	if args.amount then
		num = args.amount
	end
	ddnetpp.send_chat_target(client_id, "spawned " .. num .. " blob")
	for _ = 1, num do
		local blob = new_blob()
		blob.pos = chr:pos()
		blob.vel.y = math.random(-1, -20)
		blob.vel.x = math.random(-10, 10)
		table.insert(blobs, blob)
	end
end)

function ddnetpp.on_snap()
	for _, blob in pairs(blobs) do
		snap_blob(blob)
	end
end

function ddnetpp.on_tick()
	for _, blob in pairs(blobs) do
		tick_blob(blob)
	end
end

function ddnetpp.on_init()
	table.insert(blobs, new_blob())
end
