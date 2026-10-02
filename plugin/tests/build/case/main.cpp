#include "Class.hpp"
#include "Constructor.hpp"
#include "Destructor.hpp"
#include "Field.hpp"
#include "Method.hpp"
#include "Modifier.hpp"
#include "Parameter.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const std::string &label) {
  if (condition) {
    std::cout << "[ok]   " << label << "\n";
  } else {
    std::cout << "[FAIL] " << label << "\n";
    ++failures;
  }
}

std::string typeName(const Class *cls) {
  return cls ? cls->getName() : std::string("void");
}

const Field *findField(const Class &cls, const std::string &name) {
  for (const Field *f : cls.getDeclaredFields()) {
    if (f->getName() == name) return f;
  }
  return nullptr;
}

const Method *findMethod(const Class &cls, const std::string &name) {
  for (const Method *m : cls.getDeclaredMethods()) {
    if (m->getName() == name) return m;
  }
  return nullptr;
}

const Constructor *findConstructor(const Class &cls, std::size_t argc) {
  for (const Constructor *c : cls.getDeclaredConstructors()) {
    if (c->getParameters().size() == argc) return c;
  }
  return nullptr;
}

void dumpAllClasses(std::ostream &out) {
  std::vector<const Class *> classes = Class::getAllClasses();
  std::sort(classes.begin(), classes.end(),
            [](const Class *a, const Class *b) {
              return a->getName() < b->getName();
            });

  out << "\n=== Registry: " << classes.size() << " classes ===\n\n";

  for (const Class *cls : classes) {
    out << "class " << cls->getName();
    if (cls->isPrimitive()) out << "  [primitive]";
    const std::string mods = cls->getModifier().toString();
    if (mods != "none") out << "  [" << mods << "]";
    out << "\n";

    const auto &bases = cls->getSuperclasses();
    if (!bases.empty()) {
      out << "  bases:";
      for (const Class *b : bases) out << " " << typeName(b);
      out << "\n";
    }

    for (const Field *f : cls->getDeclaredFields()) {
      out << "  field: " << f->getModifier().toString() << " "
          << typeName(f->getType()) << " " << f->getName() << "\n";
    }

    for (const Method *m : cls->getDeclaredMethods()) {
      out << "  method: " << m->getModifier().toString() << " "
          << typeName(m->getReturnType()) << " " << m->getName() << "(";
      const auto &params = m->getParameters();
      for (std::size_t i = 0; i < params.size(); ++i) {
        if (i > 0) out << ", ";
        out << typeName(params[i].getType()) << " " << params[i].getName();
      }
      out << ")\n";
    }

    for (const Constructor *c : cls->getDeclaredConstructors()) {
      out << "  ctor: " << c->getModifier().toString() << "(";
      const auto &params = c->getParameters();
      for (std::size_t i = 0; i < params.size(); ++i) {
        if (i > 0) out << ", ";
        out << typeName(params[i].getType()) << " " << params[i].getName();
      }
      out << ")\n";
    }

    if (!cls->getDestructors().empty()) {
      out << "  destructors: " << cls->getDestructors().size() << "\n";
    }
    out << "\n";
  }
}

} // namespace

