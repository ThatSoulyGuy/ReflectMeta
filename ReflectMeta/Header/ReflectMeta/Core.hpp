#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string_view>
#include <string>
#include <vector>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <optional>
#include <utility>
#include <tuple>
#include <cassert>
#include <new>

#if defined(_MSC_VER)
#define REFLECT_META_USED
#else
#define REFLECT_META_USED [[gnu::used]]
#endif

#define REFLECT_META_ALIAS_CONCEPT_TYPE(NS, ConceptName) namespace NS { struct ConceptName##_Tag final { }; template <typename T> using ConceptName = ::ReflectMeta::ConceptType<ConceptName##_Tag, T>; }

namespace ReflectMeta
{
    template <typename T>
    struct Reflect;

    template <typename T>
    struct Reflect_Impl;

    struct TypenameClass final {};

    enum class Access : uint8_t
    {
        PUBLIC,
        PROTECTED,
        PRIVATE
    };

    enum class Qualifiers : uint32_t
    {
        NONE = 0,
        CONST_ = 1 << 0,
        VOLATILE_ = 1 << 1,
        NOEXCEPT_ = 1 << 2,
        LVALUE_ = 1 << 3,
        RVALUE_ = 1 << 4,
        STATIC_ = 1 << 5
    };

    constexpr auto operator|(Qualifiers a, Qualifiers b) noexcept -> Qualifiers
    {
        return static_cast<Qualifiers>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    constexpr auto HasQualifier(Qualifiers v, Qualifiers f) noexcept -> bool
    {
        return (static_cast<uint32_t>(v) & static_cast<uint32_t>(f)) != 0u;
    }

    enum class TypenameType : uint8_t { DEFAULT, CONCEPT, CUSTOM };

    struct FixedString
    {
        const char* data;
        size_t size;

        template <size_t N>
        consteval FixedString(const char(&arr)[N]) : data(arr), size(N - 1) { }

        constexpr auto View() const noexcept -> std::string_view
        {
            return std::string_view(data, size);
        }
    };

    struct TypeId
    {
        uint64_t hi;
        uint64_t lo;

        auto operator==(const TypeId& rhs) const -> bool { return hi == rhs.hi && lo == rhs.lo; }

        struct Hash
        {
            auto operator()(const TypeId& k) const -> size_t
            {
                return static_cast<size_t>(k.hi ^ (k.lo << 1));
            }
        };
    };

    struct QualTypeInfo
    {
        std::string_view displayName;
        std::string_view qualifiedName;

        size_t sizeInBytes;
        size_t alignInBytes;

        bool isConst;
        bool isVolatile;
        bool isReference;
        bool isPointer;
    };

    struct FieldDesc
    {
        std::string_view name;
        QualTypeInfo type;
        size_t offsetInBytes;

        bool isBitField;

        uint32_t bitWidth;
        Access access;
    };

    struct MethodParam
    {
        std::string_view name;
        QualTypeInfo type;
    };

    struct MethodDesc
    {
        std::string_view name;
        std::string_view qualifiedName;

        QualTypeInfo returnType;

        std::vector<MethodParam> parameters;

        Access access;
        Qualifiers qualifiers;

        bool isVirtual;
        bool isStatic;
        bool isDeleted;
        bool isDefaulted;
        void* erasedCaller;

        bool isPureVirtual;
    };

    struct TemplatedMethodDesc
    {
        std::string_view name;
        std::string_view qualifiedName;

        Access access;
        Qualifiers qualifiers;

        std::string_view templateDisplay;

        void* erasedTemplatedCaller;
        void* binderTag;
    };

    struct BaseDesc
    {
        using AdjustPtr = void* (*)(void*) noexcept;
        using AdjustConstPtr = const void* (*)(const void*) noexcept;

        TypeId baseTypeId;
        size_t offsetInBytes;

        bool isVirtual;

        Access access;

        AdjustPtr adjustPtr;
        AdjustConstPtr adjustConstPtr;
    };

    struct CtorDesc
    {
        std::string_view qualifiedName;
        std::vector<MethodParam> parameters;
        Access access;
        bool isExplicit;

        using Erased = void (*)(void* storage, void** args) noexcept;
        Erased erasedCtor;
    };

    struct DtorDesc
    {
        std::string_view qualifiedName;
        Access access;
        bool isVirtual;
        bool isNoexcept;

        using Erased = void (*)(void* obj) noexcept;
        Erased erasedDtor;
    };

    struct TypeDesc
    {
        TypeId id;

        std::string_view name;
        std::string_view qualifiedName;

        size_t sizeInBytes;
        size_t alignInBytes;

        bool isClass;
        bool isStruct;
        bool isUnion;
        bool isEnum;
        bool isPolymorphic;

        std::vector<FieldDesc> fields;
        std::vector<MethodDesc> methods;
        std::vector<TemplatedMethodDesc> templatedMethods;
        std::vector<BaseDesc> bases;

        std::vector<CtorDesc> constructors;
        std::optional<DtorDesc> destructor;
    };

    template <typename MemberT>
    class MemberTypeTyped
    {

    public:

        using ValueType = MemberT;

        MemberTypeTyped(std::string_view qualifiedMemberName, size_t offsetInBytes) : qualifiedMemberName(qualifiedMemberName), offsetInBytes(offsetInBytes) {}

        auto GetQualifiedName() const noexcept -> std::string_view
        {
            return qualifiedMemberName;
        }

        auto Get(void* object) const noexcept -> MemberT&
        {
            return *reinterpret_cast<MemberT*>(reinterpret_cast<char*>(object) + offsetInBytes);
        }

        auto Get(const void* object) const noexcept -> const MemberT&
        {
            return *reinterpret_cast<const MemberT*>(reinterpret_cast<const char*>(object) + offsetInBytes);
        }

        auto Assign(const MemberT& value, void* object) const noexcept -> void
        {
            *reinterpret_cast<MemberT*>(reinterpret_cast<char*>(object) + offsetInBytes) = value;
        }

    private:

        std::string_view qualifiedMemberName;
        size_t offsetInBytes;
    };

    class MemberTypeErased
    {

    public:

        MemberTypeErased(std::string_view qualifiedMemberName, QualTypeInfo typeInfo, size_t offsetInBytes) : qualifiedMemberName(qualifiedMemberName), typeInfo(typeInfo), offsetInBytes(offsetInBytes) {}

        auto GetQualifiedName() const noexcept -> std::string_view
        {
            return qualifiedMemberName;
        }

        auto GetType() const noexcept -> const QualTypeInfo&
        {
            return typeInfo;
        }

        template <typename T>
        auto AsTyped() const -> std::optional<MemberTypeTyped<T>>
        {
            if (typeInfo.sizeInBytes != sizeof(T))
                return std::nullopt;

            return MemberTypeTyped<T>(qualifiedMemberName, offsetInBytes);
        }

        auto GetAny(void* object, void* outValueBuffer) const noexcept -> void
        {
            std::memcpy(outValueBuffer, reinterpret_cast<char*>(object) + offsetInBytes, typeInfo.sizeInBytes);
        }

        auto AssignAny(void* object, const void* inValueBuffer) const noexcept -> void
        {
            std::memcpy(reinterpret_cast<char*>(object) + offsetInBytes, inValueBuffer, typeInfo.sizeInBytes);
        }

    private:

        std::string_view qualifiedMemberName;
        QualTypeInfo typeInfo;
        size_t offsetInBytes;
    };


    template <typename R, typename ClassT, typename... Args>
    class MethodTypeTyped
    {

    public:

        using Caller = R(*)(ClassT&, Args&&...) noexcept;

        MethodTypeTyped(std::string_view qualifiedName, Caller caller) : qualifiedName(qualifiedName), caller(caller) { }

        auto Invoke(ClassT& self, Args... args) const noexcept -> R
        {
            return caller(self, static_cast<Args&&>(args)...);
        }

        auto GetQualifiedName() const noexcept -> std::string_view
        {
            return qualifiedName;
        }

    private:

        std::string_view qualifiedName;
        Caller caller;
    };

    class MethodTypeErased
    {

    public:

        using Caller = void (*)(void*, void** args, void* retOut) noexcept;

        MethodTypeErased(std::string_view qualifiedName, Caller caller) : qualifiedName(qualifiedName), caller(caller) { }

        auto Invoke(void* self, void** args, void* retOut) const noexcept -> void 
        {
            caller(self, args, retOut);
        }

        auto GetQualifiedName() const noexcept -> std::string_view
        {
            return qualifiedName;
        }
    
    private:
    
        std::string_view qualifiedName;
        Caller caller;
    };

    template <typename Binder, typename ClassT>
    class MethodTypeTemplatedTyped
    {

    public:

        MethodTypeTemplatedTyped(std::string_view qualifiedName) : qualifiedName(qualifiedName) { }
        
        template <typename T, typename... A>
        auto Invoke(ClassT& self, A&&... a) const noexcept -> decltype(auto)
        {
            return Binder::template Call<T>(self, static_cast<A&&>(a)...);
        }
        
        auto GetQualifiedName() const noexcept -> std::string_view
        {
            return qualifiedName;
        }
    
    private:
        
        std::string_view qualifiedName;
    };

    class MethodTypeTemplatedErased
    {

    public:

        using ErasedTemplatedCaller = void (*)(const std::type_info* const*, void*, void**, void*) noexcept;

        MethodTypeTemplatedErased(std::string_view qualifiedName, ErasedTemplatedCaller caller) : qualifiedName(qualifiedName), caller(caller) {}

        auto InvokeWithType(const std::type_info* const* typeArgs, void* self, void** args, void* retOut) const noexcept -> void
        {
            caller(typeArgs, self, args, retOut);
        }

        auto GetQualifiedName() const noexcept -> std::string_view
        {
            return qualifiedName;
        }

    private:

        std::string_view qualifiedName;
        mutable ErasedTemplatedCaller caller;
    };

    class Registry
    {

    public:

        static auto Instance() -> Registry&
        {
            static Registry r;

            return r;
        }

        auto RegisterRange(const TypeDesc* begin, const TypeDesc* end) -> void
        {
            for (const TypeDesc* p = begin; p != end; ++p)
            {
                auto [it, inserted] = typeById.emplace(p->id, p);
                nameToId.emplace(p->qualifiedName, p->id);

                FixUpTemplatedForType(*const_cast<TypeDesc*>(p));
            }
        }

        auto MapStdTypeIndex(std::type_index idx, TypeId id) -> void
        {
            byStdTypeIndex.emplace(idx, id);
        }

        auto Find(TypeId id) const -> const TypeDesc*
        {
            auto it = typeById.find(id);

            if (it == typeById.end())
                return nullptr;

            return it->second;
        }

        auto FindByQualifiedName(std::string_view qn) const -> const TypeDesc*
        {
            auto it = nameToId.find(qn);

            if (it == nameToId.end())
                return nullptr;

            return Find(it->second);
        }

        template <class T>
        auto Get() const -> const TypeDesc*
        {
            auto it = byStdTypeIndex.find(std::type_index(typeid(T)));

            if (it == byStdTypeIndex.end())
                return nullptr;

            return Find(it->second);
        }

        auto Get(std::string_view name) const -> const TypeDesc*
        {
            if (const TypeDesc* t = FindByQualifiedName(name))
                return t;

            const TypeDesc* hit = nullptr;

            for (const auto& kv : typeById)
            {
                const TypeDesc* p = kv.second;

                if (p != nullptr && p->name == name)
                {
                    if (hit != nullptr)
                        return nullptr;

                    hit = p;
                }
            }

            return hit;
        }

        auto GetAllBySimpleName(std::string_view simpleName) const -> std::vector<const TypeDesc*>
        {
            std::vector<const TypeDesc*> v; v.reserve(2);

            for (const auto& kv : typeById)
            {
                const TypeDesc* p = kv.second;

                if (p != nullptr && p->name == simpleName)
                    v.push_back(p);
            }

            return v;
        }

        auto AttachTemplatedDispatcher(std::string_view classQualifiedName, std::string_view methodSimpleOrQualifiedName, MethodTypeTemplatedErased::ErasedTemplatedCaller caller, void* binderTag) -> bool
        {
            if (const TypeDesc* typeDesc = FindByQualifiedName(classQualifiedName))
            {
                TypeDesc* t = const_cast<TypeDesc*>(typeDesc);

                for (TemplatedMethodDesc& m : t->templatedMethods)
                {
                    if (MatchName(m.name, methodSimpleOrQualifiedName) || MatchName(m.qualifiedName, methodSimpleOrQualifiedName))
                    {
                        m.erasedTemplatedCaller = reinterpret_cast<void*>(caller);
                        m.binderTag = binderTag;

                        return true;
                    }
                }
            }

            PendingKey k{ std::string(classQualifiedName), std::string(methodSimpleOrQualifiedName) };

            pendingTemplated[k] = PendingEntry{ caller, binderTag };

            return false;
        }

        auto New(std::string_view typeName, void** args, size_t argc, const std::type_info* const* argTypes, bool accessibilityConsidered = true) -> void*
        {
            const TypeDesc* t = Get(typeName);
            
            if (!t)
                return nullptr;
            
            return New(*t, args, argc, argTypes, accessibilityConsidered);
        }

        auto ConstructInto(std::string_view typeName, void* storage, void** args, size_t argc, const std::type_info* const* argTypes, bool accessibilityConsidered = true) -> bool
        {
            const TypeDesc* t = Get(typeName);

            if (!t)
                return false;
            
            return ConstructInto(*t, storage, args, argc, argTypes, accessibilityConsidered);
        }

        auto Delete(std::string_view typeName, void* p, bool accessibilityConsidered = true) -> bool
        {
            const TypeDesc* t = Get(typeName);
            
            if (!t)
                return false;
            
            return Delete(*t, p, accessibilityConsidered);
        }

        template <class T>
        auto New(void** args, size_t argc, const std::type_info* const* argTypes, bool accessibilityConsidered = true) -> T*
        {
            const TypeDesc* t = Get<T>(); if (!t) return nullptr; return reinterpret_cast<T*>(New(*t, args, argc, argTypes, accessibilityConsidered));
        }

private:

        struct PendingKey
        {
            std::string typeQN;
            std::string method;

            bool operator==(const PendingKey& o) const noexcept
            {
                return typeQN == o.typeQN && method == o.method;
            }
        };
        struct PendingKeyHash
        {
            size_t operator()(const PendingKey& k) const noexcept
            {
                size_t h1 = std::hash<std::string>{}(k.typeQN);
                size_t h2 = std::hash<std::string>{}(k.method);

                return h1 ^ (h2 << 1);
            }
        };

        struct PendingEntry
        {
            MethodTypeTemplatedErased::ErasedTemplatedCaller caller;
            void* binderTag;
        };

        static auto MatchCtor(const CtorDesc& c, size_t argc, const std::type_info* const* argTypes) noexcept -> bool
        {
            if (c.parameters.size() != argc)
                return false;

            for (size_t i = 0; i < argc; ++i)
            {
                if (c.parameters[i].type.qualifiedName != std::string_view(argTypes[i]->name()))
                    return false;
            }

            return true;
        }

        static auto FindCtor(const TypeDesc& t, size_t argc, const std::type_info* const* argTypes, bool accessibilityConsidered) -> const CtorDesc*
        {
            for (const CtorDesc& c : t.constructors)
            {
                if (!c.erasedCtor)
                    continue;

                if (accessibilityConsidered && c.access != Access::PUBLIC)
                    continue;

                if (c.parameters.size() != argc)
                    continue;

                bool ok = true;

                for (size_t i = 0; i < argc; ++i)
                {
                    if (c.parameters[i].type.qualifiedName != std::string_view(argTypes[i]->name()))
                    {
                        ok = false;
                        break;
                    }
                }

                if (ok)
                    return &c;
            }

            return nullptr;
        }

        auto New(const TypeDesc& t, void** args, size_t argc, const std::type_info* const* argTypes, bool accessibilityConsidered) -> void*
        {
            const CtorDesc* c = FindCtor(t, argc, argTypes, accessibilityConsidered);
            
            if (!c)
                return nullptr;
            
            void* mem = ::operator new(t.sizeInBytes, std::align_val_t(t.alignInBytes));
            
            c->erasedCtor(mem, args);
            
            return mem;
        }

        auto ConstructInto(const TypeDesc& t, void* storage, void** args, size_t argc, const std::type_info* const* argTypes, bool accessibilityConsidered) -> bool
        {
            const CtorDesc* c = FindCtor(t, argc, argTypes, accessibilityConsidered);
            
            if (!c)
                return false;
            
            c->erasedCtor(storage, args);
            
            return true;
        }

        auto Delete(const TypeDesc& t, void* p, bool accessibilityConsidered) -> bool
        {
            if (!t.destructor.has_value())
                return false;
            
            const DtorDesc& d = *t.destructor;
            
            if (accessibilityConsidered && d.access != Access::PUBLIC)
                return false; d.erasedDtor(p);
            
            ::operator delete(p, std::align_val_t(t.alignInBytes));
            
            return true;
        }

        static bool MatchName(std::string_view stored, std::string_view q) noexcept
        {
            return stored == q || (stored.rfind("::") != std::string_view::npos && stored.substr(stored.rfind("::") + 2) == q);
        }

        void FixUpTemplatedForType(TypeDesc& t)
        {
            for (TemplatedMethodDesc& m : t.templatedMethods)
            {
                if (m.erasedTemplatedCaller)
                    continue;

                PendingKey k1{ std::string(t.qualifiedName), std::string(m.name) };
                PendingKey k2{ std::string(t.qualifiedName), std::string(m.qualifiedName) };

                auto tryApply = [&](const PendingKey& k)
                {
                    auto it = pendingTemplated.find(k);

                    if (it == pendingTemplated.end())
                        return false;

                    m.erasedTemplatedCaller = reinterpret_cast<void*>(it->second.caller);
                    m.binderTag = it->second.binderTag;

                    pendingTemplated.erase(it);

                    return true;
                };

                (void)(tryApply(k1) || tryApply(k2));
            }
        }

        std::unordered_map<TypeId, const TypeDesc*, TypeId::Hash> typeById;
        std::unordered_map<std::string_view, TypeId> nameToId;
        std::unordered_map<std::type_index, TypeId> byStdTypeIndex;
        std::unordered_map<PendingKey, PendingEntry, PendingKeyHash> pendingTemplated;

    };

    template <typename Tag, typename T>
    struct ConceptType
    {
        using TagType = Tag;
        using Arg = T;
    };

}