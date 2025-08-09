#pragma once
#include <typeinfo>
#include <type_traits>
#include <utility>
#include "ReflectMeta/Core.hpp"

namespace ReflectMeta
{
    template <typename... Ts>
    struct TypeList final {};

    template <typename Tag>
    struct SupportedTypes;

    namespace Meta
    {
        template <typename U>
        struct RetVoid final { using Type = void; };

        template <typename U>
        struct RetSelf final { using Type = U; };

        template <typename U>
        struct ParamSelf final { using Type = U; };

        template <typename U>
        struct ParamPtr final { using Type = U*; };

        template <typename U>
        struct ParamLRef final { using Type = U&; };

        template <typename U>
        struct ParamCLRef final { using Type = const U&; };

        template <typename U>
        struct ParamCVal final { using Type = const U; };

        template <typename U>
        struct ParamPtrPtr final { using Type = U**; };
    }

    template <typename P>
    static inline auto Fetch(void** args, size_t index) noexcept -> decltype(auto)
    {
        using Elem = std::remove_reference_t<P>;
        return *reinterpret_cast<Elem*>(args[index]);
    }

    template <typename T, typename ClassT, template <typename> class RExpr, typename Binder, template <typename> class... AExprs, std::size_t... I>
    static inline auto CallBinderIndexed(void* self, void** args, void* retOut, std::index_sequence<I...>) noexcept -> void
    {
        using R = typename RExpr<T>::Type;
        ClassT& object = *reinterpret_cast<ClassT*>(self);
        if constexpr (!std::is_void_v<R>)
        {
            using Store = std::remove_cvref_t<R>;
            Store result = Binder::template Call<T>(object, Fetch<typename AExprs<T>::Type>(args, I)...);
            *reinterpret_cast<Store*>(retOut) = result;
        }
        else
            Binder::template Call<T>(object, Fetch<typename AExprs<T>::Type>(args, I)...);
    }

    template <typename List>
    struct TypeSwitch;

    template <typename... Ts>
    struct TypeSwitch<TypeList<Ts...>>
    {
        template <typename F>
        static inline auto Apply(const std::type_info& id, F&& f) noexcept -> bool
        {
            bool matched = false;
            ((id == typeid(Ts) ? (f.template operator() < Ts > (), matched = true) : false) || ...);
            return matched;
        }
    };

    namespace Detail
    {
        template <typename ClassT, template <typename> class RExpr, typename Binder, template <typename> class... AExprs>
        struct GenericInvoker final
        {
            void* self;
            void** args;
            void* retOut;

            template <typename T>
            inline auto operator()() const noexcept -> void
            {
                CallBinderIndexed<T, ClassT, RExpr, Binder, AExprs...>(self, args, retOut, std::make_index_sequence<sizeof...(AExprs)>{});
            }
        };
    }

    template <typename ConceptTag, typename ClassT, template <typename> class RExpr, typename Binder, template <typename> class... AExprs>
    static inline auto GenericTemplatedErasedCaller(const std::type_info* const* typeArgs, void* self, void** args, void* retOut) noexcept -> void
    {
        using List = typename SupportedTypes<ConceptTag>::List;
        const std::type_info& id = *typeArgs[0];
        Detail::GenericInvoker<ClassT, RExpr, Binder, AExprs...> inv{ self, args, retOut };
        (void)TypeSwitch<List>::Apply(id, inv);
    }

    struct DefaultTemplateTag final {};
}