int main() {
  dumpAllClasses(std::cout);

  // ============================================================
  // 1. Primitivos
  // ============================================================

  check(Class::hasClass("int"), "hasClass int");
  check(Class::hasClass("float"), "hasClass float");
  check(Class::hasClass("std::size_t"), "hasClass std::size_t");
  check(Class::findClass("int") != nullptr &&
            Class::findClass("int")->isPrimitive(),
        "int é primitivo");
  check(Class::findClass("float") != nullptr &&
            Class::findClass("float")->isPrimitive(),
        "float é primitivo");
  check(Class::findClass("void") != nullptr &&
            Class::findClass("void")->isPrimitive(),
        "void é primitivo");

  // ============================================================
  // 2. Behavior (base)
  // ============================================================

  const Class *behaviorPtr = Class::findClass("com::engine::Behavior");
  check(behaviorPtr != nullptr, "Behavior no registry");

  if (behaviorPtr == nullptr) {
    std::cout << "\n=== " << failures << " falha(s) ===\n";
    return failures == 0 ? 0 : 1;
  }

  const Class &behaviorClass = *behaviorPtr;
  check(!behaviorClass.isPrimitive(), "Behavior não é primitivo");
  check(behaviorClass.getClassName() == "Behavior", "Behavior.getClassName()");
  check(behaviorClass.getNamespaceName() == "com::engine",
        "Behavior.getNamespaceName()");
  check(behaviorClass.getName() == "com::engine::Behavior",
        "Behavior.getName() completo");
  check(behaviorClass.getSuperclasses().empty(),
        "Behavior não tem bases");

  // ============================================================
  // 3. Player — construtor, fields, hierarquia
  // ============================================================

  const Class *playerPtr = Class::findClass("com::engine::Player");
  check(playerPtr != nullptr, "Player no registry");

  if (playerPtr != nullptr) {
    const Class &playerClass = *playerPtr;

    check(playerClass.getClassName() == "Player", "Player.getClassName()");
    check(playerClass.getNamespaceName() == "com::engine",
          "Player.getNamespaceName()");
    check(playerClass.getName() == "com::engine::Player",
          "Player.getName() completo");
    check(playerClass.getSuperclasses().size() == 1,
          "Player tem 1 superclasse");
    check(!playerClass.getSuperclasses().empty() &&
              playerClass.getSuperclasses().front()->getClassName() ==
                  "Behavior",
          "Player herda Behavior");

    check(Class::isSubclass(&playerClass, &behaviorClass),
          "isSubclass(Player, Behavior)");
    check(!Class::isSubclass(&behaviorClass, &playerClass),
          "isSubclass(Behavior, Player) = false");
    check(Class::isSubclass(&playerClass, &playerClass),
          "isSubclass(Player, Player) = true");

    check(Class::isSubclass("com::engine::Player", "com::engine::Behavior"),
          "isSubclass por string");
    check(!Class::isSubclass("com::engine::Behavior", "com::engine::Player"),
          "isSubclass por string (invertido)");
    check(!Class::isSubclass("Missing", "com::engine::Player"),
          "isSubclass com tipo ausente");

    check(Class::isAssignable("com::engine::Player", "com::engine::Behavior"),
          "isAssignable(Player, Behavior)");
    check(!Class::isAssignable("com::engine::Behavior", "com::engine::Player"),
          "isAssignable(Behavior, Player) = false");

    // --- Fields ---

    const Field *lifeField = findField(playerClass, "life");
    const Field *attackField = findField(playerClass, "attack");

    check(lifeField != nullptr, "field Player::life");
    check(attackField != nullptr, "field Player::attack");
    check(lifeField != nullptr && lifeField->getType() != nullptr &&
              lifeField->getType()->getName() == "int",
          "Player::life é int");
    check(attackField != nullptr && attackField->getType() != nullptr &&
              attackField->getType()->getName() == "int",
          "Player::attack é int");
    check(lifeField != nullptr && lifeField->getModifier().isPublic(),
          "Player::life é público");

    // --- Construtor e uso ---

    const Constructor *ctor = findConstructor(playerClass, 1);
    check(ctor != nullptr, "Player tem ctor de 1 arg");

    if (ctor != nullptr && lifeField != nullptr && attackField != nullptr) {
      int initialLife = 100;
      void *player = ctor->construct({&initialLife});

      int lifeValue = 0;
      lifeField->get(player, &lifeValue);
      check(lifeValue == 100, "life após ctor");

      int attackValue = 0;
      attackField->get(player, &attackValue);
      check(attackValue == 0, "attack default = 0");

      int newLife = 42;
      lifeField->set(player, &newLife);
      lifeField->get(player, &lifeValue);
      check(lifeValue == 42, "life após set");

      int newAttack = 7;
      attackField->set(player, &newAttack);
      attackField->get(player, &attackValue);
      check(attackValue == 7, "attack após set");

      // Destrutor
      const auto &dtors = playerClass.getDestructors();
      check(dtors.size() == 1, "Player tem 1 destrutor");
      if (!dtors.empty()) {
        dtors.front().destruct(player);
      }
    }
  }

  // ============================================================
  // 4. Transform — ctor privado, fields privados
  // ============================================================

  const Class *transformPtr = Class::findClass("com::engine::Transform");
  check(transformPtr != nullptr, "Transform no registry");

  if (transformPtr != nullptr) {
    const Class &transformClass = *transformPtr;

    check(transformClass.getClassName() == "Transform",
          "Transform.getClassName()");
    check(transformClass.getSuperclasses().size() == 1,
          "Transform tem 1 superclasse");
    check(!transformClass.getSuperclasses().empty() &&
              transformClass.getSuperclasses().front()->getClassName() ==
                  "Behavior",
          "Transform herda Behavior");

    const Field *xField = findField(transformClass, "x");
    const Field *yField = findField(transformClass, "y");
    const Field *zField = findField(transformClass, "z");

    check(xField != nullptr, "field Transform::x");
    check(yField != nullptr, "field Transform::y");
    check(zField != nullptr, "field Transform::z");
    check(xField != nullptr && xField->getModifier().isPrivate(),
          "Transform::x é privado");
    check(yField != nullptr && yField->getModifier().isPrivate(),
          "Transform::y é privado");
    check(zField != nullptr && zField->getModifier().isPrivate(),
          "Transform::z é privado");

    const Constructor *ctor = findConstructor(transformClass, 3);
    check(ctor != nullptr, "Transform tem ctor de 3 args");
    check(ctor != nullptr && ctor->getModifier().isPrivate(),
          "Transform ctor é privado (mas acessível via reflection)");

    if (ctor != nullptr && xField != nullptr && yField != nullptr &&
        zField != nullptr) {
      int x = 1, y = 2, z = 3;
      void *transform = ctor->construct({&x, &y, &z});

      int xv = 0, yv = 0, zv = 0;
      xField->get(transform, &xv);
      yField->get(transform, &yv);
      zField->get(transform, &zv);

      check(xv == 1 && yv == 2 && zv == 3, "Transform valores do ctor");

      int newX = 10;
      xField->set(transform, &newX);
      xField->get(transform, &xv);
      check(xv == 10, "Transform::x após set em campo privado");

      const auto &dtors = transformClass.getDestructors();
      if (!dtors.empty()) {
        dtors.front().destruct(transform);
      }
    }
  }

  // ============================================================
  // 5. Registry: hasClass / findClass / getClass / putClass
  // ============================================================

  check(Class::hasClass("com::engine::Player"), "hasClass Player");
  check(!Class::hasClass("Missing"), "hasClass Missing = false");
  check(Class::findClass("Missing") == nullptr, "findClass Missing = null");

  bool threw = false;
  try {
    Class::getClass("Missing");
  } catch (const std::out_of_range &) {
    threw = true;
  }
  check(threw, "getClass Missing lança out_of_range");

  // putClass preserva endereço e sobrescreve conteúdo
  const Class *before = Class::findClass("com::engine::Player");
  check(before != nullptr, "Player no registry antes do putClass");

  if (before != nullptr) {
    Class replacement("Player", Modifier(Modifier::PRIVATE), {}, {}, {}, {},
                      "com::engine");
    Class::putClass("com::engine::Player", replacement);

    const Class *after = Class::findClass("com::engine::Player");
    check(after == before, "putClass preserva endereço");
    check(after != nullptr && after->getModifier().isPrivate(),
          "putClass atualizou modifier");
    check(after != nullptr && after->getSuperclasses().empty(),
          "putClass atualizou superclasses");
    check(after != nullptr && after->getDeclaredFields().empty(),
          "putClass atualizou fields");
  }

  // ============================================================
  // Resumo
  // ============================================================

  std::cout << "\n=== " << failures << " falha(s) ===\n";
  return failures == 0 ? 0 : 1;
}