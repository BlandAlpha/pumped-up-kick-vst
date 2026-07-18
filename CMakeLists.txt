cmake_minimum_required(VERSION 3.24)

# Both must be set before project(): project() runs CMake's Darwin platform
# initialisation, which already creates these cache entries with empty
# defaults, so a later set(... CACHE ...) without FORCE silently does nothing
# and the build falls back to host-native arch and deployment target.
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build a universal binary")

project(PumpedUpKick VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.14
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(PumpedUpKick
    COMPANY_NAME                "PumpedUp Audio"
    BUNDLE_ID                   com.pumpedupaudio.pumpedupkick
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     TRUE
    PLUGIN_MANUFACTURER_CODE    PuAu
    PLUGIN_CODE                 Pukk
    FORMATS                     AU VST3 Standalone
    VST3_CATEGORIES             Fx Dynamics
    AU_MAIN_TYPE                kAudioUnitType_MusicEffect
    PRODUCT_NAME                "PumpedUpKick"
)

target_sources(PumpedUpKick PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
)

# Special Gothic (SIL OFL 1.1, see src/font/OFL.txt) compiled into the binary
# so the UI never depends on fonts installed on the user's system.
juce_add_binary_data(PumpedUpKickAssets
    SOURCES
        src/font/SpecialGothic-Medium.ttf
        src/font/SpecialGothicCondensedOne-Regular.ttf
)

target_compile_definitions(PumpedUpKick PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(PumpedUpKick
    PRIVATE
        PumpedUpKickAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
