#include "item.h"

namespace layout::item{

    using UprightPair = pair<Box3D, Box3D>;
    using pyUprightPair = pair<pyBox3D, pyBox3D>;

    struct Upright : Item, vector<UprightPair>{

        Upright () {info = "upright";}
        virtual ~Upright (){}

        virtual py::object python () const override{
            py::list ret;
            for (auto const &it : *this) ret.append(reinterpret_cast<pyUprightPair const &>(it));
            return ret;
        }

        virtual Box bbox () const override{
            Box3D ret;
            for (auto const &pu : *this){
                ret.expand(pu.first); ret.expand(pu.second);
            }
            return ret;
        }

        virtual void unmap (const Local2D &region) override{
            for (auto &it : *this){
                it.first = region.unmap(it.first);
                it.second = region.unmap(it.second);
            }
        }

        virtual void insert1 (int left, int right, int front, int end, int bottom, int top, int depth) override{
            emplace_back(Box3D{{left, front, bottom}, {left + depth, end, top}}, Box3D{{right - depth, front, bottom}, {right, end, top}});
        }
    };

    namespace{
        Item::Registrar _1("upright", []{return new Upright();});
    }
}
