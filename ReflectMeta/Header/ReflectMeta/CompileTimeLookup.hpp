#pragma once

#include <array>
#include "ReflectMeta/Core.hpp"

namespace ReflectMeta
{
    template <FixedString S>
    struct NameTag
    {
        static constexpr auto Value() noexcept -> std::string_view
        {
            return S.View();
        }
    };

    template <typename ClassT>
    class ClassType
    {

    public:

        explicit ClassType(const TypeDesc* desc) : desc(desc) { }

        auto GetDesc() const noexcept -> const TypeDesc*
        {
            return desc;
        }

        auto GetMember(bool accessibilityConsidered, std::string_view name) const -> MemberTypeErased
        {
            const FieldDesc* hit = nullptr;

            for (const FieldDesc& f : desc->fields)
            {
                if (MatchName(accessibilityConsidered, f.access, f.name, name))
                {
                    hit = &f;
                    break;
                }
            }

            assert(hit != nullptr && "member not found");

            return MemberTypeErased(hit->name, hit->type, hit->offsetInBytes);
        }

        auto GetMethod(bool accessibilityConsidered, std::string_view name) const -> MethodTypeErased
        {
            const MethodDesc* hit = nullptr;
            
            for (const MethodDesc& m : desc->methods)
            {
                if (MatchName(accessibilityConsidered, m.access, m.name, name))
                {
                    hit = &m;
                    break;
                }
            }

            assert(hit != nullptr && "method not found");
            
            return MethodTypeErased(hit->qualifiedName, reinterpret_cast<MethodTypeErased::Caller>(hit->erasedCaller));
        }

        auto GetMethodTemplated(bool accessibilityConsidered, std::string_view name) const -> MethodTypeTemplatedErased
        {
            const TemplatedMethodDesc* hit = nullptr;
            
            for (const TemplatedMethodDesc& m : desc->templatedMethods)
            {
                if (MatchName(accessibilityConsidered, m.access, m.name, name))
                {
                    hit = &m;
                    break;
                }
            }

            assert(hit != nullptr && "templated method not found");
            
            return MethodTypeTemplatedErased(hit->qualifiedName, reinterpret_cast<MethodTypeTemplatedErased::ErasedTemplatedCaller>(hit->erasedTemplatedCaller));
        }

        template <FixedString MemberName>
        auto GetMemberCT(bool accessibilityConsidered = true) const -> MemberTypeErased
        {
            const FieldDesc* hit = nullptr;

            for (const FieldDesc& f : desc->fields)
            {
                if (MatchName(accessibilityConsidered, f.access, f.name, NameTag<MemberName>::Value()))
                {
                    hit = &f;
                    break;
                }
            }

            assert(hit != nullptr && "member not found");

            return MakeTypedMember(*hit);
        }

        template <FixedString MethodName, typename R, typename... Args>
        auto GetMethodCT(bool accessibilityConsidered = true) const -> MethodTypeTyped<R, ClassT, Args...>
        {
            const MethodDesc* hit = nullptr; for (const MethodDesc& m : desc->methods)
            {
                if (MatchName(accessibilityConsidered, m.access, m.name, NameTag<MethodName>::Value()))
                {
                    hit = &m; break;
                }
            }

            assert(hit != nullptr && "method not found");

            using Caller = R(*)(ClassT&, Args&&...) noexcept;
            
            return MethodTypeTyped<R, ClassT, Args...>(hit->qualifiedName, reinterpret_cast<Caller>(hit->erasedCaller));
        }

        template <FixedString MethodName, typename Binder>
        auto GetMethodTemplatedCT(bool accessibilityConsidered = true) const -> MethodTypeTemplatedTyped<Binder, ClassT>
        {
            const TemplatedMethodDesc* hit = nullptr;
            
            for (const TemplatedMethodDesc& m : desc->templatedMethods)
            {
                if (MatchName(accessibilityConsidered, m.access, m.name, NameTag<MethodName>::Value()))
                {
                    hit = &m;
                    break;
                }
            }

            assert(hit != nullptr && "templated method not found");
            
            return MethodTypeTemplatedTyped<Binder, ClassT>(hit->qualifiedName);
        }

