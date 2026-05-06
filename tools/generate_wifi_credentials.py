from pathlib import Path
from configparser import ConfigParser

Import("env")


PROJECT_DIR = Path(env.subst("$PROJECT_DIR"))
OUTPUT = PROJECT_DIR / "src" / "Config" / "WifiCredentials.generated.h"
CANDIDATES = [
    PROJECT_DIR / "wifi_credentials.ini",
    PROJECT_DIR / "configs" / "wifi_credentials.ini",
]


def escape_cpp(value: str) -> str:
    return (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
        .replace("\r", "\\r")
    )


def load_credentials():
    source = next((path for path in CANDIDATES if path.exists()), None)
    if source is None:
        return []

    parser = ConfigParser()
    parser.read(source)

    credentials = []
    for section in parser.sections():
        ssid = parser.get(section, "ssid", fallback="").strip()
        password = parser.get(section, "password", fallback="")
        if ssid and password:
            credentials.append((ssid, password))

    return credentials


def render(credentials):
    lines = [
        "#pragma once",
        "",
        "namespace iotsmartsys::config",
        "{",
    ]

    if credentials:
        lines.append("    static constexpr WifiCredential kWifiCredentials[] = {")
        for ssid, password in credentials:
            lines.append(
                f'        {{"{escape_cpp(ssid)}", "{escape_cpp(password)}"}},'
            )
        lines.append("    };")
        lines.append(
            "    static constexpr std::size_t kWifiCredentialCount = "
            "sizeof(kWifiCredentials) / sizeof(kWifiCredentials[0]);"
        )
    else:
        lines.append(
            "    static constexpr WifiCredential kWifiCredentials[1] = {{nullptr, nullptr}};"
        )
        lines.append("    static constexpr std::size_t kWifiCredentialCount = 0;")

    lines.append("}")
    lines.append("")
    return "\n".join(lines)


credentials = load_credentials()
OUTPUT.parent.mkdir(parents=True, exist_ok=True)
OUTPUT.write_text(render(credentials), encoding="utf-8")
