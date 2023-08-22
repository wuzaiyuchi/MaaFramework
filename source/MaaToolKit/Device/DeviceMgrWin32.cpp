#if defined(_WIN32)

#include "Utils/SafeWindows.hpp"

#include "DeviceMgrWin32.h"

#include <filesystem>
#include <map>

#include <Psapi.h>

#include "AdbConfigDef.hpp"
#include "Utils/Logger.h"
#include "Utils/Platform.h"
#include "Utils/Ranges.hpp"

MAA_TOOLKIT_DEVICE_NS_BEGIN

using namespace path_literals;

std::ostream& operator<<(std::ostream& os, const DeviceMgrWin32::Emulator& emulator)
{
    os << VAR_RAW(emulator.name) << VAR_RAW(emulator.process);
    return os;
}

struct EmulatorConstantData
{
    std::string keyword;
    std::vector<std::filesystem::path> adb_candidate_paths;
    std::vector<std::string> adb_common_serials;
};

std::filesystem::path get_adb_path(const EmulatorConstantData& emulator, os_pid pid);

static const std::map<std::string, EmulatorConstantData> kEmulators = {
    { "BlueStacks",
      { .keyword = "HD-Player",
        .adb_candidate_paths = { "HD-Adb.exe"_path, "Engine\\ProgramFiles\\HD-Adb.exe"_path },
        .adb_common_serials = { "127.0.0.1:5555", "127.0.0.1:5556", "127.0.0.1:5565", "127.0.0.1:5575",
                                "127.0.0.1:5585", "127.0.0.1:5595", "127.0.0.1:5554" } } },

    { "LDPlayer",
      { .keyword = "dnplayer",
        .adb_candidate_paths = { "adb.exe"_path },
        .adb_common_serials = { "emulator-5554", "emulator-5556", "emulator-5558", "emulator-5560", "127.0.0.1:5555",
                                "127.0.0.1:5556", "127.0.0.1:5554" } } },

    { "Nox",
      { .keyword = "Nox",
        .adb_candidate_paths = { "nox_adb.exe"_path },
        .adb_common_serials = { "127.0.0.1:62001", "127.0.0.1:59865" } } },

    { "MuMuPlayer6",
      { .keyword = "NemuPlayer",
        .adb_candidate_paths = { "vmonitor\\bin\\adb_server.exe"_path,
                                 "MuMu\\emulator\\nemu\\vmonitor\\bin\\adb_server.exe"_path, "adb.exe"_path },
        .adb_common_serials = { "127.0.0.1:7555" } } },

    { "MuMuPlayer12",
      { .keyword = "MuMuPlayer",
        .adb_candidate_paths = { "vmonitor\\bin\\adb_server.exe"_path,
                                 "MuMu\\emulator\\nemu\\vmonitor\\bin\\adb_server.exe"_path, "adb.exe"_path },
        .adb_common_serials = { "127.0.0.1:16384", "127.0.0.1:16416", "127.0.0.1:16448", "127.0.0.1:16480",
                                "127.0.0.1:16512", "127.0.0.1:16544", "127.0.0.1:16576" } } },

    { "MEmuPlayer",
      { .keyword = "MEmu", .adb_candidate_paths = { "adb.exe"_path }, .adb_common_serials = { "127.0.0.1:21503" } } },
};

std::vector<Device> DeviceMgrWin32::find_device_impl(std::string_view specified_adb)
{
    // TODO
    std::ignore = specified_adb;

    std::vector<Device> result;

    auto all_emulators = find_emulators();
    for (const Emulator& e : all_emulators) {
        const auto& constant = kEmulators.at(e.name);
        std::filesystem::path adb_path = get_adb_path(constant, e.process.pid);

        Device device;
        device.name = e.name;
        device.adb_path = path_to_utf8_string(adb_path);
        device.adb_config = kAdbConfig.to_string();

        // TODO
        // device.adb_serial = ;
        // device.adb_controller_type = ;

        result.emplace_back(std::move(device));
    }

    return result;
}

std::vector<DeviceMgrWin32::Emulator> DeviceMgrWin32::find_emulators() const
{
    std::vector<Emulator> result;

    auto all_processes = list_processes();
    for (const auto& process : all_processes) {
        auto find_it = MAA_RNS::ranges::find_if(kEmulators, [&process](const auto& pair) -> bool {
            return process.name.find(pair.second.keyword) != std::string::npos;
        });
        if (find_it == kEmulators.cend()) {
            continue;
        }

        Emulator emulator {
            .name = find_it->first,
            .process = process,
        };
        result.emplace_back(std::move(emulator));
    }

    LogInfo << VAR(result);

    return result;
}

std::filesystem::path get_adb_path(const EmulatorConstantData& emulator, os_pid pid)
{
    auto path_opt = get_process_path(pid);
    if (!path_opt) {
        return {};
    }
    auto dir = path_opt->parent_path();

    for (const auto& adb_rel_path : emulator.adb_candidate_paths) {
        auto adb_path = dir / adb_rel_path;
        if (!std::filesystem::exists(adb_path)) {
            continue;
        }
        return adb_path;
    }
    return {};
}

MAA_TOOLKIT_DEVICE_NS_END

#endif
