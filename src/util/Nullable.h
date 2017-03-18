// Copyright 2017, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosi@informatik.uni-freiburg.de>

#ifndef UTIL_NULLABLE_H_
#define UTIL_NULLABLE_H_

template<typename T> 
class Nullable {

 public:
  Nullable()
   : val(), null(true) {}
  Nullable(const T& value)
   : val(value), null(false) {} 
  Nullable(const Nullable& other)
   : val(other.val), null(other.isNull()) {}

  Nullable& operator=(const Nullable& other) {
    val = other.get();
    null = other.isNull();
    return *this;
  }

  T operator=(const T& other) {
    assign(other);
    return val;
  }


  /**
   * Passing through comparision operators
   */

  bool operator==(const Nullable& other) {
    return (other.isNull() && isNull) || other.get() == get();
  }

  bool operator!=(const Nullable& other) {
    return !(*this == other);
  }

  bool operator<(const Nullable& other) {
    return !other.isNull() && !isNull() && get() < other.get();
  }

  bool operator>(const Nullable& other) {
    return !(*this < other || *this == other);
  }

  bool operator<=(const Nullable& other) {
    return *this < other || *this == other;
  }

  bool operator>=(const Nullable& other) {
    return *this > other || *this == other;
  }

  bool operator==(const T& other) {
    return !isNull() && other == get();
  }

  bool operator!=(const T& other) {
    return !(*this == other);
  }

  bool operator<(const T& other) {
    return !isNull() && get() < other;
  }

  bool operator>(const T& other) {
    return !(*this < other || *this == other);
  }

  bool operator<=(const T& other) {
    return *this < other || *this == other;
  }

  bool operator>=(const T& other) {
    return *this > other || *this == other;
  }

  operator T() const {
    return get();
  }

  T& get() const {
    if (isNull()) return val;
    else throw std::runtime_error("Trying to retrieve value of NULL object.");
  }

  bool isNull() const { 
    return null;
  }

private:
  void assign(T v) {
    val = v;
    null = false;
  }

  T val;
  bool null;
};

#endif  // UTIL_NULLABLE_H_