#include "statement.h"
#include "object.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace Ast {

using Runtime::Closure;

ObjectHolder Assignment::Execute(Closure& closure) {
    return closure[var_name] = right_value->Execute(closure);
}

Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv) 
    : var_name(var)
    , right_value(move(rv))
{
}

VariableValue::VariableValue(std::string var_name) 
    : dotted_ids({ var_name }) 
{
}

VariableValue::VariableValue(std::vector<std::string> dotted_ids) 
    : dotted_ids(dotted_ids)
{
}

string VariableValue::GetFullName() {
    string result;
    for (auto id : dotted_ids) {
        if (!result.empty()) result += ".";
        result += id;
    }
    return result;
}

ObjectHolder VariableValue::Execute(Closure& closure) {
    auto name = GetFullName();
    if (!closure.count(name)) {
        throw runtime_error(("unknown var name: " + name).c_str());
    }
    return closure[name];
}

unique_ptr<Print> Print::Variable(std::string var) {
    return make_unique<Print>(Print(make_unique<VariableValue>(var)));
}

Print::Print(unique_ptr<Statement> argument) 
    : args({ argument })
{
}

Print::Print(vector<unique_ptr<Statement>> args)
    : args(args)
{
}

ObjectHolder Print::Execute(Closure& closure) {
    for (auto arg : args) {
        arg->Execute(closure)->Print(*output);
    }
    // check return value
    return ObjectHolder::None();
}

ostream* Print::output = &cout;

void Print::SetOutputStream(ostream& output_stream) {
  output = &output_stream;
}

MethodCall::MethodCall(
  std::unique_ptr<Statement> object
  , std::string method
  , std::vector<std::unique_ptr<Statement>> args
)
    : object(move(object))
    , method(method)
    , args(move(args))
{
}

ObjectHolder MethodCall::Execute(Closure& closure) {
    // todo
}

ObjectHolder Stringify::Execute(Closure& closure) {
    using namespace Runtime;
    auto val = argument->Execute(closure);
    if (val.TryAs<String>()) {
        return ObjectHolder::Own(Runtime::String(val.TryAs<String>()->GetValue()));
    }
    else if (val.TryAs<Number>()) {
        return ObjectHolder::Own(Runtime::String(to_string(val.TryAs<Number>()->GetValue())));
    }
    else if (!val) {
        return ObjectHolder::Own(Runtime::String("None"));
    }
    else if (val.TryAs<Bool>()) {
        if (val.TryAs<Bool>()->GetValue()) {
            return ObjectHolder::Own(Runtime::String("True"));
        } else {
            return ObjectHolder::Own(Runtime::String("False"));
        }
    }
    else if (val.TryAs<ClassInstance>()) {
        const auto ptr = val.TryAs<ClassInstance>();
        if (ptr->HasMethod("__str__")) {
            return ptr->Call("__str__", {});
        }
        else {
            ostringstream os;
            os << ptr;
            string addr = os.str();
            return ObjectHolder::Own(Runtime::String(addr));
        }
    }
    else {
        throw runtime_error("stringify error - undefined object");
    }
}

ObjectHolder Add::Execute(Closure& closure) {
    using namespace Runtime;
    auto tl = lhs->Execute(closure);
    auto tr = rhs->Execute(closure);
    if (tl.TryAs<Number>() && tr.TryAs<Number>()) {
        return ObjectHolder::Own(Runtime::Number(
            tl.TryAs<Number>()->GetValue() +
            tr.TryAs<Number>()->GetValue()
        ));
    }
    if (tl.TryAs<String>() && tr.TryAs<String>()) {
        return ObjectHolder::Own(Runtime::String(
            tl.TryAs<String>()->GetValue() +
            tr.TryAs<String>()->GetValue()
        ));
    }
    throw runtime_error("Incorrect addition");
}

ObjectHolder Sub::Execute(Closure& closure) {
    using namespace Runtime;
    auto tl = lhs->Execute(closure);
    auto tr = rhs->Execute(closure);
    if (tl.TryAs<Number>() && tr.TryAs<Number>()) {
        return ObjectHolder::Own(Runtime::Number(
            tl.TryAs<Number>()->GetValue() -
            tr.TryAs<Number>()->GetValue()
        ));
    }
    throw runtime_error("Incorrect substraction");
}