    private:

        static auto MatchName(bool accessibilityConsidered, Access memberAccess, std::string_view stored, std::string_view query) noexcept -> bool
        {
            if (accessibilityConsidered && memberAccess != Access::PUBLIC)
                return false;
            
            return stored == query || (stored.rfind("::") != std::string_view::npos && stored.substr(stored.rfind("::") + 2) == query);
        }

        static auto MakeTypedMember(const FieldDesc& f) -> MemberTypeErased
        {
            return MemberTypeErased(f.name, f.type, f.offsetInBytes);
        }

        const TypeDesc* desc;
    };

    class ClassTypeErased
    {

    public:

        explicit ClassTypeErased(const TypeDesc* desc) : desc(desc) {}

        auto GetDesc() const noexcept -> const TypeDesc* { return desc; }

        auto GetMember(bool accessibilityConsidered, std::string_view name) const -> MemberTypeErased
        {
            const FieldDesc* hit = nullptr;

            for (const FieldDesc& f : desc->fields)
            {
                if (MatchName(accessibilityConsidered, f.access, f.name, name))
                {
                    hit = &f;
                    break;
                }
            }

            assert(hit != nullptr && "member not found");

            return MemberTypeErased(hit->name, hit->type, hit->offsetInBytes);
        }

        auto GetMethod(bool accessibilityConsidered, std::string_view name) const -> MethodTypeErased
        {
            const MethodDesc* hit = nullptr;

            for (const MethodDesc& m : desc->methods)
            {
                if (MatchName(accessibilityConsidered, m.access, m.name, name))
                {
                    hit = &m; break;
                }
            }

            assert(hit != nullptr && "method not found");

            return MethodTypeErased(hit->qualifiedName, reinterpret_cast<MethodTypeErased::Caller>(hit->erasedCaller));
        }

        auto GetMethodTemplated(bool accessibilityConsidered, std::string_view name) const -> MethodTypeTemplatedErased
        {
            const TemplatedMethodDesc* hit = nullptr;

            for (const TemplatedMethodDesc& m : desc->templatedMethods)
            {
                if (MatchName(accessibilityConsidered, m.access, m.name, name))
                {
                    hit = &m;
                    break;
                }
            }

            assert(hit != nullptr && "templated method not found");

            return MethodTypeTemplatedErased(hit->qualifiedName, reinterpret_cast<MethodTypeTemplatedErased::ErasedTemplatedCaller>(hit->erasedTemplatedCaller));
        }

    private:

        static auto MatchName(bool accessibilityConsidered, Access memberAccess, std::string_view stored, std::string_view query) noexcept -> bool
        {
            if (accessibilityConsidered && memberAccess != Access::PUBLIC)
                return false;

            return stored == query || (stored.rfind("::") != std::string_view::npos && stored.substr(stored.rfind("::") + 2) == query);
        }

        const TypeDesc* desc;
    };

    class TypeHierarchy
    {

    public:

        static auto New() -> TypeHierarchy&
        {
            static TypeHierarchy th;
            
            th.Reset();
            return th;
        }

        template <typename ClassT>
        auto Struct(std::string_view qualifiedName) -> TypeHierarchy&
        {
            current.qualifiedName = qualifiedName;
            current.name = qualifiedName.substr(qualifiedName.rfind("::") == std::string_view::npos ? 0 : qualifiedName.rfind("::") + 2);
            current.isStruct = true;
            current.isClass = false;
            current.isUnion = false;
            current.isEnum = false;
            current.sizeInBytes = sizeof(ClassT);
            current.alignInBytes = alignof(ClassT);
            current.isPolymorphic = std::is_polymorphic_v<ClassT>;
            current.id = HashName(qualifiedName);
            
            return *this;
        }

