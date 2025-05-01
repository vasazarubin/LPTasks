#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <assert.h>

class BaseObject
{
    size_t objectID;
public:
    BaseObject();
    size_t getID() const;
    virtual ~BaseObject();
};

enum class VarType
{
    Binary,
    Int,
    Real
};

class BaseVariable : public BaseObject
{
    VarType _type;
    std::string _name;
public:
    const std::string& getVarName() const;
    VarType getType() const;
};

class BinaryVariable : public BaseVariable
{
    bool _value;
public:
    BinaryVariable(const std::string& name, bool value);
    bool getValue();
};

class IntVariable : public BaseVariable
{
    int _value;
public:
    IntVariable(const std::string& name, int value);
    int getValue();
};

class RealVariable : public BaseVariable
{
    double _value;
public:
    RealVariable(const std::string& name, double value);
};

class BaseTask : public BaseObject
{
    size_t _taskID;
    const std::vector<BaseVariable> _variables;
    size_t _order;
public:
    size_t getTaskOrder() const;
    size_t getTaskID() const;
};

class Function;
using TaskMapping = std::unordered_map<int, std::unique_ptr<BaseTask>>; //TaskID, Task
using VariableMapping = std::unordered_map<std::string, std::unique_ptr<BaseVariable>>; //VariableName, Variable
using FunctionMapping = std::unordered_map<std::string, std::unique_ptr<Function>>; //FunctionName, Function

class Function : public BaseObject
{
    std::string _name;
public:
    Function(const std::string& name, const std::string& definition);
    virtual double Evaluate(const VariableMapping& varMap, const FunctionMapping& funcMap) const;
    ~Function();
};

class BaseConstraint : public BaseObject
{
public:
    BaseConstraint();
    virtual bool CheckConstraint(const TaskMapping& tasks) const = 0;
    ~BaseConstraint();
};

class TaskOrderConstraint : public BaseConstraint
{
    size_t _taskID1, _taskID2;
public:
    TaskOrderConstraint(size_t id1, size_t id2);
    bool CheckConstraint(const TaskMapping& tasks) const override;
};

enum class OptimizationType
{
    Minimum,
    Maximum
};

class Objective : public Function
{
    OptimizationType _optimType;
public:
    virtual double Evaluate(const VariableMapping& varMap, const FunctionMapping& funcMap) const override;
};

class BaseOptimizationAlgorithm : public BaseObject
{
public:
    virtual void Optimize(OptimizationType type) const = 0;
    ~BaseOptimizationAlgorithm();
};

class SyntaxParser
{
    std::ifstream _inputModel;
public:
    SyntaxParser(const std::string &filename);
    BaseObject* ObjectFabricGetNext() const;
    ~SyntaxParser();
};

class Planner
{
    std::unique_ptr<Objective> _objective;
    std::unique_ptr<BaseOptimizationAlgorithm> _optimizationAlgorithm;
    std::vector<std::unique_ptr<BaseConstraint>> _constraints;
    TaskMapping _tasks;
    FunctionMapping _functions;
    VariableMapping _variables;
public:
    Planner();
    void AddFunction(Function* function); 
    void AddTask(BaseTask* task);
    void AddConstraint(BaseConstraint *constraint);
    void SetSolver(BaseOptimizationAlgorithm* solver);
    void SetObjective(Objective* objective);
    void Solve();
    void SaveResult(const std::string &filename) const;
};

int main(int argc, char *argv)
{
    SyntaxParser parser("InFilename");
    Planner planner;

    while (auto *object = parser.ObjectFabricGetNext())
    {
        if (auto* task = dynamic_cast<BaseTask*>(object))
            planner.AddTask(task);
        else if (auto* solver = dynamic_cast<BaseOptimizationAlgorithm*>(object))
            planner.SetSolver(solver);
        else if (auto* objective = dynamic_cast<Objective*>(object))
            planner.SetObjective(objective);
        else if (auto* function = dynamic_cast<Function*>(object))
            planner.AddFunction(function);
        else if (auto* constraint = dynamic_cast<BaseConstraint*>(object))
            planner.AddConstraint(constraint);
        else 
            assert(false);
    }

    planner.Solve();
    planner.SaveResult("OutFilename");

    return 0;
}