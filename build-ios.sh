export GEODE_SDK=/Users/nyaauwu/Documents/geode-cgs
export GEODE_BINDINGS_REPO_PATH=/Users/nyaauwu/Documents/bindings
cmake -B build-ios -DGEODE_TARGET_PLATFORM=iOS \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-ios