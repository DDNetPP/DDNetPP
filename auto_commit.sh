#!/bin/bash
#much wow bash script to super simplyfy git commands
#mede by ChillerDragon for nobody

read -p "Commit message: " commit_msg

git add .
git commit -m "$commit_msg"
git push
