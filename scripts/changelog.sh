#!/bin/bash

# ddnet++

# This script should be run after
# the release tag was created
# then it will generate a changelog
# from the current release to the previous tag

current_tag="$(git --no-pager tag --sort=-creatordate | head -n1)"
# the [0-9][0-9] exclude should match all ddnet tags like 18.7
# so we only get ddnet-insta releases here
previous_tag="$(git describe --tags --abbrev=0 "$current_tag"^ --exclude '[0-9][0-9].*')"

printf "# Changelog\n\n"

tag_date=$(git log -1 --pretty=format:'%ad' --date=short "$current_tag")
printf "## %s (%s)\n\n" "$current_tag" "$tag_date"
while read -r commit; do
	[ "$(git branch ddnet --contains "$commit")" == "" ] || continue

	line="$(git \
		show "$commit" \
		--pretty=format:"%h" \
		--no-patch \
		--pretty=format:'* %s [View](https://github.com/DDNetPP/DDNetPP/commit/%H) @%an')"
	if [[ "$line" = *"@ChillerDragon" ]] || [[ "$line" = *"@Chiller Dragon" ]]; then
		line="${line%@*}"
	fi
	[[ "$line" = '* Merge branch '* ]] && continue

	printf '%s\n' "$line"
done < <(git --no-pager log "${previous_tag}...$current_tag" --pretty=format:'%H')
printf "\n\n"
