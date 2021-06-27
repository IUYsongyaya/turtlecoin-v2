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
#pragma once
#include <cstdint>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
// clang-format off
#include <windows.h>
#include <wincon.h>
// clang-format on
#endif

// usage:
// std::cout << COLOR::red     << " red "
//           << COLOR::yellow  << " yellow "
//           << COLOR::green   << " green "
//           << COLOR::cyan    << " cyan "
//           << COLOR::blue    << " blue "
//           << COLOR::magenta << " magenta "
//           << COLOR::grey    << " grey "
//           << COLOR::white   << " white "
//           << COLOR::reset   << " reset\n";
// std::cout << COLOR::red     << COLOR::on_cyan    << " red "
//           << COLOR::yellow  << COLOR::on_blue    << " yellow "
//           << COLOR::green   << COLOR::on_magenta << " green "
//           << COLOR::cyan    << COLOR::on_red     << " cyan "
//           << COLOR::blue    << COLOR::on_yellow  << " blue "
//           << COLOR::magenta << COLOR::on_green   << " magenta "
//           << COLOR::grey    << COLOR::on_white   << " grey "
//           << COLOR::white   << COLOR::on_grey    << " white "
//           << COLOR::reset                      << " reset\n";

#ifdef _WIN32
enum class COLOR : uint16_t
{
    grey,
    blue,
    green,
    cyan,
    red,
    magenta,
    yellow,
    white,
    on_blue,
    on_red,
    on_magenta,
    on_grey,
    on_green,
    on_cyan,
    on_yellow,
    on_white,
    reset = 0xFF
#elif __unix__
enum class COLOR : uint8_t
{
    grey = 30,
    red = 31,
    green = 32,
    yellow = 33,
    blue = 34,
    magenta = 35,
    cyan = 36,
    white = 37,
    on_grey = 40,
    on_red = 41,
    on_green = 42,
    on_yellow = 43,
    on_blue = 44,
    on_magenta = 45,
    on_cyan = 46,
    on_white = 47,
    reset
#else
#error unsupported
#endif
};

#ifdef _WIN32
namespace colored_cout_impl
{
    uint16_t getColorCode(const COLOR color);

    uint16_t getConsoleTextAttr();

    void setConsoleTextAttr(const uint16_t attr);
} // namespace colored_cout_impl
#endif

template<typename type> type &operator<<(type &ostream, const COLOR color)
{
#ifdef _WIN32
    // static const uint16_t initial_attributes = colored_cout_impl::getConsoleTextAttr();
    static const uint16_t initial_attributes = colored_cout_impl::getColorCode(COLOR::grey);

    static uint16_t background = initial_attributes & 0x00F0;

    static uint16_t foreground = initial_attributes & 0x000F;
#endif
    if (color == COLOR::reset)
    {
#ifdef _WIN32
        ostream.flush();

        colored_cout_impl::setConsoleTextAttr(initial_attributes);

        background = initial_attributes & 0x00F0;

        foreground = initial_attributes & 0x000F;

#elif __unix__
        ostream << "\033[m";
#endif
    }
    else
    {
#ifdef _WIN32
        uint16_t set = 0;

        const uint16_t colorCode = colored_cout_impl::getColorCode(color);

        if (colorCode & 0x00F0)
        {
            background = colorCode;

            set = background | foreground;
        }
        else if (colorCode & 0x000F)
        {
            foreground = colorCode;

            set = background | foreground;
        }

        ostream.flush();

        colored_cout_impl::setConsoleTextAttr(set);

#elif __unix__
        ostream << "\033[" << static_cast<uint32_t>(color) << "m";
#endif
    }

    return ostream;
}
