#!/bin/bash

if [ ! -f ./scripts/ddpp_test.sh ] || [ ! -f CMakeLists.txt ]; then
	echo "Error: make sure your are in the root of the repo"
	exit 1
fi

source ./scripts/ddnetpp/logger.sh
source ./scripts/ddnetpp/test_ascii.sh # will be called by test_tournament.sh
source ./scripts/ddnetpp/test_tournament.sh
source ./scripts/ddnetpp/test_account.sh

arg_build_dir="build"
arg_end_args=0
arg_verbose=0

for arg in "$@"; do
	if [[ "${arg::1}" == "-" ]] && [[ "$arg_end_args" == "0" ]]; then
		if [ "$arg" == "-h" ] || [ "$arg" == "--help" ]; then
			echo "usage: $(basename "$0") [OPTION..] [build dir]"
			echo "description:"
			echo "  Runs a simple integration test of the client and server"
			echo "  binaries from the given build dir"
			echo "options:"
			echo "  --help|-h     show this help"
			echo "	--verbose|-v  verbose output"
		elif [ "$arg" == "-v" ] || [ "$arg" == "--verbose" ]; then
			arg_verbose=1
		elif [ "$arg" == "--" ]; then
			arg_end_args=1
		else
			echo "Error: unknown arg '$arg'"
		fi
	else
		arg_build_dir="$arg"
	fi
done

if [ ! -d "$arg_build_dir" ]; then
	echo "Error: build directory '$arg_build_dir' not found"
	exit 1
fi
if [ ! -f "$arg_build_dir"/DDNet ]; then
	echo "Error: client binary not found '$arg_build_dir/DDNet' not found"
	exit 1
fi
if [ ! -f "$arg_build_dir"/DDNetPP ]; then
	echo "Error: server binary not found '$arg_build_dir/DDNetPP' not found"
	exit 1
fi

mkdir -p integration_test
cp "$arg_build_dir"/DDNet* integration_test

cd integration_test || exit 1

function kill_all() {
	if [ "$arg_verbose" == "1" ]; then
		echo "[*] shutting down test clients and server ..."
	fi
	local srv_pid
	local client1_pid
	local client2_pid
	sleep 1
	echo "shutdown" > server.fifo &
	srv_pid="$!"
	echo "quit" > client1.fifo &
	client1_pid="$!"
	echo "quit" > client2.fifo &
	client2_pid="$!"
	sleep 1
	kill -9 "$srv_pid" &> /dev/null
	kill -9 "$client1_pid" &> /dev/null
	kill -9 "$client2_pid" &> /dev/null
}

function fifo() {
	local cmd="$1"
	local fifo_file="$2"
	if [ -f fail_fifo_timeout.txt ]; then
		echo "[fifo] skipping because of timeout cmd: $cmd"
		return
	fi
	if [ "$arg_verbose" == "1" ]; then
		echo "[fifo] $cmd >> $fifo_file"
	fi
	if printf '%s' "$cmd" | grep -q '[`'"'"']'; then
		echo "[-] fifo commands can not contain backticks or single quotes"
		echo "[-] invalid fifo command: $cmd"
		exit 1
	fi
	if ! timeout 3 sh -c "printf '%s\n' '$cmd' >> \"$fifo_file\""; then
		fifo_error="[-] fifo command timeout: $cmd >> $fifo_file"
		printf '%s\n' "$fifo_error"
		printf '%s\n' "$fifo_error" >> fail_fifo_timeout.txt
		kill_all
		echo 'FIFO ERROR!!!'
		exit 1
	fi
}

got_cleanup=0

function cleanup() {
	# needed to fix hang fifo with additional ctrl+c
	if [ "$got_cleanup" == "1" ]; then
		exit
	fi
	got_cleanup=1
	kill_all
}

trap cleanup EXIT

{
	echo $'add_path $CURRENTDIR'
	echo $'add_path $USERDIR'
	echo $'add_path $DATADIR'
	echo $'add_path ../data'
} > storage.cfg

function fail() {
	local parent_pid="$1"
	local type="$2"
	local exit_code="$3"
	sleep 1
	if [ "$type" == "server" ]; then
		tail server.log
	fi
	tail -n2 "$type".log > fail_"$type".txt
	echo "$type exited with code $exit_code" >> fail_"$type".txt
	echo "[-] $type exited with code $exit_code"
	kill_all
	kill -9 "$parent_pid"
	exit 1
}

