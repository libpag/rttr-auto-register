#!/bin/bash



cert='Developer ID Application: Tencent Technology (Shanghai) Co., Ltd (FN2V63AD2J)'

APP="RttrAutoRegister"
# EXECUTABLE="${APP}/Contents/MacOS/RttrAutoRegister"
EXECUTABLE="${APP}"
KEYCHAIN_PROFILE="AC_PASSWORD"
TMPDIR="Applications"
ZIP="Applications.zip"

echo "Code sign"
codesign --deep --force -s "${cert}" --options=runtime --timestamp "${EXECUTABLE}"
echo " "

echo "codesign validate"
codesign -v -vvv --strict --deep --verbose=4 "${APP}"
echo " "

rm -fr "${ZIP}"
rm -fr "${TMPDIR}"
mkdir "${TMPDIR}"

# cp -RP "${APP}" "${TMPDIR}"
cp "${EXECUTABLE}" "${TMPDIR}"

zip --symlinks -r -q -X "${ZIP}" "./${TMPDIR}"

echo "Notarize"
xcrun notarytool submit --keychain-profile "${KEYCHAIN_PROFILE}" --wait "${ZIP}" 2>&1 | tee tmp
echo " "

if [[ $? -ne 0 ]]; then
    echo "Failed to submit app for notarization."
    cat tmp
    exit 1
fi
echo tmp
# 提取 UUID
UUID=$(cat tmp | grep -Eo '\w{8}-(\w{4}-){3}\w{12}' | head -n 1)
echo "Submission successful. UUID: $UUID"

while true; do
    echo "Checking notarization status..."
    xcrun notarytool info "$UUID" --keychain-profile "${KEYCHAIN_PROFILE}" 2>&1 | tee tmp
    cat tmp
    # 检查输出
    STATUS=$(cat tmp | grep "status" | awk '{print $2}' | tr -d '"')
    echo "Current status: $STATUS"

    if [[ "$STATUS" == "Accepted" ]]; then
        echo "Notarization successful!"
        break
    elif [[ "$STATUS" == "Invalid" ]]; then
        echo "Notarization failed."
        cat tmp
        exit 1
    else
        echo "Notarization not completed yet. Waiting 20 seconds..."
        sleep 20
    fi
done

rm -fr "${ZIP}"
rm -fr "${TMPDIR}"

echo " "
echo "Notarize log:"
xcrun notarytool log "$UUID" --keychain-profile "${KEYCHAIN_PROFILE}"
echo " "

echo "xcrun stabler staple"
xcrun stapler staple "${APP}"
echo " "

echo "xcrun stapler validate"
xcrun stapler validate "${APP}"
echo " "


