#include "Class.hpp"
#include <stdexcept>
#include <utility>

__attribute__((constructor))
static void Class_init() {
  static const char *const primitives[] = {
      "void",
      "bool",
      "char",
      "signed char",
      "unsigned char",
      "wchar_t",
      "char8_t",
      "char16_t",
      "char32_t",
      "short",
      "unsigned short",
      "int",
      "unsigned int",
      "long",
      "unsigned long",
      "long long",
      "unsigned long long",
      "float",
      "double",
      "long double",
      "std::size_t",
      "std::ptrdiff_t",
      "std::int8_t",
      "std::int16_t",
      "std::int32_t",
      "std::int64_t",
      "std::uint8_t",
      "std::uint16_t",
      "std::uint32_t",
      "std::uint64_t",
  };

  for (const char *name : primitives) {
    Class::putClass(name, Class(name, true));
  }
}

std::mutex &Class::staticMutex() {
  static std::mutex m;
  return m;
}

std::unordered_map<std::string, std::unique_ptr<Class>> &Class::registry() {
  static std::unordered_map<std::string, std::unique_ptr<Class>> r;
  return r;
}

Class::Class()
    : name_(), namespaceName_(), className_(), modifier_(), primitive_(false),
      superclasses_(), fields_(), methods_(), constructors_(), destructors_() {}

Class::Class(std::string name, bool primitive)
    : name_(name), namespaceName_(), className_(std::move(name)),
      modifier_(), primitive_(primitive), superclasses_(), fields_(),
      methods_(), constructors_(), destructors_() {}

Class::Class(std::string name, Modifier modifier,
             std::vector<const Class *> superclasses, std::vector<Field> fields,
             std::vector<Method> methods, std::vector<Constructor> constructors,
             std::string namespaceName, std::vector<Destructor> destructors,
             bool primitive)
    : name_(namespaceName.empty() ? name : namespaceName + "::" + name),
      namespaceName_(std::move(namespaceName)), className_(std::move(name)),
      modifier_(modifier), primitive_(primitive),
      superclasses_(std::move(superclasses)), fields_(std::move(fields)),
      methods_(std::move(methods)), constructors_(std::move(constructors)),
      destructors_(std::move(destructors)) {}

const std::string &Class::getName() const { return name_; }

const std::string &Class::getNamespaceName() const { return namespaceName_; }

const std::string &Class::getClassName() const { return className_; }

Modifier Class::getModifier() const { return modifier_; }

bool Class::isPrimitive() const { return primitive_; }

const std::vector<const Class *> &Class::getSuperclasses() const {
  return superclasses_;
}

const std::vector<Field> &Class::getFields() const { return fields_; }

const std::vector<Method> &Class::getMethods() const { return methods_; }

const std::vector<Constructor> &Class::getConstructors() const {
  return constructors_;
}

const std::vector<Destructor> &Class::getDestructors() const {
  return destructors_;
}

std::vector<const Field *> Class::getDeclaredFields() const {
  std::vector<const Field *> result;
  for (std::vector<Field>::const_iterator it = fields_.begin();
       it != fields_.end(); ++it) {
    if (it->getDeclaringClass() == this) {
      result.push_back(&*it);
    }
  }
  return result;
}

std::vector<const Method *> Class::getDeclaredMethods() const {
  std::vector<const Method *> result;
  for (std::vector<Method>::const_iterator it = methods_.begin();
       it != methods_.end(); ++it) {
    if (it->getDeclaringClass() == this) {
      result.push_back(&*it);
    }
  }
  return result;
}

std::vector<const Constructor *> Class::getDeclaredConstructors() const {
  std::vector<const Constructor *> result;
  for (std::vector<Constructor>::const_iterator it = constructors_.begin();
       it != constructors_.end(); ++it) {
    if (it->getDeclaringClass() == this) {
      result.push_back(&*it);
    }
  }
  return result;
}

bool Class::isEmpty() const {
  return fields_.empty() && methods_.empty() && constructors_.empty() &&
         destructors_.empty() && superclasses_.empty();
}

const Class &Class::putClass(const std::string &name, const Class &cls) {
  std::lock_guard<std::mutex> lock(staticMutex());
  auto &reg = registry();
  auto it = reg.find(name);
  if (it != reg.end()) {
    *it->second = cls;
    return *it->second;
  }
  auto result = reg.emplace(name, std::make_unique<Class>(cls));
  return *result.first->second;
}

bool Class::hasClass(const std::string &name) {
  std::lock_guard<std::mutex> lock(staticMutex());
  return registry().count(name) != 0;
}

const Class *Class::findClass(const std::string &name) {
  std::lock_guard<std::mutex> lock(staticMutex());
  auto &reg = registry();
  auto it = reg.find(name);
  return it == reg.end() ? nullptr : it->second.get();
}

const Class &Class::getClass(const std::string &name) {
  std::lock_guard<std::mutex> lock(staticMutex());
  auto &reg = registry();
  auto it = reg.find(name);
  if (it == reg.end()) {
    throw std::out_of_range("Class not found: " + name);
  }
  return *it->second;
}

std::vector<const Class *> Class::getAllClasses() {
  std::lock_guard<std::mutex> lock(staticMutex());
  std::vector<const Class *> result;
  result.reserve(registry().size());
  for (const auto &kv : registry()) {
    result.push_back(kv.second.get());
  }
  return result;
}

std::size_t Class::classCount() {
  std::lock_guard<std::mutex> lock(staticMutex());
  return registry().size();
}

void Class::clearClasses() {
  std::lock_guard<std::mutex> lock(staticMutex());
  registry().clear();
}

bool Class::isSubclass(const Class *sub, const Class *base) {
  if (sub == nullptr || base == nullptr) {
    return false;
  }
  if (sub == base || sub->getName() == base->getName()) {
    return true;
  }
  for (const Class *parent : sub->getSuperclasses()) {
    if (isSubclass(parent, base)) {
      return true;
    }
  }
  return false;
}

bool Class::isSubclass(const std::string &sub, const std::string &base) {
  if (sub == base) {
    return true;
  }
  const Class *subCls = findClass(sub);
  const Class *baseCls = findClass(base);
  if (subCls == nullptr || baseCls == nullptr) {
    return false;
  }
  return isSubclass(subCls, baseCls);
}

bool Class::isAssignable(const Class *from, const Class *to) {
  if (from == nullptr || to == nullptr) {
    return false;
  }
  if (from == to || from->getName() == to->getName()) {
    return true;
  }
  for (const Class *parent : from->getSuperclasses()) {
    if (isAssignable(parent, to)) {
      return true;
    }
  }
  return false;
}

bool Class::isAssignable(const std::string &from, const std::string &to) {
  if (from == to) {
    return true;
  }
  const Class *fromCls = findClass(from);
  const Class *toCls = findClass(to);
  if (fromCls == nullptr || toCls == nullptr) {
    return false;
  }
  return isAssignable(fromCls, toCls);
}