#include "Inspector.hpp"
#include "Constructor.hpp"
#include "Field.hpp"
#include "Method.hpp"
#include <ostream>

void Inspector::inspect(const Class &classInfo, std::ostream &output) const {
  output << "Class: " << classInfo.getName() << '\n';

  const std::vector<const Class *> &superclasses =
      classInfo.getSuperclasses();
  output << "  Bases:";
  if (superclasses.empty()) {
    output << " none";
  }
  for (const Class *superclass : superclasses) {
    output << ' ' << superclass->getName();
  }
  output << '\n';

  output << "  Fields:";
  if (classInfo.getFields().empty()) {
    output << " none";
  }
  for (const Field &field : classInfo.getFields()) {
    output << ' ' << field.getName();
  }
  output << '\n';

  output << "  Methods:";
  if (classInfo.getMethods().empty()) {
    output << " none";
  }
  for (const Method &method : classInfo.getMethods()) {
    output << ' ' << method.getName();
    if (method.getModifier().isStatic()) {
      output << " (static)";
    }
  }
  output << '\n';
  output << "  Constructors: " << classInfo.getConstructors().size() << '\n';
  output << "  Destructors: " << classInfo.getDestructors().size() << '\n';
}