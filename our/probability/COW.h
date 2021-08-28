#pragma once

#include <memory>

//template class used to enforce the copy-on-write pattern
template<typename T=float>
class COW{
public:
    COW(){}
    // call this function in derived constructor
    void construct() {
        m_ptr = std::make_shared<T>();
    }

    // call this function in derived's setter before other code
    void clone_if_needed() {
        if (m_ptr.use_count() > 1) {
            std::shared_ptr<T> old = m_ptr;
            construct();
            *m_ptr = *old; // copy the old contents to new ptr.
        }
    }

    void duplicate(const COW<T>& c) {
        m_ptr = c.m_ptr;
    }

    // function to get the internal raw ptr
    const T* ptr() const {
        return m_ptr.get();
    }

    T* ptr() {
        return m_ptr.get();
    }

    // returns count of the shared_ptr instance
    long use_count() const {
        return m_ptr.use_count();
    }

private:
    std::shared_ptr<T> m_ptr;
};
