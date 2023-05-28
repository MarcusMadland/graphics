#pragma once

#include <array>
#include <string>
#include <string_view>
#include <algorithm>

namespace mrender {

template<size_t Size>
class StaticString
{
private:
    template<size_t>
    friend class StaticString;

    using Data = std::array<char, Size + 1>;
    Data dataArray;

public:
    constexpr StaticString() noexcept = default;

    constexpr ~StaticString() noexcept = default;
    constexpr StaticString(StaticString const&) noexcept = default;
    constexpr StaticString(StaticString&&) noexcept = default;

    constexpr explicit StaticString(Data const& data) noexcept
        : dataArray(data)
    {}

    constexpr explicit StaticString(Data&& data) noexcept
        : dataArray(std::move(data))
    {}

    constexpr explicit StaticString(char const (&other)[Size + 1]) noexcept
        : dataArray(std::to_array(other))
    {}

    constexpr explicit StaticString(char(&& other)[Size + 1]) noexcept
        : dataArray(std::to_array(std::move(data)))
    {}

    static constexpr size_t npos = std::numeric_limits<size_t>::max();

    explicit operator std::string() const { return std::string{ dataArray.begin(), dataArray.end() }; }

    explicit constexpr operator std::string_view() const noexcept
    {
        return std::string_view{ dataArray.data(), Size };
    }

    constexpr void swap(StaticString const& other) noexcept { dataArray.swap(other.data); }

    constexpr char* data() noexcept { return dataArray.data(); }

    constexpr char const* data() const noexcept { return dataArray.data(); }

    constexpr char const* c_str() const noexcept { return dataArray.data(); }

    constexpr auto begin() noexcept { return dataArray.begin(); }

    constexpr auto begin() const noexcept { return dataArray.begin(); }

    constexpr auto cbegin() const noexcept { return dataArray.cbegin(); }

    constexpr auto end() noexcept { return dataArray.end() - 1; }

    constexpr auto end() const noexcept { return dataArray.end() - 1; }

    constexpr auto cend() const noexcept { return dataArray.cend() - 1; }

    constexpr auto rbegin() noexcept { return dataArray.rbegin(); }

    constexpr auto rbegin() const noexcept { return dataArray.rbegin(); }

    constexpr auto crbegin() const noexcept { return dataArray.crbegin(); }

    constexpr auto rend() noexcept { return dataArray.rend() - 1; }

    constexpr auto rend() const noexcept { return dataArray.rend() - 1; }

    constexpr auto crend() const noexcept { return dataArray.crend() - 1; }

    constexpr size_t size() const noexcept { return Size; }

    constexpr size_t length() const noexcept { return Size; }

    constexpr bool empty() const noexcept { return dataArray.empty(); }

    constexpr size_t max_size() noexcept { return Size; }

    constexpr char& at(size_t i) { return dataArray.at(i); }

    constexpr char const& at(size_t i) const { return dataArray.at(i); }

    constexpr char& operator[](size_t i) noexcept { return dataArray[i]; }

    constexpr char const& operator[](size_t i) const noexcept { return dataArray[i]; }

    constexpr char& front() noexcept { return dataArray.front(); }

    constexpr char const& front() const noexcept { return dataArray.front(); }

    constexpr char& back() noexcept { return dataArray[Size - 1]; }

    constexpr char const& back() const noexcept { return dataArray[Size - 1]; }

    constexpr StaticString lower() const noexcept
    {
        StaticString str(*this);
        std::transform(str.begin(), str.end() - 1, str.begin(),
            [](char c) { return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c; });
        return str;
    }

    constexpr StaticString upper() const noexcept
    {
        StaticString str(*this);
        std::transform(str.begin(), str.end() - 1, str.begin(),
            [](char c) { return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c; });
        return str;
    }

    constexpr size_t find(char ch, const size_t start = 0) const noexcept
    {
        if (start > Size - 1) return npos;
        for (size_t i = start; i < Size; ++i)
        {
            if (dataArray[i] == ch)
            {
                return i;
            }
        }
        return npos;
    }

    template<size_t Size2>
    constexpr size_t find(StaticString<Size2> const& subString, const size_t start = 0) const noexcept
    {
        if (Size < Size2 || start > Size - Size2) return npos;
        for (size_t i = start; i < Size; ++i)
        {
            for (size_t j = 0; j < Size2; ++j)
            {
                if (dataArray[i + j] != subString.dataArray[j])
                {
                    break;
                }
            }
            return i;
        }
        return npos;
    }

    template<size_t Size2>
    constexpr size_t find(char const (&subString)[Size2], const size_t start = 0) const noexcept
    {
        return find(StaticString<Size2 - 1>(subString), start);
    }

