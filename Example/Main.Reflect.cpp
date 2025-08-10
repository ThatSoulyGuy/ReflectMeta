#pragma region Main.Reflect.cpp

#include "ReflectMeta/ReflectMeta.hpp"
#include "Foo.hpp"

namespace ReflectMeta
{
    template <>
    struct Reflect<::MyBaseClass<int>>
    {
        auto Get() const noexcept -> const TypeHierarchy&
        {
            auto& th = TypeHierarchy::New();

            th.Struct<::MyBaseClass<int>>("::MyBaseClass<int>")
				.Ctor<Access::PUBLIC, false, ::MyBaseClass<int>>()
                .Member<Access::PUBLIC, int>("::MyBaseClass<int>::myTemplate", offsetof(::MyBaseClass<int>, myTemplate))
                .Method<Access::PUBLIC, Qualifiers::NONE, const int&, ::MyBaseClass<int>>("::MyBaseClass<int>::SomeCoolFunction", &::MyBaseClass<int>::SomeCoolFunction, true)
                .MethodPureVirtual<Access::PUBLIC, Qualifiers::NONE, void, ::MyBaseClass<int>, int*>("::MyBaseClass<int>::SomeCoolerFunction")
                .Commit();

            return th;
        }
    };

    template <>
    struct Reflect<::MyOtherBaseClass>
    {
        auto Get() const noexcept -> const TypeHierarchy&
        {
            auto& th = TypeHierarchy::New();

            th.Struct<::MyOtherBaseClass>("::MyOtherBaseClass")
                .Ctor<Access::PUBLIC, false, ::MyOtherBaseClass>()
                .MethodPureVirtual<Access::PUBLIC, Qualifiers::NONE, void, ::MyOtherBaseClass>("::MyOtherBaseClass::MyCoolerMethod")
                .Commit();

            return th;
        }
    };

    template <>
    struct Reflect<::Foo>
    {
        auto Get() const noexcept -> const TypeHierarchy&
        {
            auto& th = TypeHierarchy::New();

            th.Struct<::Foo>("::Foo")
                .Ctor<Access::PUBLIC, false, ::Foo>()
                .Base<Access::PUBLIC, ::Foo, ::MyBaseClass<int>>("::MyBaseClass<int>", false)
                .Base<Access::PUBLIC, ::Foo, ::MyOtherBaseClass>("::MyOtherBaseClass", false)
                .Member<Access::PUBLIC, int>("::Foo::x", offsetof(::Foo, x))
                .Member<Access::PUBLIC, float>("::Foo::y", offsetof(::Foo, y))
                .Method<Access::PUBLIC, Qualifiers::CONST_, void, ::Foo, float*, std::string&>("::Foo::SomeMethod", &::Foo::SomeMethod)
                .Method<Access::PUBLIC, Qualifiers::NONE, const int&, ::Foo>("::Foo::SomeCoolFunction", &::Foo::SomeCoolFunction, true)
                .Method<Access::PUBLIC, Qualifiers::NONE, void, ::Foo, int*>("::Foo::SomeCoolerFunction", &::Foo::SomeCoolerFunction, true)
                .Method<Access::PUBLIC, Qualifiers::NONE, void, ::Foo>("::Foo::MyCoolerMethod", &::Foo::MyCoolerMethod, true)
                .MethodTemplated<TypenameType::DEFAULT, TypenameClass, Access::PUBLIC, Qualifiers::NOEXCEPT_, void, ::Foo, TypenameClass*, TypenameClass&, const TypenameClass>("::Foo::SomeOtherMethod")
                .MethodTemplated<TypenameType::CONCEPT, ConceptTypesInternal::MyConcept<TypenameClass>, Access::PUBLIC, Qualifiers::CONST_ | Qualifiers::NOEXCEPT_, ConceptTypesInternal::MyConcept<TypenameClass>, ::Foo, ConceptTypesInternal::MyConcept<TypenameClass>*, const ConceptTypesInternal::MyConcept<TypenameClass>&, ConceptTypesInternal::MyConcept<TypenameClass>**>("::Foo::SomeOtherOtherMethod")
                .Commit();

            return th;
        }
    };

    template <>
    struct Reflect_Impl<::MyBaseClass<int>>
    {
        inline static bool done = []
            {
                auto& th = Reflect<::MyBaseClass<int>>{}.Get();
                const TypeDesc* td = th.Get("::MyBaseClass<int>"); static TypeDesc single = *td;
                static bool once = (Registry::Instance().RegisterRange(&single, &single + 1), Registry::Instance().MapStdTypeIndex(std::type_index(typeid(::MyBaseClass<int>)), single.id), true); (void)once; return true;
            }();
    };

    template <> 
    struct Reflect_Impl<::MyOtherBaseClass>
    {
        inline static bool done = []
            {
                auto& th = Reflect<::MyOtherBaseClass>{}.Get();
                const TypeDesc* td = th.Get("::MyOtherBaseClass"); static TypeDesc single = *td;
                static bool once = (Registry::Instance().RegisterRange(&single, &single + 1), Registry::Instance().MapStdTypeIndex(std::type_index(typeid(::MyOtherBaseClass)), single.id), true); (void)once; return true;
            }();
    };

    template <>
    struct Reflect_Impl<::Foo>
    {
        inline static bool done = []
            {
                auto& th = Reflect<::Foo>{}.Get();
                const TypeDesc* td = th.Get("::Foo"); static TypeDesc single = *td;
                static bool once = (Registry::Instance().RegisterRange(&single, &single + 1), Registry::Instance().MapStdTypeIndex(std::type_index(typeid(::Foo)), single.id), true); (void)once; return true;
            }();
    };
}

#pragma endregion