#pragma once

#include "Constructor.hpp"
#include "Destructor.hpp"
#include "Field.hpp"
#include "Method.hpp"
#include "Modifier.hpp"
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class Class {
private:
  std::string name_;
  std::string namespaceName_;
  std::string className_;
  Modifier modifier_;
  bool primitive_;
  std::vector<const Class *> superclasses_;
  std::vector<Field> fields_;
  std::vector<Method> methods_;
  std::vector<Constructor> constructors_;
  std::vector<Destructor> destructors_;

  static std::mutex &staticMutex();
  static std::unordered_map<std::string, std::unique_ptr<Class>> &registry();

public:
  Class();
  explicit Class(std::string name, bool primitive = false);
  Class(std::string name, Modifier modifier,
        std::vector<const Class *> superclasses, std::vector<Field> fields,
        std::vector<Method> methods, std::vector<Constructor> constructors,
        std::string namespaceName = {},
        std::vector<Destructor> destructors = {}, bool primitive = false);

  const std::string &getName() const;
  const std::string &getNamespaceName() const;
  const std::string &getClassName() const;
  Modifier getModifier() const;
  bool isPrimitive() const;

  const std::vector<const Class *> &getSuperclasses() const;
  const std::vector<Field> &getFields() const;
  const std::vector<Method> &getMethods() const;
  const std::vector<Constructor> &getConstructors() const;
  const std::vector<Destructor> &getDestructors() const;

  std::vector<const Field *> getDeclaredFields() const;
  std::vector<const Method *> getDeclaredMethods() const;
  std::vector<const Constructor *> getDeclaredConstructors() const;

  bool isEmpty() const;

  static const Class &putClass(const std::string &name, const Class &cls);
  static bool hasClass(const std::string &name);
  static const Class *findClass(const std::string &name);
  static const Class &getClass(const std::string &name);
  static std::vector<const Class *> getAllClasses();
  static std::size_t classCount();
  static void clearClasses();

  static bool isSubclass(const Class *sub, const Class *base);
  static bool isSubclass(const std::string &sub, const std::string &base);
  static bool isAssignable(const Class *from, const Class *to);
  static bool isAssignable(const std::string &from, const std::string &to);
};