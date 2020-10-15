Opus = {
	basepath = PathDir(ModuleFilename()),

	OptFind = function (name, required)
		local check = function(option, settings)
			option.value = false
			option.lib_path = nil

			if IsNegativeTerm(ScriptArgs[name .. ".use_pkgconfig"]) then
				option.use_pkgconfig = false
			elseif IsPositiveTerm(ScriptArgs[name .. ".use_pkgconfig"]) then
				option.use_pkgconfig = true
			end

			if ExecuteSilent("pkg-config opus") == 0 then
				option.value = true
				if option.use_pkgconfig == nil then
					option.use_pkgconfig = true
				end
			end

			if option.use_pkgconfig == nil then
				option.use_pkgconfig = false
			end

			if platform == "win32" then
				option.value = true
			elseif platform == "win64" then
				option.value = true
			elseif platform == "macosx" and string.find(settings.config_name, "32") then
				option.value = true
			elseif platform == "macosx" and string.find(settings.config_name, "64") then
				option.value = true
			elseif platform == "linux" and arch == "ia32" then
				option.value = true
			elseif platform == "linux" and arch == "amd64" then
				option.value = true
			end
		end

		local apply = function(option, settings)
			if option.use_pkgconfig == true then
				settings.cc.flags:Add("`pkg-config --cflags opus`")
				settings.link.flags:Add("`pkg-config --libs opus`")
			else
				settings.cc.includes:Add(Opus.basepath .. "/include")
				settings.cc.includes:Add(Opus.basepath .. "/include/opus")

				if family ~= "windows" then
					settings.link.libs:Add("opus")
				end
			end
		end

		local save = function(option, output)
			output:option(option, "value")
			output:option(option, "use_pkgconfig")
		end

		local display = function(option)
			if option.value == true then
				if option.use_pkgconfig == true then return "using pkg-config" end
				return "using bundled libs"
			else
				if option.required then
					return "not found (required)"
				else
					return "not found (optional)"
				end
			end
		end

		local o = MakeOption(name, 0, check, save, display)
		o.Apply = apply
		o.include_path = nil
		o.lib_path = nil
		o.required = required
		return o
	end
}
