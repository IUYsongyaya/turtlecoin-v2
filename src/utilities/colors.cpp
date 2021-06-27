/*********************************************************************************
 * "colored cout" is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to:
 * - http://unlicense.org/
 * - https://github.com/yurablok/colored-cout
 ********************************************************************************/

#include "colors.h"

#ifdef _WIN32

uint16_t colored_cout_impl::getColorCode(const COLOR color)
{
    switch (color)
    {
        case COLOR::grey:
            return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
        case COLOR::blue:
            return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        case COLOR::green:
            return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case COLOR::cyan:
            return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case COLOR::red:
            return FOREGROUND_RED | FOREGROUND_INTENSITY;
        case COLOR::magenta:
            return FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
        case COLOR::yellow:
            return FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
        case COLOR::white:
            return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
        case COLOR::on_blue:
            return BACKGROUND_BLUE; //| BACKGROUND_INTENSITY
        case COLOR::on_red:
            return BACKGROUND_RED; //| BACKGROUND_INTENSITY
        case COLOR::on_magenta:
            return BACKGROUND_BLUE | BACKGROUND_RED; //| BACKGROUND_INTENSITY
        case COLOR::on_grey:
            return BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
        case COLOR::on_green:
            return BACKGROUND_GREEN | BACKGROUND_INTENSITY;
        case COLOR::on_cyan:
            return BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
        case COLOR::on_yellow:
            return BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;
        case COLOR::on_white:
            return BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;
        case COLOR::reset:
        default:
            break;
    }
    return static_cast<uint16_t>(COLOR::reset);
}

uint16_t colored_cout_impl::getConsoleTextAttr()
{
    CONSOLE_SCREEN_BUFFER_INFO buffer_info;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buffer_info);

    return buffer_info.wAttributes;
}

void colored_cout_impl::setConsoleTextAttr(const uint16_t attr)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attr);
}

#endif // _WIN32
