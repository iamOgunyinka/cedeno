#pragma once

#include <condition_variable>
#include <deque>
#include <list>
#include <mutex>

namespace utils {

template <typename T, typename Container = std::deque<T>>
struct waitable_container_t {
private:
  std::mutex m_mutex{};
  Container m_container{};
  std::condition_variable m_cv{};

public:
  waitable_container_t(Container &&container)
      : m_container{std::move(container)} {}
  waitable_container_t() = default;

  waitable_container_t(waitable_container_t &&vec)
      : m_mutex{std::move(vec.m_mutex)},
        m_container{std::move(vec.m_container)}, m_cv{std::move(vec.m_cv)} {}
  waitable_container_t &operator=(waitable_container_t &&) = delete;
  waitable_container_t(waitable_container_t const &) = delete;
  waitable_container_t &operator=(waitable_container_t const &) = delete;

  void clear() {
    std::lock_guard<std::mutex> lock_g{m_mutex};
    m_container.clear();
  }

  T get() {
    std::unique_lock<std::mutex> u_lock{m_mutex};
    m_cv.wait(u_lock, [this] { return !m_container.empty(); });
    if (m_container.empty()) // avoid spurious wakeup
      throw std::runtime_error("container is empty when it isn't supposed to");
    T value{std::move(m_container.front())};
    m_container.pop_front();
    return value;
  }

  template <typename U> void append(U &&data) {
    std::lock_guard<std::mutex> lock_g{m_mutex};
    m_container.push_back(std::forward<U>(data));
    m_cv.notify_all();
  }
};

template <typename T> struct mutexed_list_t {
private:
  std::mutex m_mutex;
  std::list<T> m_list;

public:
  using value_type = T;
  mutexed_list_t() = default;
  template <typename U> void append(U &&data) {
    std::lock_guard<std::mutex> lock_g{m_mutex};
    m_list.emplace_back(std::forward<U>(data));
  }
  T get() {
    if (isEmpty())
      throw std::runtime_error("container is empty");

    std::lock_guard<std::mutex> lock_g{m_mutex};
    T value{m_list.front()};
    m_list.pop_front();
    return value;
  }

  bool isEmpty() {
    std::lock_guard<std::mutex> lock_g{m_mutex};
    return m_list.empty();
  }
};
} // namespace utils
