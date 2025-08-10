#pragma region Main.TemplatedDispatch.cpp

#include "ReflectMeta/ReflectMeta.hpp"
#include "Foo.hpp"

using namespace ReflectMeta;

namespace ReflectMetaGenerated
{
    struct FooBinders
    {
        struct BinderSomeOtherMethod
        {
            template <typename T>
            static auto Call(::Foo& self, T* pointerValue, T& referenceValue, const T constValue) noexcept -> void
            {
                return self.template SomeOtherMethod<T>(pointerValue, referenceValue, constValue);
            }
        };

        struct BinderSomeOtherOtherMethod
        {
            template <typename T>
            static auto Call(::Foo& self, T* pointerValue, const T& referenceValue, T** pointerToPointer) noexcept -> T
            {
                return self.template SomeOtherOtherMethod<T>(pointerValue, referenceValue, pointerToPointer);
            }
        };
    };
}

namespace ReflectMeta
{
    template <>
    struct SupportedTypes<DefaultTemplateTag> { using List = TypeList<int, float, double>; };

    template <>
    struct SupportedTypes<ConceptTypesInternal::MyConcept_Tag> { using List = TypeList<int, float>; };
}

static constexpr auto kSomeOtherMethodErased =
&ReflectMeta::GenericTemplatedErasedCaller<
    ReflectMeta::DefaultTemplateTag, ::Foo, 
    ReflectMeta::Meta::RetVoid,
    ReflectMetaGenerated::FooBinders::BinderSomeOtherMethod,
    ReflectMeta::Meta::ParamPtr, ReflectMeta::Meta::ParamLRef, ReflectMeta::Meta::ParamCVal>;

static constexpr auto kSomeOtherOtherMethodErased =
&ReflectMeta::GenericTemplatedErasedCaller<
    ReflectMeta::ConceptTypesInternal::MyConcept_Tag, ::Foo,
    ReflectMeta::Meta::RetSelf,
    ReflectMetaGenerated::FooBinders::BinderSomeOtherOtherMethod,
    ReflectMeta::Meta::ParamPtr, ReflectMeta::Meta::ParamCLRef, ReflectMeta::Meta::ParamPtrPtr>;

inline static bool gWireTemplatedDispatch = []
    {
        Registry::Instance().AttachTemplatedDispatcher("::Foo", "SomeOtherMethod", kSomeOtherMethodErased, nullptr);
        Registry::Instance().AttachTemplatedDispatcher("::Foo", "SomeOtherOtherMethod", kSomeOtherOtherMethodErased, nullptr);

        return true;
    }();

#pragma endregion