if test -n "$(find . -maxdepth 1 -name '*.fifo' -print -quit)"; then
	rm ./*.fifo
fi
if test -n "$(find . -maxdepth 1 -name 'SAN.*' -print -quit)"; then
	rm SAN.*
fi
if test -n "$(find . -maxdepth 1 -name 'fail_*' -print -quit)"; then
	rm ./fail_*
fi
if [ -f ddnet-server.sqlite ]; then
	rm ddnet-server.sqlite
fi
if [ -f accounts.db ]; then
	rm accounts.db
fi

# TODO: check for open ports instead
port=17822

cp ../ubsan.supp .

export UBSAN_OPTIONS=suppressions=./ubsan.supp:log_path=./SAN:print_stacktrace=1:halt_on_errors=0
export ASAN_OPTIONS=log_path=./SAN:print_stacktrace=1:check_initialization_order=1:detect_leaks=1:halt_on_errors=0

function print_san() {
	if test -n "$(find . -maxdepth 1 -name 'SAN.*' -print -quit)"; then
		cat ./SAN.*
		return 1
	fi
	return 0
}

log "connecting clients to server at port $port"

./DDNetPP \
	"sv_input_fifo server.fifo;
	sv_map ddnetpp-test;
	sv_sqlite_file ddnet-server.sqlite;
	sv_register ipv4;
	sv_accounts 1;
	sv_database_path accounts.db;
	sv_allow_block_tourna 1;
	sv_block_tourna_players 2;
	sv_block_tourna_delay 5;
	sv_port $port" &> server.log || fail "$$" server "$?" &

./DDNet \
	"cl_input_fifo client1.fifo;
	player_name client1;
	cl_download_skins 0;
	connect localhost:$port" &> client1.log || fail "$$" client1 "$?" &

sleep 0.5

./DDNet \
	"cl_input_fifo client2.fifo;
	player_name client2;
	cl_download_skins 0;
	connect localhost:$port" &> client2.log || fail "$$" client2 "$?" &

fails=0
# give the client time to launch and create the fifo file
# but assume after 3 secs that the client crashed before
# being able to create the file
while [[ ! -p client1.fifo ]]; do
	fails="$((fails + 1))"
	if [ "$arg_verbose" == "1" ]; then
		echo "[!] client fifo not found (attempts $fails/3)"
	fi
	if [ "$fails" -gt "2" ]; then
		print_san
		echo "[-] Error: client possibly crashed on launch"
		exit 1
	fi
	sleep 1
done

sleep 5
fifo "rcon_auth rcon" client1.fifo

test_account
test_tournament

sleep 1
fifo "rcon shutdown" client1.fifo

kill_all
wait

sleep 1

function check_account() {
	# check_account <sql column> <expected value> [database]
	#
	#   example:
	#	check_account ProfileEmail client1@zillyhuhn.com
	local column="$1"
	local expected="$2"
	local database="${3:-accounts.db}"
	local from=''
	local to=''
	if [[ "$expected" =~ '-' ]]; then
		from="$(echo "$expected" | cut -d'-' -f1)"
		to="$(echo "$expected" | cut -d'-' -f2)"
		if [ "$from" -gt "$to" ]; then
			echo "[-] Error: from has to be smaller than to."
			exit 1
		fi
	fi
	got="$(sqlite3 -init /dev/null "$database" < <(echo "select $column from Accounts;"))"
	if [ "$from" != "" ] && [ "$to" != "" ]; then
		if [ "$got" -gt "$to" ] || [ "$got" -lt "$from" ]; then
			touch fail_accs.txt
			local db_note=''
			if [ "$database" != "accounts.db" ]; then
				db_note=" ($database)"
			fi
			echo "[-] Error: Expected $column to be in range '$expected' but got '$got'$db_note"
		fi
	elif [ "$got" != "$expected" ]; then
		touch fail_accs.txt
		local db_note=''
		if [ "$database" != "accounts.db" ]; then
			db_note=" ($database)"
		fi
		echo "[-] Error: Expected $column to be '$expected' but got '$got'$db_note"
	fi
}

accs="$(sqlite3 -init /dev/null accounts.db < <(echo "select * from Accounts;"))"
if [ "$accs" == "" ]; then
	touch fail_accs.txt
	echo "[-] Error: no accounts found in database"
elif [ "$(echo "$accs" | wc -l)" != "1" ]; then
	touch fail_accs.txt
	echo "[-] Error: expected 1 account got $(echo "$accs" | wc -l)"
elif ! echo "$accs" | grep -q client1; then
	touch fail_accs.txt
	echo "[-] Error: expected an account from client1 instead got:"
	echo "  $accs"
else
	user="$(sqlite3 -init /dev/null accounts.db < <(echo "
		select * from Accounts
		where LastLogoutIGN1 = 'client1_alt'
		and Username = 'foo';
	"))"
	if [ "$user" == "" ]; then
		touch fail_accs.txt
		echo "[-] Error: user client1 with account foo not found"
	fi
	check_account LastLogoutIGN2 client1
	check_account Shit 1
	check_account Money '1-5'
	check_account Level 0
	check_account ProfileEmail client1@zillyhuhn.com
	check_account ProfileHomepage zillyhuhn.com
	check_account ProfileTwitter @chillerdragon
	check_account ProfileYoutube chillerdragon
	check_account ProfileSkype "discord:chillerdragon@xxxx"
	check_account ShowHideConfig 0010000000
	check_account FngConfig 100
	check_account IsLoggedIn 0
	check_account IsLoggedIn 1 before_logout.db
	check_account IsModerator 0
	check_account IsAccFrozen 0
	check_account GrenadeKills 0
	check_account GrenadeDeaths 0
	check_account GrenadeShots 0
	check_account SpookyGhost 0
	check_account LastLoginPort "$port"
	check_account AsciiFrame0 foo
	check_account AsciiFrame1 bar
	check_account AsciiFrame2 baz
fi

if test -n "$(find . -maxdepth 1 -name 'fail_*' -print -quit)"; then
	if [ "$arg_verbose" == "1" ]; then
		for fail in ./fail_*; do
			cat "$fail"
		done
	fi
	print_san
	echo "[-] Test failed. See errors above."
	exit 1
else
	echo "[*] all tests passed"
fi

print_san || exit 1
