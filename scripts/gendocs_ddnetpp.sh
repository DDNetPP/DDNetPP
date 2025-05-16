#!/bin/bash

arg_is_dry=0

if [ "$1" == "--dry-run" ]; then
	arg_is_dry=1
fi

tmp() {
	mkdir -p scripts/
	echo "scripts/tmp.swp"
}

gen_configs() {
	local cfg
	local desc
	local cmd
	while read -r cfg; do
		desc="$(echo "$cfg" | cut -d',' -f7- | cut -d'"' -f2-)"
		desc="${desc::-2}"
		cmd="$(echo "$cfg" | cut -d',' -f2 | xargs)"
		echo "+ \`$cmd\` $desc"
	done < <(grep '^MACRO_CONFIG_INT' src/engine/shared/config_variables_ddpp.h)
	while read -r cfg; do
		desc="$(echo "$cfg" | cut -d',' -f6- | cut -d'"' -f2-)"
		desc="${desc::-2}"
		cmd="$(echo "$cfg" | cut -d',' -f2 | xargs)"
		echo "+ \`$cmd\` $desc"
	done < <(grep '^MACRO_CONFIG_STR' src/engine/shared/config_variables_ddpp.h)
}

# gen_console_cmds() {
# 	local prefix="$1"
# 	local header_file="$2"
# 	local cfg
# 	local desc
# 	local cmd
# 	while read -r cfg; do
# 		desc="$(echo "$cfg" | cut -d',' -f3- | cut -d'"' -f2-)"
# 		desc="${desc::-2}"
# 		cmd="$(echo "$cfg" | cut -d',' -f1 | cut -d'"' -f2)"
# 		echo "+ \`$prefix$cmd\` $desc"
# 	done < <(grep '^CONSOLE_COMMAND' "$header_file")
# }
# 
# gen_rcon_cmds() {
# 	gen_console_cmds "" src/game/server/instagib/rcon_commands.h
# }
# 
# gen_chat_cmds() {
# 	gen_console_cmds "/" src/game/server/instagib/chat_commands.h |
# 		grep -Ev '(ready|pause|shuffle|swap|drop)'
# }

insert_at() {
	# insert_at [from_pattern] [to_pattern] [new content] [filename]
	local from_pattern="$1"
	local to_pattern="$2"
	local content="$3"
	local filename="$4"
	local from_ln
	local to_ln
	if ! grep -q "$from_pattern" "$filename"; then
		echo "Error: pattern '$from_pattern' not found in '$filename'"
		exit 1
	fi
	from_ln="$(grep -n "$from_pattern" "$filename" | cut -d':' -f1 | head -n1)"
	from_ln="$((from_ln + 1))"
	to_ln="$(tail -n +"$from_ln" "$filename" | grep -n "$to_pattern" | cut -d':' -f1 | head -n1)"
	to_ln="$((from_ln + to_ln - 2))"

	{
		head -n "$((from_ln - 1))" "$filename"
		printf '%b\n' "$content"
		tail -n +"$to_ln" "$filename"
	} > "$(tmp)"
	if [ "$arg_is_dry" == "1" ]; then
		if [ "$(cat "$(tmp)")" != "$(cat "$filename")" ]; then
			echo "Error: missing docs for $filename"
			echo "       run ./scripts/gendocs_ddnetpp.sh"
			git diff --no-index --color "$(tmp)" "$filename"
			exit 1
		fi
	else
		mv "$(tmp)" "$filename"
	fi
}

insert_at '^# DDNet++ configs$' '^# ' "\n$(gen_configs)" docs/settings_and_commands.md
# insert_at '^# Rcon commands$' '^# ' "\n$(gen_rcon_cmds)" docs/settings_and_commands.md
# insert_at '^# Chat commands$' '^# ' "$(gen_chat_cmds)" docs/settings_and_commands.md

[[ -f "$(tmp)" ]] && rm "$(tmp)"
exit 0
