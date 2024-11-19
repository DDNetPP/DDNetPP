#!/bin/bash

function log() {
	printf '[*] %s\n' "$1"
}
function wrn() {
	printf '[!] %s\n' "$1"
}
function succ() {
	printf '[+] %s\n' "$1"
}
function err() {
	printf '[-] %s\n' "$1"
}
