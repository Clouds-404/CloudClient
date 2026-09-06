cmake_minimum_required(VERSION 3.15)
project(launcher Java)
find_package(Java 1.7 REQUIRED COMPONENTS Development)

include(UseJava)
set(CMAKE_JAVA_JAR_ENTRY_POINT org.Cloudlauncher.EntryPoint)
set(CMAKE_JAVA_COMPILE_FLAGS -target 8 -source 8)

set(SRC
    org/Cloudlauncher/EntryPoint.java
    org/Cloudlauncher/launcher/Launcher.java
    org/Cloudlauncher/launcher/impl/AbstractLauncher.java
    org/Cloudlauncher/launcher/impl/StandardLauncher.java
    org/Cloudlauncher/exception/ParameterNotFoundException.java
    org/Cloudlauncher/exception/ParseException.java
    org/Cloudlauncher/utils/Parameters.java
    org/Cloudlauncher/utils/ReflectionUtils.java
    org/Cloudlauncher/utils/logging/Level.java
    org/Cloudlauncher/utils/logging/Log.java
    org/Cloudlauncher/legacy/LegacyProxy.java
)

set(LEGACY_SRC
    legacy/org/Cloudlauncher/legacy/LegacyFrame.java
    legacy/org/Cloudlauncher/legacy/LegacyLauncher.java
    legacy/org/Cloudlauncher/legacy/fix/online/Handler.java
    legacy/org/Cloudlauncher/legacy/fix/online/OnlineFixes.java
    legacy/org/Cloudlauncher/legacy/fix/online/OnlineModeFix.java
    legacy/org/Cloudlauncher/legacy/fix/online/SkinFix.java
    legacy/org/Cloudlauncher/legacy/utils/Base64.java
    legacy/org/Cloudlauncher/legacy/utils/api/MojangApi.java
    legacy/org/Cloudlauncher/legacy/utils/api/Texture.java
    legacy/org/Cloudlauncher/legacy/utils/json/JsonParseException.java
    legacy/org/Cloudlauncher/legacy/utils/json/JsonParser.java
    legacy/org/Cloudlauncher/legacy/utils/url/ByteArrayUrlConnection.java
    legacy/org/Cloudlauncher/legacy/utils/url/UrlUtils.java
    legacy/net/minecraft/Launcher.java
    legacy/org/Cloudlauncher/legacy/LegacyProxy.java
)

add_jar(NewLaunch ${SRC})
add_jar(NewLaunchLegacy ${LEGACY_SRC} INCLUDE_JARS NewLaunch)
install_jar(NewLaunch "${JARS_DEST_DIR}")
install_jar(NewLaunchLegacy "${JARS_DEST_DIR}")
