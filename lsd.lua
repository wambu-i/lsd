require "socket"

local iwinfo = require("iwinfo")
local uci = require("uci")
local log = require 'log'

local args = {...}
local iface = args[1]
local interval = args[2]
local threshold = tonumber(args[3])

function get_connected_stations(iface)
	local api = iwinfo.type(iface)
   	local iw = iwinfo[api]
	local tmp = 0
	local str = ""
	local stations = iw.assoclist(iface)
	if not stations or next(stations) == nil then
		str = string.format("%s: no associated stations", iface)
		log.error(str)
		return nil
	end
	for addr, info in pairs(stations) do
		str = string.format("Client: %s - Signal: %s", addr, info.signal)
		log.info(str)
		if math.abs(info.signal) < threshold then tmp = tmp + 1 end
	end
	if tmp ~= 0 then
		str = string.format("%d client(s) with less than %d signal", tmp, threshold)
		log.info(str)
	end
end

function get_devices_signal()
	local now = socket.gettime()
	local next_poll = now + interval
	get_connected_stations(iface)
	local secs = next_poll - now
--	if secs > 0 then socket.sleep(secs) end
end

--[[ while true do
	local _, err = pcall(function () get_devices_signal() end)
	if err ~= nil then
		log.error(err)
		break
	end
end ]]

for i = 1, threshold do
	get_devices_signal()
	socket.sleep(interval)
end