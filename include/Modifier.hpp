#pragma once

#include <cstdint>
#include <string>

class Modifier {
private:
  uint32_t bits_;

public:
  static constexpr uint32_t NONE = 0U;
  static constexpr uint32_t PUBLIC = 1U << 0;
  static constexpr uint32_t PRIVATE = 1U << 1;
  static constexpr uint32_t PROTECTED = 1U << 2;
  static constexpr uint32_t STATIC = 1U << 3;
  static constexpr uint32_t CONST = 1U << 4;
  static constexpr uint32_t VIRTUAL = 1U << 5;
  static constexpr uint32_t FINAL = 1U << 6;
  static constexpr uint32_t VOLATILE = 1U << 7;
  static constexpr uint32_t CONSTEXPR = 1U << 8;
  static constexpr uint32_t MUTABLE = 1U << 9;
  static constexpr uint32_t INLINE = 1U << 10;
  static constexpr uint32_t EXPLICIT = 1U << 11;
  static constexpr uint32_t NOEXCEPT = 1U << 12;
  static constexpr uint32_t OVERRIDE = 1U << 13;
  static constexpr uint32_t DELETED = 1U << 14;
  static constexpr uint32_t DEFAULTED = 1U << 15;
  static constexpr uint32_t ABSTRACT = 1U << 16;
  static constexpr uint32_t THREAD_LOCAL = 1U << 17;

  Modifier();
  explicit Modifier(uint32_t bits);

  uint32_t bits() const;
  explicit operator uint32_t() const;

  Modifier &operator|=(Modifier o);
  Modifier &operator&=(Modifier o);
  Modifier &operator^=(Modifier o);

  bool has(uint32_t f) const;
  bool is(uint32_t f) const;
  bool isNone() const;

  bool isPublic() const;
  bool isPrivate() const;
  bool isProtected() const;
  bool isStatic() const;
  bool isConst() const;
  bool isVirtual() const;
  bool isFinal() const;
  bool isVolatile() const;
  bool isConstexpr() const;
  bool isMutable() const;
  bool isInline() const;
  bool isExplicit() const;
  bool isNoexcept() const;
  bool isOverride() const;
  bool isDeleted() const;
  bool isDefaulted() const;
  bool isAbstract() const;
  bool isThreadLocal() const;

  std::string toString() const;

  friend Modifier operator|(Modifier a, Modifier b);
  friend Modifier operator&(Modifier a, Modifier b);
  friend Modifier operator^(Modifier a, Modifier b);
  friend Modifier operator~(Modifier a);
  friend bool operator==(Modifier a, Modifier b);
  friend bool operator!=(Modifier a, Modifier b);
};