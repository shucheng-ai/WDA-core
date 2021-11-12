#pragma once
#include "common/common.h"

namespace layout::configurable{

    struct Variable{
        const static int constexpr INVALID = numeric_limits<int>::min();
        string name, description;

        virtual py::dict python() const;
        virtual int &asInt() = 0;
        virtual string &asString() = 0;
        virtual void update(py::dict);

        virtual bool check() const{return 1;}

        Variable(string _name, const string &_description);
        virtual ~Variable () {}
    };

    struct IntVariable : Variable{
        int *value;
        virtual py::dict python() const override;
        virtual int &asInt();
        virtual string &asString() override {assert(0);}
        virtual void update(py::dict) override; 

        IntVariable(string _name, int *_value, const string &_description);
        virtual ~IntVariable () {}
    };

    struct RangedIntVariable : IntVariable{
        pi range;
        virtual py::dict python() const override;
        virtual bool check() const override;
        virtual void update(py::dict) override;

        RangedIntVariable(string _name, int *_value, pi _range, const string &_description);
        virtual ~RangedIntVariable () {}
    };

    struct AlterIntVariable : RangedIntVariable{
        vector<int> choices;
        virtual py::dict python() const override;
        virtual bool check() const override;
        virtual void update(py::dict) override;

        AlterIntVariable(string _name, int *_value, pi _range, const vector<int>& _choices, const string &_description);
        virtual ~AlterIntVariable () {}
    };

    struct StrVariable : Variable{
        string *value;
        virtual py::dict python() const;
        virtual int &asInt() override {assert(0);}
        virtual string &asString();
        virtual void update(py::dict) override;

        StrVariable(string _name, string *_value, const string &_description);
        virtual ~StrVariable () {}
    };

    struct AlterStrVariable : StrVariable{
        vector<string> choices;
        virtual py::dict python() const;
        virtual bool check() const override;
        virtual void update(py::dict) override;

        AlterStrVariable(string _name, string *_value, const vector<string>& _choices, const string &_description);
        virtual ~AlterStrVariable () {}
    };

    class Configurable : public map<string,unique_ptr<Variable>>{
        private :
            vector<string> order;
        public :
            py::object python() const;
            void update(py::dict params);

            void registeR(string const &name, int *value, string const &description = "");
            void registeR(string const &name, int *value, pi range, string const &description = "");
            void registeR(string const &name, int *value, pi range, vector<int> const &choices, string const &description = "");
            void registeR(string const &name, string *value, string const &description = "");
            void registeR(string const &name, string *value, vector<string> const &choice, string const &description = "");
    };

}
