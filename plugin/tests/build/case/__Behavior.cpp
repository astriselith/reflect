// Gerado automaticamente pelo plugin Clang 'reflect'
// Origem: Behavior.hpp

#include "Behavior.hpp"
#include "Class.hpp"
#include "Field.hpp"
#include "Method.hpp"
#include "Constructor.hpp"
#include "Destructor.hpp"
#include "Modifier.hpp"
#include "Parameter.hpp"
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

namespace com::engine {

class __Behavior {
public:
    static void __destructor_0(void* object) {
        delete reinterpret_cast<com::engine::Behavior*>(object);
    }

    static void* __constructor_0(const std::vector<void*>& args) {
        if (!args.empty()) throw std::invalid_argument("Wrong argument count");
        return static_cast<void*>(new com::engine::Behavior());
    }

};

} // namespace com::engine

extern "C" __attribute__((constructor)) void __register_com_engine_Behavior() {
    if (!Class::hasClass("com::engine::Behavior")) {
        Class::putClass("com::engine::Behavior", Class("com::engine::Behavior"));
    }
    const Class &self = Class::getClass("com::engine::Behavior");

    std::vector<const Class*> superclasses = {
    };

    std::vector<Field> fields = {
    };

    std::vector<Method> methods = {
    };

    std::vector<Constructor> constructors = {
        Constructor(&self, Modifier(Modifier::PUBLIC), {}, reinterpret_cast<uintptr_t>(&com::engine::__Behavior::__constructor_0)),
    };

    std::vector<Destructor> destructors = {
        Destructor(reinterpret_cast<uintptr_t>(&com::engine::__Behavior::__destructor_0)),
    };

    Class::putClass("com::engine::Behavior",
        Class("Behavior", Modifier(Modifier::PUBLIC), superclasses, fields, methods, constructors, "com::engine", destructors));
}

