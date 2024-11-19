#!/bin/bash

# called by test_tournament.sh
# while waiting to start tournament round

function check_ascii_frame() {
	local frame="$1"
	if ! grep broadcast client2.log | grep -qF "$frame"; then
		err "Did not find frame '$frame' in '/profile view'"
		touch fail_ascii.txt
	fi
}

function test_ascii() {
	sleep 8
	tr -d '\n' > client1.fifo <<- EOF
		say "/mc
		;ascii frame -1 invalid
		;ascii frame 0 foo
		;ascii frame 1 bar
		;ascii frame 2 baz
		;ascii profile 1"
	EOF

	sleep 1
	echo "say /profile view client1_alt" > client2.fifo

	sleep 1
	check_ascii_frame foo
	check_ascii_frame bar
	check_ascii_frame baz
}