        template <Access A, typename DerivedT, typename BaseT>
        auto Base(std::string_view baseQualifiedName, bool isVirtual) -> TypeHierarchy&
        {
            BaseDesc b;

            b.baseTypeId = HashName(baseQualifiedName);
            b.offsetInBytes = 0;
            b.isVirtual = isVirtual;
            b.access = A;
            b.adjustPtr = +[](void* p) noexcept -> void* { return static_cast<BaseT*>(static_cast<DerivedT*>(p)); };
            b.adjustConstPtr = +[](const void* p) noexcept -> const void* { return static_cast<const BaseT*>(static_cast<const DerivedT*>(p)); };
            
            current.bases.push_back(b);
            
            return *this;
        }
        
        template <Access A, typename MemberT>
        auto Member(std::string_view qualifiedMemberName, size_t offset) -> TypeHierarchy&
        {
            FieldDesc f{ qualifiedMemberName, QualOf<MemberT>(), offset, false, 0, A };
            
            current.fields.push_back(f);
            return *this;
        }
        
        template <Access A, Qualifiers Q, typename R, typename ClassT, typename... Args>
        auto Method(std::string_view qualifiedName, R(ClassT::* pmf)(Args...) noexcept, bool isVirtual = false, bool isStatic = false) -> TypeHierarchy&
        {
            MethodDesc m;
            
            m.name = qualifiedName.substr(qualifiedName.rfind("::") == std::string_view::npos ? 0 : qualifiedName.rfind("::") + 2);
            m.qualifiedName = qualifiedName;
            m.returnType = QualOf<R>();
            m.access = A;
            m.qualifiers = Q;
            m.parameters = ParamsOf<Args...>();
            m.isVirtual = isVirtual;
            m.isStatic = isStatic;
            m.isDeleted = false;
            m.isDefaulted = false;
            m.isPureVirtual = false;

            static R(ClassT:: * s_pmf)(Args...) noexcept = pmf;
            m.erasedCaller = reinterpret_cast<void*>(+[](void* self, void** args, void* retOut) noexcept -> void { CallPMF<R, ClassT, Args...>(s_pmf, self, args, retOut); });

            current.methods.push_back(std::move(m));
            
            return *this;
        }

        template <Access A, Qualifiers Q, typename R, typename ClassT, typename... Args>
        auto Method(std::string_view qualifiedName, R(ClassT::* pmf)(Args...), bool isVirtual = false, bool isStatic = false) -> TypeHierarchy&
        {
            MethodDesc m;
            
            m.name = qualifiedName.substr(qualifiedName.rfind("::") == std::string_view::npos ? 0 : qualifiedName.rfind("::") + 2);
            m.qualifiedName = qualifiedName;
            m.returnType = QualOf<R>();
            m.access = A;
            m.qualifiers = Q;
            m.parameters = ParamsOf<Args...>();
            m.isVirtual = isVirtual;
            m.isStatic = isStatic;
            m.isDeleted = false;
            m.isDefaulted = false;
            m.isPureVirtual = false;

            static R(ClassT:: * s_pmf)(Args...) = pmf;
            
            m.erasedCaller = reinterpret_cast<void*>(+[](void* self, void** args, void* retOut) noexcept -> void { CallPMF<R, ClassT, Args...>(s_pmf, self, args, retOut); });

            current.methods.push_back(std::move(m));
           
            return *this;
        }

        template <Access A, Qualifiers Q, typename R, typename ClassT, typename... Args>
        auto Method(std::string_view qualifiedName, R(ClassT::* pmf)(Args...) const, bool isVirtual = false, bool isStatic = false) -> TypeHierarchy&
        {
            MethodDesc m;
            
            m.name = qualifiedName.substr(qualifiedName.rfind("::") == std::string_view::npos ? 0 : qualifiedName.rfind("::") + 2);
            m.qualifiedName = qualifiedName;
            m.returnType = QualOf<R>();
            m.access = A;
            m.qualifiers = Q;
            m.parameters = ParamsOf<Args...>();
            m.isVirtual = isVirtual;
            m.isStatic = isStatic;
            m.isDeleted = false;
            m.isDefaulted = false;
            m.isPureVirtual = false;

            static R(ClassT:: * s_pmf)(Args...) const = pmf;
            
            m.erasedCaller = reinterpret_cast<void*>(+[](void* self, void** args, void* retOut) noexcept -> void { CallPMFConst<R, ClassT, Args...>(s_pmf, self, args, retOut); });

            current.methods.push_back(std::move(m));
            
            return *this;
        }

