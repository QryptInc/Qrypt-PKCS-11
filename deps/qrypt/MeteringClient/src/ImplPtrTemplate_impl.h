#pragma once

/**
 * This is the implementation of the ImplPtrTemplate class.
 */

#include <utility>

template <typename T>
ImplPtrTemplate<T>::ImplPtrTemplate() : m{new T{}} {}

template <typename T>
template <typename... Args>
ImplPtrTemplate<T>::ImplPtrTemplate(Args &&... args)
    : m{new T{std::forward<Args>(args)...}} {}

template <typename T>
ImplPtrTemplate<T>::~ImplPtrTemplate() {}

// movable
template <typename T>
ImplPtrTemplate<T>::ImplPtrTemplate(ImplPtrTemplate<T> &&) noexcept = default;
template <typename T>
ImplPtrTemplate<T> &ImplPtrTemplate<T>::operator=(ImplPtrTemplate<T> &&rhs) noexcept = default;

template <typename T>
T *ImplPtrTemplate<T>::operator->() { return m.get(); }

template <typename T>
T &ImplPtrTemplate<T>::operator*() { return *m.get(); }