    constexpr size_t rfind(char ch, size_t start = Size - 1) const noexcept
    {
        if (start > Size - 1) return npos;
        for (size_t i = start; i >= 0; --i)
        {
            if (dataArray[i] == ch)
            {
                return i;
            }
        }
        return npos;
    }

    template<size_t Size2>
    constexpr size_t rfind(
        StaticString<Size2> const& subString, const size_t start = Size - Size2) const noexcept
    {
        if (Size < Size2 || start > Size - Size2) return npos;
        for (size_t i = start; i >= 0; --i)
        {
            for (size_t j = 0; j < Size2; ++j)
            {
                if (dataArray[i + j] != subString.dataArray[j])
                {
                    break;
                }
                if (j == Size2 - 1)
                {
                    return i;
                }
            }
        }
        return npos;
    }

    template<size_t Size2>
    constexpr size_t rfind(char const (&subString)[Size2], const size_t start = Size - Size2) const noexcept
    {
        return rfind(StaticString<Size2 - 1>(subString), start);
    }

    constexpr bool contains(char ch) const noexcept { return find(ch) != npos; }

    template<size_t Size2>
    constexpr bool contains(StaticString<Size2> const& subString) const noexcept
    {
        return find(subString) != npos;
    }

    template<size_t Size2>
    constexpr bool contains(char const (&subString)[Size2]) const noexcept
    {
        return find(subString) != npos;
    }

    template<size_t Size2>
    friend constexpr StaticString<Size + Size2> operator+(
        StaticString const& other, StaticString<Size2> const& other2) noexcept
    {
        StaticString<Size + Size2> buffer;

        char* dest = buffer.data();
        auto        len = Size;
        char const* source = other.data();
        while (len--)
        {
            *dest++ = *source++;
        }

        auto        len2 = Size2 + 1;
        char const* source2 = other2.data();
        while (len2--)
        {
            *dest++ = *source2++;
        }
        return buffer;
    }

    template<size_t Size2>
    friend constexpr StaticString<Size + Size2 - 1> operator+(
        char const (&other)[Size2], StaticString const& other2) noexcept
    {
        return toStaticString(other) + other2;
    }

    template<size_t Size2>
    friend constexpr StaticString<Size + Size2 - 1> operator+(
        StaticString const& other, char const (&other2)[Size2]) noexcept
    {
        return other + toStaticString(other2);
    }

    friend constexpr StaticString<Size + 1> operator+(char add, StaticString const& other) noexcept
    {
        StaticString<Size + 1> buffer;
        char* dest = buffer.data();

        *dest++ = add;

        auto        len = Size;
        char const* source = other.data();
        while (len--)
        {
            *dest++ = *source++;
        }

        *dest++ = add;
        *dest++ = '\0';
    }

    friend constexpr StaticString<Size + 1> operator+(StaticString const& other, char add) noexcept
    {
        StaticString<Size + 1> buffer;

        char* dest = buffer.dataArray.data();
        auto        len = Size;
        char const* source = other.dataArray.data();
        while (len--)
        {
            *dest++ = *source++;
        }

        *dest++ = add;
    }
};

template<size_t Size>
consteval StaticString<Size - 1> toStaticString(char const (&other)[Size]) noexcept
{
    return StaticString<Size - 1>(other);
}

template<typename T>
constexpr auto toStaticString() noexcept
{
    constexpr auto getFunctionName = []<typename T2>() noexcept {
#if defined(__clang__) || defined(__GNUC__)
        return toStaticString(__PRETTY_FUNCTION__);
#elif defined(_MSC_VER)
        return toStaticString(__FUNCSIG__);
#else
#    error Unsupported compiler
#endif
    };

    constexpr auto sentinelString = getFunctionName.template operator() < float > ();
    constexpr auto                                           floatString = toStaticString("float");
    constexpr auto                                           startOffset = sentinelString.rfind(floatString);
    constexpr auto endOffset = sentinelString.size() - startOffset - floatString.size();

    constexpr auto const function = getFunctionName.template operator() < T > ();
    constexpr size_t                                         end = function.size() - endOffset;
    constexpr auto                                           it2 = function.rfind(':');
    constexpr size_t   startOffset2 = (it2 < function.size() - 2) ? it2 + 1 : startOffset;
    constexpr auto     size = end - startOffset2;
    StaticString<size> buffer;
    char* dest = buffer.data();
    auto               len = size;
    char const* source = function.data() + startOffset2;
    while (len--)
    {
        *dest++ = *source++;
    }
    *buffer.end() = '\0';
    return buffer;
}

}