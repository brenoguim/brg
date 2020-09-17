#include <initializer_list>
#include <type_traits>
#include <tuple>
#include <cassert>
#include <iostream>
#include <memory>
#include <limits>

template<class T> struct printer;

template<class T> struct UnsizedArray
{
    operator T*() const { return m_begin; }
    T* get() const { return m_begin; }
    T* const m_begin;
};

template<class T> struct SizedArray
{
    auto begin() const { return m_begin; }
    auto end() const { return m_end; }
    auto size() const { return end() - begin(); }
    T* const m_begin; T* const m_end;
};

template<class T> struct UnboundedArrayToPointer { using type = T; };

template<class T> requires std::is_trivially_destructible<T>::value
struct UnboundedArrayToPointer<T[]> { using type = UnsizedArray<T>; };

template<class T> requires (!std::is_trivially_destructible<T>::value)
struct UnboundedArrayToPointer<T[]> { using type = SizedArray<T>; };

template<class T>
struct ArrayPlaceHolder
{
    ArrayPlaceHolder(std::size_t size) : m_size(size) {}
    operator UnsizedArray<T>() { return {begin()}; }
    operator SizedArray<T>() { return {begin(), end()}; }

    void consume(void*& buf, std::size_t& space)
    {
        m_ptr = std::align(alignof(T), numBytes(), buf, space);
        assert(m_ptr);
        space -= numBytes();

        for (auto& el : *this) new (&el) T;
    }

    std::size_t numRequiredBytes(std::size_t offset)
    {
        std::size_t space = std::numeric_limits<std::size_t>::max();
        auto originalSpace = space;
        void* ptr = static_cast<char*>(nullptr) + offset;
        auto r = std::align(alignof(T), numBytes(), ptr, space);
        assert(r);
        return (originalSpace - space) + numBytes();
    }

    auto numBytes() const { return m_size*sizeof(T); }
    T* begin() const { return static_cast<T*>(m_ptr); }
    T* end() const { return begin() + m_size; }

    std::size_t m_size;
    void* m_ptr {nullptr};
};

template<class T> struct is_array_placeholder : std::false_type {};
template<class T> struct is_array_placeholder<ArrayPlaceHolder<T>> : std::true_type {};

template<class T> struct is_unsized_array : std::false_type {};
template<class T> struct is_unsized_array<UnsizedArray<T>> : std::true_type {};

template<class T> struct is_sized_array : std::false_type {};
template<class T> struct is_sized_array<SizedArray<T>> : std::true_type {};

template<class T> struct PreImplConverter { using type = T; };
template<class T> struct PreImplConverter<T[]> { using type = ArrayPlaceHolder<T>; };

namespace detail
{
    template<int... Is>
    struct seq { };

    template<int N, int... Is>
    struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };

    template<int... Is>
    struct gen_seq<0, Is...> : seq<Is...> { };
}

namespace detail
{
    template<typename T, typename F, int... Is>
    void for_each(T&& t, F f, std::integer_sequence<int, Is...>)
    {
        auto l = { (f(std::get<Is>(t)), 0)... };
    }
}

template<typename... Ts, typename F>
void for_each_in_tuple(std::tuple<Ts...>& t, F f)
{
    detail::for_each(t, f, std::make_integer_sequence<int, sizeof...(Ts)>());
}

template<typename... Ts, typename F>
void for_each_in_tuple(const std::tuple<Ts...>& t, F f)
{
    detail::for_each(t, f, std::make_integer_sequence<int, sizeof...(Ts)>());
}

template<class MemberEnum, class... T>
struct FlexibleLayoutClass
{ 
    using PreImpl = std::tuple<typename PreImplConverter<T>::type...>;

    class Impl : public std::tuple<typename UnboundedArrayToPointer<T>::type...>
    {
      public:
        template<MemberEnum e> auto& get() { return std::get<e>(*this); }
        template<MemberEnum e> auto& get() const { return std::get<e>(*this); }

        static void deleet(const Impl* p)
        {
            if (!p) return;
            for_each_in_tuple(*p,
                []<class U>(U& u) {
                    if constexpr (is_sized_array<U>::value)
                        std::destroy(u.begin(), u.end());
                });
            p->~Impl();
            ::operator delete(const_cast<Impl*>(p));
        }
      private:
        friend class FlexibleLayoutClass;
        using Base = std::tuple<typename UnboundedArrayToPointer<T>::type...>;
        using Base::Base;
        ~Impl() = default;
    };


    template<class... Args>
    static auto niw(Args&&... args)
    {
        PreImpl pi(std::forward<Args>(args)...);

        std::size_t numBytesForArrays = 0;
        for_each_in_tuple(pi,
            [&numBytesForArrays]<class U>(U& u) mutable {
                if constexpr (is_array_placeholder<U>::value)
                    numBytesForArrays += u.numRequiredBytes(sizeof(Impl) + numBytesForArrays);
            });

        auto implBuffer = ::operator new(sizeof(Impl) + numBytesForArrays);
        void* arrayBuffer = static_cast<char*>(implBuffer) + sizeof(Impl);

        for_each_in_tuple(pi,
            [arrayBuffer, &numBytesForArrays]<class U>(U& u) mutable {
                if constexpr (is_array_placeholder<U>::value)
                    u.consume(arrayBuffer, numBytesForArrays);
            });

        assert(numBytesForArrays == 0);

        return new (implBuffer) Impl(std::move(pi));
    }
};

template<class T> void deleet(const T* p) { T::deleet(p); }



#include <cstring>


auto foo()
{
    enum Members {RefCount, Header, Data, Checksum};
    using Message = FlexibleLayoutClass<Members, int, std::string, char[], std::string>;

    auto r = Message::niw(1, "header", /*msg size=*/ 1000, "f114ffabd31");
    
    std::strcpy(r->get<Data>(), "This is my message!");

    std::cout << "Refcount: " << r->get<RefCount>()
              << "\nHeader: " << r->get<Header>()
              << "\nChecksum: " << r->get<Checksum>()
              << "\nData: " << r->get<Data>()
              << std::endl;

    deleet(r);
}
