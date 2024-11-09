#!/bin/bash

function test_tournament() {
	log "test tournaments"

	sleep 1
	fifo "rcon start_block_tournament" client1.fifo

	sleep 1
	fifo "say /join" client1.fifo

	sleep 1
	fifo "say /join" client2.fifo

	# 5 seconds of delay until round starts
	sleep 5
	fifo "say /buy shit" client1.fifo

	# 10 seconds until selfkill is allowed
	# use this time to run some ascii tests
	test_ascii

	sleep 1
	fifo "kill" client1.fifo
}

