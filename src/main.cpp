#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <string>
#include <string_view>
#include <filesystem>
#include <charconv>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Core/Globals.h"
#include "Scene/Scene.h"
#include "Integrator/Integrator.h"

static bool cmd_check_flag(char **begin, char **end, const std::string& short_flag, const std::string& flag) {
    // try short variant
    char** itr = nullptr;
    if (!short_flag.empty()) {
        itr = std::find(begin, end, "-" + short_flag);
        if (itr != end) {
            return true;
        }
    }

    // try long variant
    if (!flag.empty()) {
        itr = std::find(begin, end, "--" + flag);
        return itr != end;
    }

    return false;
}

static char* cmd_find_option(char** begin, char** end, const std::string& short_option, const std::string& option) {
    // try short variant
    if (!short_option.empty()) {
        char **itr = std::find(begin, end, "-" + short_option);
        if (itr != end && ++itr != end) {
            return *itr;
        }
    }
    
    // try long variant
    if (!option.empty()) {
        char **itr = std::find(begin, end, "--" + option);
        if (itr != end && ++itr != end) {
            return *itr;
        }
    }
    
    return nullptr;
}

static void print_usage() {
    printf("Usage of ./Helios :\n");
    printf("-i path/filename\n");
    printf("\t Specify scene input file (Current supported scene formats: glTF 2.0)\n");
    printf("-o path/filename\n");
    printf("\t Specify output filename\n");
    printf("--width value\n");
    printf("\t Specify image width (default value 1920)\n");
    printf("--height value\n");
    printf("\t Specify image height (default value 1080)\n");
}

static void print_error() {
    printf("Wrong usage. Use '-h', '--help' for help\n");
}

void error_callback(void*, enum RTCError error, const char* str) {
    printf("Embree error %d: %s\n", error, str);
}

static void run(const char* input, const char* output, int width, int height) {
    g_Device = rtcNewDevice(nullptr);

    if (!g_Device) {
        exit(EXIT_FAILURE);
    }
    rtcSetDeviceErrorFunction(g_Device, error_callback, nullptr);

    Helios::Scene* scene = Helios::Scene::LoadFromFile(input, width, height);

    if (!scene) {
        printf("Failed to load scene: %s\n", input);
        rtcReleaseDevice(g_Device);
        exit(EXIT_FAILURE);
    }

    Helios::Integrator integrator;

    std::vector<Helios::Spectrum> buffer_float;
    printf("Info: Rendering...\n");
    integrator.Render(buffer_float, *scene, width, height);

    std::vector<uint8_t> buffer_bytes;
    buffer_bytes.resize(3*buffer_float.size());

    printf("Info: Writing result to file...\n");
    for (unsigned int i = 0; i < buffer_float.size(); i++) {
        buffer_bytes[3*i] = floor(buffer_float[i].r >= 1.0f ? 255 : buffer_float[i].r * 256.0f);
        buffer_bytes[3*i + 1] = floor(buffer_float[i].g >= 1.0f ? 255 : buffer_float[i].g * 256.0f);
        buffer_bytes[3*i + 2] = floor(buffer_float[i].b >= 1.0f ? 255 : buffer_float[i].b * 256.0f);
    }
    printf("Info: Done!\n");
    
    stbi_write_png(output, width, height, 3, buffer_bytes.data(), width*3);

    delete scene;
    rtcReleaseDevice(g_Device);
}


// Globals:
RTCDevice g_Device;

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if (argc > 1) {
        if (cmd_check_flag(argv, argv + argc, "h", "help")) {
            print_usage();
            return 0;
        }

        char* scene_filename = cmd_find_option(argv, argv + argc, "i", "");
        if (!scene_filename) {
            print_error();
            exit(EXIT_FAILURE);
        }
        else {
            std::string extension = fs::path(scene_filename).extension().u8string();
            if (extension != ".gltf") {
                printf("Error: Wrong scene format %s, only glTF 2.0 is supported currently\n", extension.c_str());
                exit(EXIT_FAILURE);
            }
        }

        char* output_image = cmd_find_option(argv, argv + argc, "o", "");
        if (!output_image) {
            print_error();
            exit(EXIT_FAILURE);
        }
        else {
            std::string extension = fs::path(output_image).extension().u8string();
            if (extension != ".png") {
                printf("Error: Wrong output image format %s, only PNG is supported currently\n", extension.c_str());
                exit(EXIT_FAILURE);
            }
        }

        int width = 1920;
        int height = 1080;

        char* width_option = cmd_find_option(argv, argv + argc, "", "width");
        if (width_option) {
            std::string_view width_sv = std::string_view(width_option);
            auto result = std::from_chars(width_sv.data(), width_sv.data() + width_sv.size(), width);
            if (result.ec == std::errc::invalid_argument) {
                print_error();
                exit(EXIT_FAILURE);
            }
        }        

        char* height_option = cmd_find_option(argv, argv + argc, "", "height");
        if (height_option) {
            std::string_view height_sv = std::string_view(height_option);
            auto result = std::from_chars(height_sv.data(), height_sv.data() + height_sv.size(), height);
            if (result.ec == std::errc::invalid_argument) {
                print_error();
                exit(EXIT_FAILURE);
            }
        }

        run(scene_filename, output_image, width, height);
    }
    else {
        print_error();
        exit(EXIT_FAILURE);
    }
    return 0;
}