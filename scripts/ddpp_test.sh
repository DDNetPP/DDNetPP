#!/bin/bash

if [ ! -f ./scripts/ddpp_test.sh ] || [ ! -f CMakeLists.txt ]
then
	echo "Error: make sure your are in the root of the repo"
	exit 1
fi

arg_build_dir="build"
arg_end_args=0
arg_verbose=0

for arg in "$@"
do
	if [[ "${arg::1}" == "-" ]] && [[ "$arg_end_args" == "0" ]] 
	then
		if [ "$arg" == "-h" ] || [ "$arg" == "--help" ]
		then
			echo "usage: $(basename "$0") [OPTION..] [build dir]"
			echo "description:"
			echo "  Runs a simple integration test of the client and server"
			echo "  binaries from the given build dir"
			echo "options:"
			echo "  --help|-h     show this help"
			echo "	--verbose|-v  verbose output"
		elif [ "$arg" == "-v" ] || [ "$arg" == "--verbose" ]
		then
			arg_verbose=1
		elif [ "$arg" == "--" ]
		then
			arg_end_args=1
		else
			echo "Error: unknown arg '$arg'"
		fi
	else
		arg_build_dir="$arg"
	fi
done

if [ ! -d "$arg_build_dir" ]
then
	echo "Error: build directory '$arg_build_dir' not found"
	exit 1
fi
if [ ! -f "$arg_build_dir"/DDNet ]
then
	echo "Error: client binary not found '$arg_build_dir/DDNet' not found"
	exit 1
fi
if [ ! -f "$arg_build_dir"/DDNetPP ]
then
	echo "Error: server binary not found '$arg_build_dir/DDNetPP' not found"
	exit 1
fi

mkdir -p test
cp "$arg_build_dir"/DDNet* test

cd test || exit 1

function kill_all() {
	if [ "$arg_verbose" == "1" ]
	then
		echo "[*] shutting down test clients and server ..."
	fi
	sleep 1
	echo "shutdown" > server.fifo
	echo "quit" > client1.fifo
	echo "quit" > client2.fifo
}

got_cleanup=0

function cleanup() {
	# needed to fix hang fifo with additional ctrl+c
	if [ "$got_cleanup" == "1" ]
	then
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

function fail()
{
	sleep 1
	tail -n2 "$1".log > fail_"$1".txt
	echo "$1 exited with code $2" >> fail_"$1".txt
	echo "[-] $1 exited with code $2"
}

if test -n "$(find . -maxdepth 1 -name '*.fifo' -print -quit)"
then
	rm ./*.fifo
fi
if test -n "$(find . -maxdepth 1 -name 'SAN.*' -print -quit)"
then
	rm SAN.*
fi
if test -n "$(find . -maxdepth 1 -name 'fail_*' -print -quit)"
then
	rm ./fail_*
fi
if [ -f ddnet-server.sqlite ]
then
	rm ddnet-server.sqlite
fi
if [ -f accounts.db ]
then
	rm accounts.db
fi

# TODO: check for open ports instead
port=17822

cp ../ubsan.supp .

export UBSAN_OPTIONS=suppressions=./ubsan.supp:log_path=./SAN:print_stacktrace=1:halt_on_errors=0
export ASAN_OPTIONS=log_path=./SAN:print_stacktrace=1:check_initialization_order=1:detect_leaks=1:halt_on_errors=0

function print_san() {
	if test -n "$(find . -maxdepth 1 -name 'SAN.*' -print -quit)"
	then
		cat ./SAN.*
		return 1
	fi
	return 0
}


./DDNetPP \
	"sv_input_fifo server.fifo;
	sv_map ddnetpp-test;
	sv_sqlite_file ddnet-server.sqlite;
	sv_account_stuff 1;
	sv_database_path accounts.db;
	sv_port $port" &> server.log || fail server "$?" &

./DDNet \
	"cl_input_fifo client1.fifo;
	player_name client1;
	cl_download_skins 0;
	connect localhost:$port" &> client1.log || fail client1 "$?" &

sleep 0.5

./DDNet \
	"cl_input_fifo client2.fifo;
	player_name client2;
	cl_download_skins 0;
	connect localhost:$port" &> client2.log || fail client2 "$?" &

fails=0
# give the client time to launch and create the fifo file
# but assume after 3 secs that the client crashed before
# being able to create the file
while [[ ! -p client1.fifo ]]
do
	fails="$((fails+1))"
	if [ "$arg_verbose" == "1" ]
	then
		echo "[!] client fifo not found (attempts $fails/3)"
	fi
	if [ "$fails" -gt "2" ]
	then
		print_san
		echo "[-] Error: client possibly crashed on launch"
		exit 1
	fi
	sleep 1
done

sleep 1
echo "say /register foo bar bar" > client1.fifo

sleep 1
echo "say /login foo bar" > client1.fifo

sleep 5
echo "rcon_auth rcon" > client1.fifo

sleep 1
echo "rcon shutdown" > client1.fifo

kill_all
wait

sleep 1

accs="$(sqlite3 accounts.db < <(echo "select * from Accounts;"))"
if [ "$accs" == "" ]
then
	touch fail_accs.txt
	echo "[-] Error: no accounts found in database"
elif [ "$(echo "$accs" | wc -l)" != "1" ]
then
	touch fail_accs.txt
	echo "[-] Error: expected 1 account got $(echo "$accs" | wc -l)"
elif ! echo "$accs" | grep -q client1
then
	touch fail_accs.txt
	echo "[-] Error: expected an account from client1 instead got:"
	echo "  $accs"
else
	money="$(sqlite3 accounts.db < <(echo "select Money from Accounts;"))"
	if [ "$money" != "6" ]
	then
		touch fail_accs.txt
		echo "[-] Error: expected client1 to farm '6' money but got '$money'"
	fi
fi

if test -n "$(find . -maxdepth 1 -name 'fail_*' -print -quit)"
then
	if [ "$arg_verbose" == "1" ]
	then
		for fail in ./fail_*
		do
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
