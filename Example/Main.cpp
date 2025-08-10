#pragma region Main.cpp

#include <iostream>
#include "Foo.hpp"

using namespace ReflectMeta;

static auto Expect(bool condition, const char* message) -> void
{
    if (!condition)
    {
        std::println("TEST FAILURE: {}", message);
        std::abort();
    }
}

static auto FindMethod(const TypeDesc* typeDesc, std::string_view simpleName) -> const MethodDesc*
{
    for (const MethodDesc& m : typeDesc->methods)
    {
        if (m.name == simpleName)
            return &m;
    }

    return nullptr;
}

static auto FindBaseByName(const TypeDesc* typeDesc, std::string_view baseQualifiedName) -> const BaseDesc*
{
    for (const BaseDesc& b : typeDesc->bases)
    {
        const TypeDesc* bd = Registry::Instance().Find(b.baseTypeId);

        if (bd != nullptr && bd->qualifiedName == baseQualifiedName)
            return &b;
    }

    return nullptr;
}

int main()
{
    std::println("== ReflectMeta comprehensive test ==");

    const TypeDesc* fooDesc = Registry::Instance().Get("::Foo");
    const TypeDesc* baseTDesc = Registry::Instance().Get("::MyBaseClass<int>");
    const TypeDesc* ifaceDesc = Registry::Instance().Get("::MyOtherBaseClass");

    Expect(fooDesc != nullptr, "::Foo not registered");
    Expect(baseTDesc != nullptr, "::MyBaseClass<int> not registered");
    Expect(ifaceDesc != nullptr, "::MyOtherBaseClass not registered");

    Expect(fooDesc->isPolymorphic, "::Foo should be polymorphic");
    Expect(fooDesc->bases.size() == 2, "::Foo should have exactly two direct bases");

    const BaseDesc* base1 = FindBaseByName(fooDesc, "::MyBaseClass<int>");
    const BaseDesc* base2 = FindBaseByName(fooDesc, "::MyOtherBaseClass");
    Expect(base1 != nullptr, "Missing base ::MyBaseClass<int>");
    Expect(base2 != nullptr, "Missing base ::MyOtherBaseClass");

    {
        const MethodDesc* pv = FindMethod(baseTDesc, "SomeCoolerFunction");

        Expect(pv != nullptr, "MyBaseClass<int>::SomeCoolerFunction not found");
        Expect(pv->isVirtual, "SomeCoolerFunction should be virtual");
        Expect(pv->isPureVirtual, "SomeCoolerFunction should be pure virtual");
        Expect(pv->erasedCaller == nullptr, "Pure virtual should not have an erased caller");
    }

    {
        const MethodDesc* ifacePure = FindMethod(ifaceDesc, "MyCoolerMethod");

        Expect(ifacePure != nullptr, "MyOtherBaseClass::MyCoolerMethod not found");
        Expect(ifacePure->isVirtual && ifacePure->isPureVirtual, "MyCoolerMethod should be pure virtual");
    }

    Foo foo{};
    ClassTypeErased fooClass{ fooDesc };

    auto xErased = fooClass.GetMember(true, "x");
    int assignedValue = 5;
    xErased.AssignAny(&foo, &assignedValue);

    int readBack = 0;
    xErased.GetAny(&foo, &readBack);
    Expect(readBack == 5, "::Foo::x assign/get mismatch (erased)");

    auto xTypedMaybe = xErased.AsTyped<int>();
    Expect(!!xTypedMaybe, "AsTyped<int> failed for ::Foo::x");
    auto xTyped = *xTypedMaybe;
    xTyped.Assign(42, &foo);
    Expect(xTyped.Get(&foo) == 42, "::Foo::x typed assign/get mismatch");

    auto someMethod = fooClass.GetMethod(true, "SomeMethod");
    float floatArgument = 37.0f;
    std::string textArgument = "hi";
    void* someMethodArgs[2] = { &floatArgument, &textArgument };
    someMethod.Invoke(&foo, someMethodArgs, nullptr);

    Expect(textArgument == "hi", "SomeMethod should not modify std::string in example body");

    const TypeDesc* fooDesc1 = Registry::Instance().Get("::Foo");
    ClassTypeErased fooType1{ fooDesc1 };
    
    void* foo1 = Registry::Instance().New("::Foo", nullptr, 0, nullptr);
    
    Expect(foo1 != nullptr, "Failed to create instance of ::Foo");
    
    Registry::Instance().Delete("::Foo", foo1);

    {
        auto m = fooClass.GetMethod(true, "SomeCoolerFunction");
        int localValue = 123;
        void* args[] = { &localValue };
        m.Invoke(&foo, args, nullptr);
    }
    {
        auto m = fooClass.GetMethod(true, "MyCoolerMethod");
        void* args = {};
        m.Invoke(&foo, &args, nullptr);
    }

    {
        const std::type_info* targsInt[] = { &typeid(int) };
        int r = 0;
        int* pointerIn = &r;  
        int constValue = 7;
        void* args[] = { &pointerIn, &r, &constValue };
        fooClass.GetMethodTemplated(false, "SomeOtherMethod").InvokeWithType(targsInt, &foo, args, nullptr);
    }
    {
        const std::type_info* targsFloat[] = { &typeid(float) };
        float rf = 0.0f;
        float* pointerInF = &rf;
        float constValueF = 3.5f;
        void* args[] = { &pointerInF, &rf, &constValueF };
        fooClass.GetMethodTemplated(false, "SomeOtherMethod").InvokeWithType(targsFloat, &foo, args, nullptr);
    }
    {
        const std::type_info* targsInt[] = { &typeid(int) };
        int r = 9;
        int* pointerIn = &r;
        int** pointerPointer = nullptr;
        int returnValue = -1;
        void* args[] = { &pointerIn, &r, &pointerPointer };

        fooClass.GetMethodTemplated(false, "SomeOtherOtherMethod").InvokeWithType(targsInt, &foo, args, &returnValue);

        Expect(returnValue == 0, "SomeOtherOtherMethod<int> return mismatch");
    }
    {
        const std::type_info* targsFloat[] = { &typeid(float) };
        float r = 1.0f;
        float* pointerIn = &r;
        float** pointerPointer = nullptr;
        float returnValue = -1.0f;
        void* args[] = { &pointerIn, &r, &pointerPointer };

        fooClass.GetMethodTemplated(false, "SomeOtherOtherMethod").InvokeWithType(targsFloat, &foo, args, &returnValue);

        Expect(returnValue == 0.0f, "SomeOtherOtherMethod<float> return mismatch");
    }

    {
        ::MyBaseClass<int>* basePtr = static_cast<::MyBaseClass<int>*>(&foo);
        ClassTypeErased baseClass{ baseTDesc };

        auto myTemplateErased = baseClass.GetMember(true, "myTemplate");
        int baseAssigned = 1234;
        myTemplateErased.AssignAny(basePtr, &baseAssigned);
        int baseRead = 0;
        myTemplateErased.GetAny(basePtr, &baseRead);

        Expect(baseRead == 1234, "MyBaseClass<int>::myTemplate assign/get mismatch");
    }

    {
        const std::type_info* targsBad[] = { &typeid(std::string) };

        std::string r = "nope";
        std::string* pointerIn = &r;
        std::string** pointerPointer = nullptr;
        std::string returnValue = "untouched";
        void* args[] = { &pointerIn, &r, &pointerPointer };

        fooClass.GetMethodTemplated(false, "SomeOtherOtherMethod").InvokeWithType(targsBad, &foo, args, &returnValue);

        Expect(returnValue == "untouched", "Concept gating failed: std::string call should be ignored");
    }

    std::println("All tests passed.");

    return 0;
}

#pragma endregion