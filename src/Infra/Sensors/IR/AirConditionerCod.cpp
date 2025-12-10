#include "AirConditionerCod.h"

void setACMode(String mode, uint8_t temp)
{
    uint64_t code = 0;

    if (mode == "off")
        code = 0xC02000006E56;
    else if (mode == "cool")
    {
        switch (temp)
        {
        case 16:
            code = 0x2000006C56;
            break;
        case 17:
            code = 0x2000006D56;
            break;
        case 18:
            code = 0x2000006E56;
            break;
        case 19:
            code = 0x2000006F56;
            break;
        case 20:
            code = 0x2000007056;
            break;
        case 21:
            code = 0x2000007156;
            break;
        case 22:
            code = 0x2000007256;
            break;
        case 23:
            code = 0x2000007356;
            break;
        case 24:
            code = 0x2000007456;
            break;
        }
    }
    else if (mode == "dry")
        code = 0x3300007356;
    else if (mode == "fan")
        code = 0x5200007456;
    else if (mode == "heat")
        code = 0x1000007556;
    else if (mode == "auto")
        code = 0x4000007356;
}

String getACMode(uint64_t code)
{
    switch (code)
    {
    case 0xC02000006E56:
    case 0xC02000007256:
    case 0xC02200006C56:
        return "off";
    case 0x2000006C56:
    case 0x2200006C56:
        return "16";
    case 0x2000006D56:
    case 0x2200006D56:
        return "17";
    case 0x2000006E56:
    case 0x2200006E56:
        return "18";
    case 0x2000006F56:
    case 0x2200006F56:
        return "19";
    case 0x2000007056:
    case 0x2200007056:
        return "20";
    case 0x2000007156:
    case 0x2200007156:
        return "21";
    case 0x2000007256:
    case 0x2200007256:
        return "22";
    case 0x2000007356:
    case 0x2200007356:
    case 0x14DEAC5A:
        return "23";
    case 0x2000007456:
    case 0x2200007456:
        return "24";
    case 0x3300007356:
        return "dry";
    case 0x5200007456:
        return "fan";
    case 0x1000007556:
        return "heat";
    case 0x4000007356:
        return "auto";
    default:
        return "unknown";
    }
}
