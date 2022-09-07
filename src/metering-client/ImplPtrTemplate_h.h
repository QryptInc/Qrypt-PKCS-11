#pragma once

/**
 * Heavy reference to https://www.bfilipek.com/2018/01/pimpl.html and links in there.
 * Suggested wrapper from Herb Butter(https://herbsutter.com/gotw/_101/). This is a
 * wrapper template for PIMPL idoim and cuts down on boilerplate code in the private
 * implementations.
 */

#include <memory> // unique_ptr

// taken from Herb Sutter
template <typename T>
class ImplPtrTemplate
{
private:
    std::unique_ptr<T> m;

public:
    ImplPtrTemplate();
    template <typename... Args>
    ImplPtrTemplate(Args &&...);
    ~ImplPtrTemplate();
    // movable:
    ImplPtrTemplate(ImplPtrTemplate &&rhs) noexcept;
    ImplPtrTemplate &operator=(ImplPtrTemplate &&rhs) noexcept;
    // not copyable
    ImplPtrTemplate(const ImplPtrTemplate &rhs) = delete;
    ImplPtrTemplate &operator=(const ImplPtrTemplate &rhs) = delete;
    T *operator->();
    T &operator*();
};
