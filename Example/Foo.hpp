#pragma region Foo.hpp

#pragma once

#include <print>
#include <string>
#include <type_traits>
#include <concepts>
#include "ReflectMeta/ReflectMeta.hpp"

template <typename T>
concept MyConcept = std::same_as<T, ReflectMeta::TypenameClass> || std::is_arithmetic_v<T>;

namespace ReflectMeta
{
    REFLECT_META_ALIAS_CONCEPT_TYPE(ConceptTypesInternal, MyConcept)
}

template <typename T>
class MyBaseClass
{
public:

    T myTemplate;

    virtual const T& SomeCoolFunction()
    {
        return myTemplate;
    }

    virtual void SomeCoolerFunction(int*) = 0;
};

class MyOtherBaseClass
{

public:
    virtual void MyCoolerMethod() = 0;
};

class Foo final : public MyBaseClass<int>, public MyOtherBaseClass
{
public:
    int x;
    float y;

    void SomeMethod(float* pointerValue, std::string& text) const
    {
        std::println("Foo::SomeMethod");
    }

    template <typename T>
    void SomeOtherMethod(T* pointerValue, T& referenceValue, const T constValue) noexcept
    {
        std::println("Foo::SomeOtherMethod");
    }

    template <MyConcept T>
    T SomeOtherOtherMethod(T* pointerValue, const T& referenceValue, T** pointerToPointer) const noexcept
    {
        std::println("Foo::SomeOtherOtherMethod");
        return T();
    }

    const int& SomeCoolFunction() override
    {
        std::println("MyBaseClass::SomeCoolFunction");

        return x;
    }

    void SomeCoolerFunction(int*) override
    {
        std::println("MyBaseClass::SomeOtherOtherMethod");
    }

    void MyCoolerMethod() override
    {
        std::println("MyOtherBaseClass::SomeOtherOtherMethod");
    }

};

#pragma endregion