        template <Access A, Qualifiers Q, typename R, typename ClassT, typename... Args>
        auto MethodPureVirtual(std::string_view qualifiedName) -> TypeHierarchy&
        {
            MethodDesc m;

            m.name = qualifiedName.substr(qualifiedName.rfind("::") == std::string_view::npos ? 0 : qualifiedName.rfind("::") + 2);
            m.qualifiedName = qualifiedName;
            m.returnType = QualOf<R>();
            m.access = A;
            m.qualifiers = Q;
            m.parameters = ParamsOf<Args...>();
            m.isVirtual = true;
            m.isStatic = false;
            m.isDeleted = false;
            m.isDefaulted = false;
            m.isPureVirtual = true;
            m.erasedCaller = nullptr;

            current.methods.push_back(std::move(m));

            return *this;
        }

        template <TypenameType TT, typename TemplateTypeExpr, Access A, Qualifiers Q, typename R, typename ClassT, typename... FormalArgs>
        auto MethodTemplated(std::string_view qualifiedName, MethodTypeTemplatedErased::ErasedTemplatedCaller caller = nullptr) -> TypeHierarchy&
        {
            TemplatedMethodDesc t;

            t.name = qualifiedName.substr(qualifiedName.rfind("::") == std::string_view::npos ? 0 : qualifiedName.rfind("::") + 2);
            t.qualifiedName = qualifiedName;
            t.access = A;
            t.qualifiers = Q;
            t.templateDisplay = "<typename T>";
            t.erasedTemplatedCaller = reinterpret_cast<void*>(caller);
            t.binderTag = nullptr;

            current.templatedMethods.push_back(std::move(t));
            return *this;
        }

        auto Commit() -> const TypeDesc&
        {
            types.push_back(current);
            
            return types.back();
        }

        auto Reset() -> void
        {
            current = TypeDesc{};
        }

        auto Get(std::string_view qualifiedName) const -> const TypeDesc*
        {
            for (const TypeDesc& t : types)
            {
                if (t.qualifiedName == qualifiedName)
                    return &t;
            }

            return nullptr;
        }

        auto All() const -> const std::vector<TypeDesc>&
        {
            return types;
        }

    private:

        template <typename T>
        static auto QualOf() -> QualTypeInfo
        {
            if constexpr (std::is_same_v<T, void>)
                return QualTypeInfo{ TypeName<T>(), TypeName<T>(), 0, 0, std::is_const_v<T>, std::is_volatile_v<T>, std::is_reference_v<T>, std::is_pointer_v<T> };
            else
                return QualTypeInfo{ TypeName<T>(), TypeName<T>(), sizeof(T), alignof(T), std::is_const_v<T>, std::is_volatile_v<T>, std::is_reference_v<T>, std::is_pointer_v<T> };
        }

        template <typename T>
        static auto TypeName() -> std::string_view
        {
            return typeid(T).name();
        }

        template <typename... A>
        static auto ParamsOf() -> std::vector<MethodParam>
        {
            std::vector<MethodParam> v;
            
            (v.push_back(MethodParam{ std::string_view(), QualOf<A>() }), ...);
            
            return v; 
        }

        template <typename R, typename C, typename... A, std::size_t... I>
        static auto CallPMFIndexed(R(C::* pmf)(A...) noexcept, void* self, void** args, void* retOut, std::index_sequence<I...>) noexcept -> void
        {
            if constexpr (!std::is_void_v<R>)
            {
                using Store = std::remove_cvref_t<R>;
                *reinterpret_cast<Store*>(retOut) = (reinterpret_cast<C*>(self)->*pmf)(*reinterpret_cast<std::remove_reference_t<std::tuple_element_t<I, std::tuple<A...>>>*>(args[I])...);
            }
            else
                (reinterpret_cast<C*>(self)->*pmf)(*reinterpret_cast<std::remove_reference_t<std::tuple_element_t<I, std::tuple<A...>>>*>(args[I])...);
        }

