#pragma once
#include "common/error.h"
#include "common/geometry.hpp"

namespace layout::item{

    class Item{
        private : 

            static map<string, function<Item *()>> & registry(){
                static map<string, function<Item *()>> impl;
                return impl;
            }

        public :
            string info;

            virtual py::object python() const = 0;
            virtual Box bbox() const = 0;
            virtual void unmap(const Local2D &region) = 0;

            virtual void insert1(int left, int right, int front, int end, int bottom, int top, int depth){}

            struct Registrar{
                Registrar (string const &name, function<Item *()> create){
                    Item::registry()[name] = create;
                }
            };

            static Item *create (string const &name) {
                auto it = registry().find(name);
                if (it == registry().end())
                    raiseError(UnknownError, "To live is suffer, to survive is to find some meaning in the suffering : " + name);
                return it->second();
            }
    };

}
