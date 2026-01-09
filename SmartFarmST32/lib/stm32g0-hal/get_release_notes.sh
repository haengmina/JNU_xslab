#/bin/bash

cd $(dirname $0)

if [ -z "${CI_PROJECT_URL}" ]; then
    export CI_PROJECT_URL=$(git remote get-url origin | sed -E 's@.git$|.git/$@@g')
fi

git cliff $@