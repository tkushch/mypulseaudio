cmake_minimum_required(VERSION 3.26)
project(mypulseaudio)

set(CMAKE_CXX_STANDARD 17)

# Указываем путь к заголовочным файлам PulseAudio
include_directories(/opt/homebrew/Cellar/pulseaudio/16.1/include)

# Ваш исходный код
add_executable(mypulseaudio main.cpp)

# Указываем библиотеки PulseAudio
find_library(PULSE_LIBRARY pulse)
find_library(PULSE_SIMPLE_LIBRARY pulse-simple)

# Проверка, найдены ли библиотеки
if(NOT PULSE_LIBRARY OR NOT PULSE_SIMPLE_LIBRARY)
    message(FATAL_ERROR "Библиотеки PulseAudio не найдены.")
endif()

# Линкуем с библиотеками PulseAudio
target_link_libraries(mypulseaudio ${PULSE_LIBRARY} ${PULSE_SIMPLE_LIBRARY})
