#!/bin/bash

# Ensure we're in the top-level Git directory
if [ ! -d ".git" ]; then
    echo "❌ Error: This is not a Git repository."
    exit 1
fi

# Prompt for commit message
read -p "📝 Enter commit message: " msg

# Run Git commands
git add .
git commit -m "$msg"
git push -u origin main

echo "✅ Pushed to origin main with message: \"$msg\""

