#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <string>


#include "globals.h"
#include "scene.h"
#include "renderer.h"

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
    printf("Here usage will be described\n");
}

static void print_error() {
    printf("Wrong usage. Use '-h', '--help' for help\n");
}

void error_callback(void*, enum RTCError error, const char* str) {
    printf("Embree error %d: %s\n", error, str);
}

static void run(const char* input, const char*) {
    g_Device = rtcNewDevice(nullptr);

    if (!g_Device) {
        exit(EXIT_FAILURE);
    }
    rtcSetDeviceErrorFunction(g_Device, error_callback, nullptr);

    Helios::Scene* scene = Helios::Scene::LoadFromFile(input);

    if (!scene) {
        printf("Failed to load scene: %s\n", input);
        rtcReleaseDevice(g_Device);
        exit(EXIT_FAILURE);
    }

    Helios::Renderer renderer;

    renderer.Draw(*scene, 1920, 1080);

    delete scene;
    rtcReleaseDevice(g_Device);
}


// Globals:
RTCDevice g_Device;

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

        char* output_image = cmd_find_option(argv, argv + argc, "o", "");
        if (!output_image) {
            print_error();
            exit(EXIT_FAILURE);
        }

        run(scene_filename, output_image);
    }
    else {
        print_error();
        exit(EXIT_FAILURE);
    }
    return 0;
}