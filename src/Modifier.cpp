#include "Modifier.hpp"

Modifier::Modifier() : bits_(NONE) {}
Modifier::Modifier(uint32_t bits) : bits_(bits) {}

uint32_t Modifier::bits() const { return bits_; }
Modifier::operator uint32_t() const { return bits_; }

Modifier &Modifier::operator|=(Modifier o) {
  bits_ |= o.bits_;
  return *this;
}

Modifier &Modifier::operator&=(Modifier o) {
  bits_ &= o.bits_;
  return *this;
}

Modifier &Modifier::operator^=(Modifier o) {
  bits_ ^= o.bits_;
  return *this;
}

bool Modifier::has(uint32_t f) const { return (bits_ & f) != 0; }
bool Modifier::is(uint32_t f) const { return bits_ == f; }
bool Modifier::isNone() const { return bits_ == 0; }

bool Modifier::isPublic() const { return has(PUBLIC); }
bool Modifier::isPrivate() const { return has(PRIVATE); }
bool Modifier::isProtected() const { return has(PROTECTED); }
bool Modifier::isStatic() const { return has(STATIC); }
bool Modifier::isConst() const { return has(CONST); }
bool Modifier::isVirtual() const { return has(VIRTUAL); }
bool Modifier::isFinal() const { return has(FINAL); }
bool Modifier::isVolatile() const { return has(VOLATILE); }
bool Modifier::isConstexpr() const { return has(CONSTEXPR); }
bool Modifier::isMutable() const { return has(MUTABLE); }
bool Modifier::isInline() const { return has(INLINE); }
bool Modifier::isExplicit() const { return has(EXPLICIT); }
bool Modifier::isNoexcept() const { return has(NOEXCEPT); }
bool Modifier::isOverride() const { return has(OVERRIDE); }
bool Modifier::isDeleted() const { return has(DELETED); }
bool Modifier::isDefaulted() const { return has(DEFAULTED); }
bool Modifier::isAbstract() const { return has(ABSTRACT); }
bool Modifier::isThreadLocal() const { return has(THREAD_LOCAL); }

Modifier operator|(Modifier a, Modifier b) {
  return Modifier(a.bits_ | b.bits_);
}

Modifier operator&(Modifier a, Modifier b) {
  return Modifier(a.bits_ & b.bits_);
}

Modifier operator^(Modifier a, Modifier b) {
  return Modifier(a.bits_ ^ b.bits_);
}

Modifier operator~(Modifier a) { return Modifier(~a.bits_); }

bool operator==(Modifier a, Modifier b) { return a.bits_ == b.bits_; }
bool operator!=(Modifier a, Modifier b) { return a.bits_ != b.bits_; }

std::string Modifier::toString() const {
  if (bits_ == 0) {
    return "none";
  }

  std::string s;

  auto append = [&s](const char *word) {
    if (!s.empty()) {
      s += ' ';
    }
    s += word;
  };

  switch (bits_ & (PUBLIC | PRIVATE | PROTECTED)) {
  case PUBLIC:
    append("public");
    break;
  case PRIVATE:
    append("private");
    break;
  case PROTECTED:
    append("protected");
    break;
  default:
    break;
  }

  if (has(STATIC))       append("static");
  if (has(CONST))        append("const");
  if (has(VOLATILE))     append("volatile");
  if (has(CONSTEXPR))    append("constexpr");
  if (has(MUTABLE))      append("mutable");
  if (has(THREAD_LOCAL)) append("thread_local");
  if (has(INLINE))       append("inline");
  if (has(VIRTUAL))      append("virtual");
  if (has(OVERRIDE))     append("override");
  if (has(FINAL))        append("final");
  if (has(ABSTRACT))     append("abstract");
  if (has(EXPLICIT))     append("explicit");
  if (has(NOEXCEPT))     append("noexcept");
  if (has(DELETED))      append("= delete");
  if (has(DEFAULTED))    append("= default");

  return s;
}