        template <typename R, typename C, typename... A>
        static auto CallPMF(R(C::* pmf)(A...) noexcept, void* self, void** args, void* retOut) noexcept -> void
        {
            CallPMFIndexed<R, C, A...>(pmf, self, args, retOut, std::index_sequence_for<A...>{});
        }

        template <typename R, typename C, typename... A, std::size_t... I>
        static auto CallPMFConstIndexed(R(C::* pmf)(A...) const noexcept, void* self, void** args, void* retOut, std::index_sequence<I...>) noexcept -> void
        {
            if constexpr (!std::is_void_v<R>)
            {
                using Store = std::remove_cvref_t<R>;
                *reinterpret_cast<Store*>(retOut) = (reinterpret_cast<const C*>(self)->*pmf)(*reinterpret_cast<std::remove_reference_t<std::tuple_element_t<I, std::tuple<A...>>>*>(args[I])...);
            }
            else
                (reinterpret_cast<const C*>(self)->*pmf)(*reinterpret_cast<std::remove_reference_t<std::tuple_element_t<I, std::tuple<A...>>>*>(args[I])...);
        }

        template <typename R, typename C, typename... A>
        static auto CallPMFConst(R(C::* pmf)(A...) const noexcept, void* self, void** args, void* retOut) noexcept -> void
        {
            CallPMFConstIndexed<R, C, A...>(pmf, self, args, retOut, std::index_sequence_for<A...>{});
        }

        template <typename R, typename C, typename... A, std::size_t... I>
        static auto CallPMFIndexed(R(C::* pmf)(A...), void* self, void** args, void* retOut, std::index_sequence<I...>) noexcept -> void
        {
            if constexpr (!std::is_void_v<R>)
            {
                using Store = std::remove_cvref_t<R>;
                *reinterpret_cast<Store*>(retOut) = (reinterpret_cast<C*>(self)->*pmf)(*reinterpret_cast<std::remove_reference_t<std::tuple_element_t<I, std::tuple<A...>>>*>(args[I])...);
            }
            else
                (reinterpret_cast<C*>(self)->*pmf)(*reinterpret_cast<std::remove_reference_t<std::tuple_element_t<I, std::tuple<A...>>>*>(args[I])...);
        }

        template <typename R, typename C, typename... A>
        static auto CallPMF(R(C::* pmf)(A...), void* self, void** args, void* retOut) noexcept -> void
        {
            CallPMFIndexed<R, C, A...>(pmf, self, args, retOut, std::index_sequence_for<A...>{});
        }

        template <typename R, typename C, typename... A, std::size_t... I>
        static auto CallPMFConstIndexed(R(C::* pmf)(A...) const, void* self, void** args, void* retOut, std::index_sequence<I...>) noexcept -> void
        {
            if constexpr (!std::is_void_v<R>)
            {
                using Store = std::remove_cvref_t<R>;
                *reinterpret_cast<Store*>(retOut) = (reinterpret_cast<const C*>(self)->*pmf)(*reinterpret_cast<std::remove_reference_t<std::tuple_element_t<I, std::tuple<A...>>>*>(args[I])...);
            }
            else
                (reinterpret_cast<const C*>(self)->*pmf)(*reinterpret_cast<std::remove_reference_t<std::tuple_element_t<I, std::tuple<A...>>>*>(args[I])...);
        }

        template <typename R, typename C, typename... A>
        static auto CallPMFConst(R(C::* pmf)(A...) const, void* self, void** args, void* retOut) noexcept -> void
        {
            CallPMFConstIndexed<R, C, A...>(pmf, self, args, retOut, std::index_sequence_for<A...>{});
        }

        static auto HashName(std::string_view qn) -> TypeId
        {
            uint64_t h1 = 1469598103934665603ull;
            uint64_t h2 = 1099511628211ull;
            
            for (char c : qn)
            {
                h1 ^= static_cast<uint8_t>(c);
                h1 *= h2;
            }
            
            return TypeId{ h1, ~h1 };
        }

        TypeDesc current;
        std::vector<TypeDesc> types;
    };

}