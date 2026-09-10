#!/usr/bin/env python3
"""Compile ADC components for the configured SoCs, without linking or uploading."""

import argparse
import configparser
import json
from pathlib import Path
import subprocess
import sys

ENVIRONMENTS = (
    "adc1_c3_arduino2",
    "adc1_esp32_arduino3",
    "adc1_c3_arduino3",
    "adc1_s2_arduino3",
    "adc1_s3_arduino3",
    "adc1_c5_arduino3",
    "adc1_c6_arduino3",
    "adc1_h2_arduino3",
    "adc1_p4_arduino3",
)
SOURCES = (
    "Platform/Arduino/Sensors/NtcTemperatureSensor.cpp",
    "Platform/Arduino/Sensors/ACS712C30ACurrentSensor.cpp",
    "Platform/Arduino/Sensors/ResistiveDividerVoltageSensor.cpp",
    "Platform/Arduino/Factories/ArduinoHardwareAdapterFactory.cpp",
    "Infra/Factories/SensorFactory.cpp",
    "App/Builders/Builders/CapabilitiesBuilder.cpp",
)


def compile_arduino3(root, env, args, log):
    config = configparser.ConfigParser(interpolation=None)
    config.read(root / "configs/adc1_build.ini")
    board = json.loads((args.boards / (config[f"env:{env}"]["board"] + ".json")).read_text())
    target = board["build"]["mcu"]
    sdk = args.arduino3_sdk / target
    if target in ("esp32", "esp32s2", "esp32s3"):
        compiler = args.toolchains / f"toolchain-xtensa-esp-elf/bin/xtensa-{target}-elf-g++"
    else:
        compiler = args.toolchains / "toolchain-riscv32-esp/bin/riscv32-esp-elf-g++"
    sdkconfig = sorted(sdk.glob("*/include/sdkconfig.h"))[0].parent
    framework = args.arduino3_source
    includes = [root / "src", framework / "cores/esp32",
                framework / "variants" / board["build"]["variant"], sdkconfig]
    includes += sorted((framework / "libraries").glob("*/src"))
    for library in sorted((root / ".pio/libdeps/esp32_dev").iterdir()):
        if library.is_dir():
            includes += [library, library / "src"]
    flags = [str(compiler), "@" + str(sdk / "flags/cpp_flags"),
             "@" + str(sdk / "flags/defines"), "-iprefix", str(sdk / "include") + "/",
             "@" + str(sdk / "flags/includes"), "-std=gnu++17", "-DARDUINO=10819",
             "-DARDUINO_ARCH_ESP32", "-DESP32", "-DIOTSMARTSYS_SENSORS_ENABLED=1"]
    for include in includes:
        flags += ["-I", str(include)]
    output = root / ".pio/adc1-components" / env
    output.mkdir(parents=True, exist_ok=True)
    subprocess.run([str(compiler), "--version"], stdout=log, stderr=subprocess.STDOUT, check=True)
    failed = False
    for index, source in enumerate(SOURCES):
        command = flags + ["-c", str(root / "src" / source), "-o", str(output / f"{index}.o")]
        (output / f"{index}.command.json").write_text(json.dumps(command, indent=2) + "\n")
        result = subprocess.run(command, cwd=root, stdout=log, stderr=subprocess.STDOUT)
        log.write(f"{source}: exit {result.returncode}\n")
        log.flush()
        failed |= result.returncode != 0
    return 1 if failed else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("environments", nargs="*", help="Default: all ADC environments")
    parser.add_argument("--arduino3-source", type=Path, help="Compile Arduino 3 directly from existing sources")
    parser.add_argument("--arduino3-sdk", type=Path)
    parser.add_argument("--toolchains", type=Path, help="Directory containing existing cross compilers")
    parser.add_argument("--boards", type=Path, help="Directory containing PlatformIO board JSON files")
    args = parser.parse_args()
    if args.arduino3_source and not all((args.arduino3_sdk, args.toolchains, args.boards)):
        parser.error("Direct compilation requires --arduino3-sdk, --toolchains and --boards")
    for name in ("arduino3_source", "arduino3_sdk", "toolchains", "boards"):
        value = getattr(args, name)
        if value is not None:
            setattr(args, name, value.resolve())
    environments = args.environments or ENVIRONMENTS
    if any(env not in ENVIRONMENTS for env in environments):
        parser.error("Unknown ADC environment")
    root = Path(__file__).resolve().parents[1]
    logs = root / ".pio" / "adc1-components"
    logs.mkdir(parents=True, exist_ok=True)
    failed = False
    for env in environments:
        command = ["pio", "run", "-e", env]
        for source in SOURCES:
            command += ["-t", f".pio/build/{env}/src/{source}.o"]
        log_path = logs / f"{env}.log"
        print(f"Building {env}; log: {log_path}", flush=True)
        with log_path.open("w") as log:
            if args.arduino3_source and env.endswith("arduino3"):
                code = compile_arduino3(root, env, args, log)
            else:
                code = subprocess.run(command, cwd=root, stdout=log, stderr=subprocess.STDOUT).returncode
        print(f"{env}: exit {code}", flush=True)
        failed |= code != 0
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
