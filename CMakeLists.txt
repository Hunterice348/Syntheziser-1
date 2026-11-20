cmake_minimum_required(VERSION 3.15)
project(HUNTERICE_ABYSS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Загрузка JUCE через FetchContent (используем стабильную версию 7.0.12)
include(FetchContent)
FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
)
FetchContent_MakeAvailable(juce)

# 1. Создание цели плагина
juce_add_plugin(HuntericeAbyss
    COMPANY_NAME "HUNTERICE"
    PLUGIN_NAME "ABYSS"
    PLUGIN_MANUFACTURER_CODE "Hunt"
    PLUGIN_CODE "Abys"
    FORMATS VST3
    PRODUCT_NAME "HUNTERICE ABYSS"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
)

# 2. !!! КРИТИЧЕСКИ ВАЖНО: Генерация JuceHeader.h !!!
juce_generate_juce_header(HuntericeAbyss)

# 3. Добавление исходных файлов
target_sources(HuntericeAbyss PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

# 4. Линковка необходимых модулей JUCE
target_link_libraries(HuntericeAbyss PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
)

# 5. Оптимизация: отключение ненужных функций для ускорения сборки и уменьшения размера
target_compile_definitions(HuntericeAbyss PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
