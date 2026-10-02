#include "Parameter.hpp"

Parameter::Parameter() : modifier_(0), type_(nullptr), name_("") {}

Parameter::Parameter(std::string name, const Class *type, Modifier modifier)
    : modifier_(modifier), type_(type), name_(std::move(name)) {}

Modifier Parameter::getModifier() const { return modifier_; }
const Class *Parameter::getType() const { return type_; }
const std::string &Parameter::getName() const { return name_; }