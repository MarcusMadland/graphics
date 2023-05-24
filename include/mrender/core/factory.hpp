#pragma once

#include "static_string.hpp"

#include <memory>
#include <string_view>
#include <unordered_map>

namespace mrender {

template<typename Product, typename... Args>
class Factory
{
    friend Product;

public:
    template<typename... Args2>
    static std::unique_ptr<Product> make(std::string_view const &name, Args2 &&...args) noexcept
    {
        auto &list = GetList();
        if (auto i = list.find(name); i != list.cend()) return i->second(std::forward<Args2>(args)...);
        return nullptr;
    }

    static std::vector<std::string_view> getNames() noexcept
    {
        std::vector<std::string_view> ret;
        for (auto &i : GetList())
        {
            ret.push_back(i.first);
        }
        return ret;
    }

    template<class T>
    class Registrar : public Product
    {
        template<typename T2, typename = void>
        static constexpr bool isDefined = false;

        template<typename T2>
        static constexpr bool isDefined<T2, std::void_t<decltype(sizeof(T2::Name))>> = true;

    public:
        friend T;

        template<typename T2 = T>
        static constexpr std::string_view registeredName =
            (isDefined<T2>) ? T2::Name : static_cast<std::string_view>(toStaticString<T>());

        static bool registerType() noexcept
        {
            Factory::GetList()[registeredName<T>] = [](Args... args) noexcept -> std::unique_ptr<Product> {
                return std::make_unique<T>(std::forward<Args>(args)...);
            };
            return true;
        }

        static bool registered; 

    private:
        Registrar() noexcept
            : Product(Key {})
        {
            (void)registered;
        }
    };

    template<class T>
    class RegistrarName : public Product
    {
    public:
        friend T;

        static bool registerType() noexcept
        {
            auto name                = T::Name;
            Factory::GetList()[name] = [](Args... args) noexcept -> std::unique_ptr<Product> {
                return std::make_unique<T>(std::forward<Args>(args)...);
            };
            return true;
        }

        static bool registered; 

    private:
        RegistrarName() noexcept
            : Product(Key {}, T::Name)
        {
            (void)registered;
        }
    };

private:
    class Key
    {
        Key() noexcept {};
        template<class T>
        friend class Registrar;
        template<class T>
        friend class RegistrarName;
    };

    using FunctionType = std::unique_ptr<Product> (*)(Args...) noexcept;
    Factory() noexcept = default;

    static auto &GetList() noexcept
    {
        static std::unordered_map<std::string_view, FunctionType> list;
        return list;
    }
};

template<class Product, class... Args>
template<class T>
bool Factory<Product, Args...>::Registrar<T>::registered = Factory<Product, Args...>::Registrar<T>::registerType();

template<class Product, class... Args>
template<class T>
bool Factory<Product, Args...>::RegistrarName<T>::registered =
    Factory<Product, Args...>::RegistrarName<T>::registerType();

}   // namespace mrender
