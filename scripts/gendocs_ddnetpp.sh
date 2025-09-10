#!/bin/bash

CONFIG_HEADER="src/engine/shared/config_variables_ddpp.h"
RCON_COMMANDS_HEADER="src/game/ddracecommands_ddpp.h"
CHAT_COMMANDS_HEADER="src/game/ddracechat_ddpp.h"
OUTPUT_FILE="docs/settings_and_commands.md"
TEMP_DIR="ddpp_scripts"
TEMP_FILE="$TEMP_DIR/tmp.swp"

mkdir -p "$TEMP_DIR"

arg_is_dry=0

for arg in "$@"; do
	case "$arg" in
	--dry-run)
		arg_is_dry=1
		;;
	*)
		echo "Unknown argument: $arg"
		echo "Usage: $0 [--dry-run]"
		exit 1
		;;
	esac
done

gen_configs() {
	local cfg
	local desc
	local cmd
	while read -r cfg; do
		desc="$(echo "$cfg" | cut -d',' -f7- | cut -d'"' -f2-)"
		desc="${desc::-2}"
		cmd="$(echo "$cfg" | cut -d',' -f2 | xargs)"
		echo "+ \`$cmd\` $desc"
	done < <(grep '^MACRO_CONFIG_INT' "$CONFIG_HEADER")
	while read -r cfg; do
		desc="$(echo "$cfg" | cut -d',' -f6- | cut -d'"' -f2-)"
		desc="${desc::-2}"
		cmd="$(echo "$cfg" | cut -d',' -f2 | xargs)"
		echo "+ \`$cmd\` $desc"
	done < <(grep '^MACRO_CONFIG_STR' "$CONFIG_HEADER")
}

gen_console_cmds() {
  local type="$1"
	local prefix="$2"
	local header_file="$3"
  local cfg
  local desc
  local cmd
  while read -r cfg; do
    cfg="$(echo "$cfg" | sed 's|//.*||')"
    desc="$(echo "$cfg" | cut -d',' -f3- | cut -d'"' -f2)"
    cmd="$(echo "$cfg" | cut -d',' -f1 | cut -d'"' -f2)"
    echo "+ \`$prefix$cmd\` $desc"
  done < <(grep "^$type" "$header_file")
}

gen_rcon_cmds() {
	gen_console_cmds "CONSOLE_COMMAND" "" "$RCON_COMMANDS_HEADER"
}

gen_chat_cmds() {
	gen_console_cmds "CHAT_COMMAND" "/" "$CHAT_COMMANDS_HEADER"
}

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
	} > "$TEMP_FILE"
	if [ "$arg_is_dry" == "1" ]; then
		if [ "$(cat "$TEMP_FILE")" != "$(cat "$filename")" ]; then
			echo "Error: missing docs for $filename"
			echo "       run ./scripts/gendocs_ddnetpp.sh"
			git diff --no-index --color "$filename" "$TEMP_FILE"
			exit 1
		fi
	else
		mv "$TEMP_FILE" "$filename"
	fi
}

insert_at '^# DDNet++ configs$' '^# ' "\n$(gen_configs)" "$OUTPUT_FILE"
insert_at '^# Rcon commands$' '^# ' "\n$(gen_rcon_cmds)" "$OUTPUT_FILE"
insert_at '^# Chat commands$' '^# ' "$(gen_chat_cmds)" "$OUTPUT_FILE"

[[ -f "$TEMP_FILE" ]] && rm "$TEMP_FILE"

if [ "$arg_is_dry" == "1" ]; then
	echo "Dry-run completed successfully. Documentation is up to date."
else
	echo "Documentation updated successfully."
fi

exit 0
