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

template<class T> struct TransformUnboundedArrays { using type = T; };

template<class T> requires std::is_trivially_destructible<T>::value
struct TransformUnboundedArrays<T[]> { using type = UnsizedArray<T>; };

template<class T> requires (!std::is_trivially_destructible<T>::value)
struct TransformUnboundedArrays<T[]> { using type = SizedArray<T>; };

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
template<class T> struct PreImplConverter<UnsizedArray<T>> { using type = ArrayPlaceHolder<T>; };
template<class T> struct PreImplConverter<SizedArray<T>> { using type = ArrayPlaceHolder<T>; };


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

template<class Derived, class... T>
class FlexibleLayoutClass : public std::tuple<typename TransformUnboundedArrays<T>::type...>
{ 
  private:
    using Base = std::tuple<typename TransformUnboundedArrays<T>::type...>;
    using Base::Base;

  protected:
    using FLC = FlexibleLayoutClass;
    ~FlexibleLayoutClass() = default;

  public:
    template<auto e> auto& get() { return std::get<e>(*this); }
    template<auto e> auto& get() const { return std::get<e>(*this); }

    template<class... Args>
    static auto niw(Args&&... args)
    {
        static_assert(sizeof(Derived) == sizeof(FlexibleLayoutClass));

        using PreImpl = std::tuple<typename PreImplConverter<T>::type...>;
        PreImpl pi(std::forward<Args>(args)...);

        std::size_t numBytesForArrays = 0;
        for_each_in_tuple(pi,
            [&numBytesForArrays]<class U>(U& u) mutable {
                if constexpr (is_array_placeholder<U>::value)
                    numBytesForArrays += u.numRequiredBytes(sizeof(FlexibleLayoutClass) + numBytesForArrays);
            });

        auto implBuffer = ::operator new(sizeof(FlexibleLayoutClass) + numBytesForArrays);
        void* arrayBuffer = static_cast<char*>(implBuffer) + sizeof(FlexibleLayoutClass);

        for_each_in_tuple(pi,
            [arrayBuffer, &numBytesForArrays]<class U>(U& u) mutable {
                if constexpr (is_array_placeholder<U>::value)
                    u.consume(arrayBuffer, numBytesForArrays);
            });

        assert(numBytesForArrays == 0);

        return new (implBuffer) Derived(std::move(pi));
    }

    static void deleet(const Derived* p)
    {
        if (!p) return;
        for_each_in_tuple(*p,
            []<class U>(U& u) {
                if constexpr (!std::is_trivially_destructible<U>::value)
                    std::destroy(u.begin(), u.end());
            });
        p->~Derived();
        ::operator delete(const_cast<Derived*>(p));
    }
};

template<class T> void deleet(const T* p) { T::deleet(p); }

#include <cstring>

struct Treta : public FlexibleLayoutClass<Treta, int, std::string, SizedArray<char>, std::string>
{
    enum Members {RefCount, Header, Data, Checksum};

    static auto* niw(std::string header) { return FLC::niw(1, std::move(header), 1000, "breno"); }
};

auto foo3()
{
    auto r = Treta::niw("header0000000000");
    
    std::strcpy(r->get<Treta::Data>().begin(), "This is my message!");

    std::cout << "Refcount: " << r->get<Treta::RefCount>()
              << "\nHeader: " << r->get<Treta::Header>()
              << "\nChecksum: " << r->get<Treta::Checksum>()
              << "\nData: " << r->get<Treta::Data>().begin()
              << "\nData size: " << r->get<Treta::Data>().size()
              << std::endl;

    deleet(r);
}

void log(int l) { printf("freeing! %d\n", l); }

void operator delete  ( void* ptr ) noexcept { log(__LINE__); }
void operator delete[]( void* ptr ) noexcept { log(__LINE__); }
void operator delete  ( void* ptr, std::align_val_t al ) noexcept { log(__LINE__); }
void operator delete[]( void* ptr, std::align_val_t al ) noexcept { log(__LINE__); }
void operator delete  ( void* ptr, std::size_t sz ) noexcept { log(__LINE__); }
void operator delete[]( void* ptr, std::size_t sz ) noexcept { log(__LINE__); }
void operator delete  ( void* ptr, std::size_t sz, std::align_val_t al ) noexcept { log(__LINE__); }
void operator delete[]( void* ptr, std::size_t sz, std::align_val_t al ) noexcept { log(__LINE__); }

int main() { foo3(); }
