#!/bin/sh
mkdir -p "${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}"
xcrun Rez -d AE_OS_MAC -i "${SRCROOT}/include/AE_SDK" -o "${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/DreamyMusic.rsrc" -useDF "${SRCROOT}/src/DreamyMusicPiPL.r"
cp -f "${SRCROOT}/Mac/PkgInfo" "${TARGET_BUILD_DIR}/${CONTENTS_FOLDER_PATH}/PkgInfo"

