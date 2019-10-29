#!/bin/bash
set -o errexit
set -o pipefail
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
root=$(pwd)
projects=""
projects+=" account_directory_page_tester"
projects+=" account_page_tester"
projects+=" compliance_page_tester"
projects+=" create_account_page_tester"
projects+=" dashboard_page_tester"
projects+=" entitlements_page_tester"
projects+=" loading_page_tester"
projects+=" login_page_tester"
projects+=" profile_page_tester"
projects+=" risk_page_tester"
projects+=" scratch"
for i in $projects; do
  if [ ! -d "$i" ]; then
    mkdir "$i"
  fi
  pushd "$i"
  "$directory/$i/build.sh" "$@"
  popd
done
