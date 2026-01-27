cmake_minimum_required(VERSION 3.20)
project(VoiceAgent LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check for CUDA
find_package(CUDAToolkit)
if(CUDAToolkit_FOUND)
    enable_language(CUDA)
    add_definitions(-DGGML_USE_CUBLAS) # For llama.cpp
endif()

# ONNX Runtime (Downloaded to libs/)
set(ORT_DIR ${CMAKE_SOURCE_DIR}/libs/onnxruntime-win-x64-1.19.0)
include_directories(${ORT_DIR}/include)
set(ORT_LIB ${ORT_DIR}/lib/onnxruntime.lib)

# Directories
include_directories(
    ${CMAKE_SOURCE_DIR}/audio
    ${CMAKE_SOURCE_DIR}/asr
    ${CMAKE_SOURCE_DIR}/llm
    ${CMAKE_SOURCE_DIR}/tts
    ${CMAKE_SOURCE_DIR}/controller
    ${CMAKE_SOURCE_DIR}/persona
)

# -----------------------------------------------------------------------------
# DEPENDENCIES
# -----------------------------------------------------------------------------

# 1. PortAudio
set(PA_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(PA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(PA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(audio/portaudio)
include_directories(audio/portaudio/include)
set(PORTAUDIO_LIB portaudio)

# 2. Llama.cpp
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(llm/llama_cpp)
include_directories(llm/llama_cpp/include)
include_directories(llm/llama_cpp)

# 3. Whisper.cpp
set(WHISPER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(asr/whisper_cpp)
include_directories(asr/whisper_cpp/include)


# -----------------------------------------------------------------------------
# SOURCES
# -----------------------------------------------------------------------------
set(SOURCES
    main.cpp
    audio/mic_stream.cpp
    audio/vad.cpp
    audio/silero_vad.cpp
    controller/dialogue_controller.cpp
    persona/persona_state.cpp
    tts/tts_stream.cpp
    tts/simple_tts.cpp
    llm/llama_stream.cpp
    asr/whisper_stream.cpp
    utils/perf_monitor.cpp
)

add_executable(voice_agent ${SOURCES})

# LINK LIBRARIES
target_link_libraries(voice_agent
    PRIVATE
    ${PORTAUDIO_LIB}
    whisper
    llama
    ${ORT_LIB}
)

if(CUDAToolkit_FOUND)
    target_link_libraries(voice_agent PRIVATE CUDA::cudart)
endif()
