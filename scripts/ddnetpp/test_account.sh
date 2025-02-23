#!/bin/bash

function test_account() {
	log "test account"
	log " - create account"

	sleep 0.5
	fifo "say /register foo bar bar" client1.fifo

	sleep 0.5
	fifo 'say /login foo bar' client1.fifo

	log " - profile"

	sleep 0.5
	fifo "say /profile email client1@zillyhuhn.com" client1.fifo

	sleep 0.5
	tr -d '\n' > client1.fifo <<- EOF
		say "/mc
		;profile twitter @chillerdragon
		;profile youtube chillerdragon
		;profile skype \"discord:chillerdragon@xxxx\"
		;profile skype \"invalid @[²¹[»ĸæ→@<script>\"
		;profile homepage zillyhuhn.com"
	EOF

	sleep 0.5
	fifo 'say "/mc;hide block_xp;hide xp;fng autojoin 1"' client1.fifo

	cp accounts.db before_logout.db
	[[ -f accounts.db-wal ]] && cp accounts.db-wal before_logout.db-wal
	[[ -f accounts.db-shm ]] && cp accounts.db-shm before_logout.db-shm

	sleep 0.5
	fifo "say /logout" client1.fifo

	sleep 0.5
	fifo "player_name client1_alt" client1.fifo

	sleep 0.5
	fifo "say /login foo bar" client1.fifo

	sleep 0.5
	fifo "say /insta leave" client1.fifo

	# wait 2 secs to respawn after /insta leave
	sleep 2
}