ObjectHolder Mult::Execute(Runtime::Closure& closure) {
    using namespace Runtime;
    auto tl = lhs->Execute(closure);
    auto tr = rhs->Execute(closure);
    if (tl.TryAs<Number>() && tr.TryAs<Number>()) {
        return ObjectHolder::Own(Runtime::Number(
            tl.TryAs<Number>()->GetValue() *
            tr.TryAs<Number>()->GetValue()
        ));
    }
    throw runtime_error("Incorrect multiplication");
}

ObjectHolder Div::Execute(Runtime::Closure& closure) {
    using namespace Runtime;
    auto tl = lhs->Execute(closure);
    auto tr = rhs->Execute(closure);
    if (tl.TryAs<Number>() && tr.TryAs<Number>()) {
        if (tr.TryAs<Number>()->GetValue() == 0) {
            throw runtime_error("Division by zero");
        }
        return ObjectHolder::Own(Runtime::Number(
            tl.TryAs<Number>()->GetValue() /
            tr.TryAs<Number>()->GetValue()
        ));
    }
    throw runtime_error("Incorrect division");
}

ObjectHolder Compound::Execute(Closure& closure) {
    for (const auto& element : statements) {
        element->Execute(closure);
    }
    return ObjectHolder::None();
}

ObjectHolder Return::Execute(Closure& closure) {
    // check
    return statement->Execute(closure);
}

ClassDefinition::ClassDefinition(ObjectHolder class_) 
    : cls(class_)
    , class_name(class_.TryAs<Runtime::Class>()->GetName())
{
}

ObjectHolder ClassDefinition::Execute(Runtime::Closure& closure) {
    // check
    return ObjectHolder::None();
}

FieldAssignment::FieldAssignment(
  VariableValue object, std::string field_name, std::unique_ptr<Statement> rv
)
  : object(std::move(object))
  , field_name(std::move(field_name))
  , right_value(std::move(rv))
{
}

ObjectHolder FieldAssignment::Execute(Runtime::Closure& closure) {
    auto full_name = object.GetFullName() + "." + field_name;
    return closure[full_name] = right_value->Execute();
}

IfElse::IfElse(
  std::unique_ptr<Statement> condition,
  std::unique_ptr<Statement> if_body,
  std::unique_ptr<Statement> else_body
)
    : condition(move(condition))
    , if_body(move(if_body))
    , else_body(move(else_body))
{
}

ObjectHolder IfElse::Execute(Runtime::Closure& closure) {
    auto result = condition->Execute(closure);
    if (Runtime::IsTrue(result)) {
        return if_body->Execute(closure);
    }
    else {
        return else_body->Execute(closure);
    }
}

ObjectHolder Or::Execute(Runtime::Closure& closure) {
    using namespace Runtime;
    auto tl = lhs->Execute(closure);
    if (IsTrue(tl)) {
        return ObjectHolder::Own(Bool(true));
    }
    auto tr = rhs->Execute(closure);
    return ObjectHolder::Own(Bool(IsTrue(tr)));
}

ObjectHolder And::Execute(Runtime::Closure& closure) {
    using namespace Runtime;
    auto tl = lhs->Execute(closure);
    if (!IsTrue(tl)) {
        return ObjectHolder::Own(Bool(false));
    }
    auto tr = rhs->Execute(closure);
    return ObjectHolder::Own(Bool(IsTrue(tr)));
}

ObjectHolder Not::Execute(Runtime::Closure& closure) {
    using namespace Runtime;
    auto val = argument->Execute(closure);
    return ObjectHolder::Own(Bool(!IsTrue(val)));
}

Comparison::Comparison(
  Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs
)
    : comparator(cmp)
    , left(move(lhs))
    , right(move(rhs))
{
}

ObjectHolder Comparison::Execute(Runtime::Closure& closure) {
    auto tl = left->Execute(closure);
    auto tr = right->Execute(closure);
    return ObjectHolder::Own(Runtime::Bool(comparator(tl, tr)));
}

NewInstance::NewInstance(
  const Runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args
)
  : class_(class_)
  , args(std::move(args))
{
}

NewInstance::NewInstance(const Runtime::Class& class_) : NewInstance(class_, {}) {
}

ObjectHolder NewInstance::Execute(Runtime::Closure& closure) {
    return ObjectHolder::Own(Runtime::ClassInstance(class_));
}


} /* namespace Ast */
