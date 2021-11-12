#pragma once
#include "common/common.h"

#define raiseError(error, info) \
    LOG(ERROR) << info, throw error(info)

#define raiseWarning(error, info) \
    LOG(WARNING) << info, layout::warning_list.emplace_back(new error(info))

namespace layout{

    class UnknownError : std::exception{
        protected :
            string name;
        public :
            explicit UnknownError (string const &_name) : name(_name) {}
            virtual const char * what () const noexcept override {return name.c_str();}
    };

    class StorageError : public UnknownError{
        public :
            explicit StorageError (string const &_name) : UnknownError(_name){}
            virtual const char * what() const noexcept override {return name.c_str();}
    };

    class ParameterError : public UnknownError{
        public :
            explicit ParameterError (const string &_name) : UnknownError("An error arise when updating parameter : " + _name){}
            virtual const char * what () const noexcept override {return name.c_str();}
    };

    class MethodError : public UnknownError{
        public :
            explicit MethodError (string const &_name) : UnknownError(_name) {}
            virtual const char * what () const noexcept override {return name.c_str();}
    };

    class InterfaceError : public UnknownError{
        public :
            explicit InterfaceError (string const &_name) : UnknownError("An error arise when interacting : " + _name) {}
            virtual const char * what () const noexcept override {return name.c_str();}
    };

    extern vector<unique_ptr<UnknownError>> warning_list;
    void errorDef(py::module m);
}
