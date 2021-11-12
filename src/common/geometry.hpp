#pragma once
#include "common.h"

namespace layout{

    template <class T> struct Exp{
        inline const T& self () const {
            return *static_cast<const T*>(this);
        }
    };

    template <int D> struct Size_ : Exp<Size_<D>> , array<int, D>{

        Size_ () {}

        template<class T> Size_ (const Exp<T>& src_) {
            const T& src = src_.self();
            for (int i = 0; i < D; i++) this->at(i) = src[i];
        }

        Size_ (std::initializer_list<int> const &x) {
            std::memcpy(this->data(), x.begin(), D * sizeof(int));
        }

        template<int D1> Size_<D>&operator = (Size_<D1> const &x){
            int DD = std::min(D1, D);
            std::memcpy(this->data(), x.data(), DD * sizeof(int));
        }

        Size_ (array<int, D> const& x) : array<int, D>(x) {}
    };

    template <class L, class R> struct AddExp : Exp<AddExp<L, R>>{
        private : 
            const L &lvalue;
            const R &rvalue;

        public :

            AddExp (const L& _lvalue, const R& _rvalue) : lvalue(_lvalue) , rvalue(_rvalue){}

            inline int operator [] (int i) const{
                return lvalue[i] + rvalue[i];
            }
    };

    template <class L, class R> struct SubExp : Exp<SubExp<L, R>>{
        private : 
            const L &lvalue;
            const R &rvalue;
        public :

            SubExp (const L& _lvalue, const R& _rvalue) : lvalue(_lvalue), rvalue(_rvalue){}

            inline int operator [] (int i) const{
                return lvalue[i] - rvalue[i];
            }
    };

    template <class T> struct AbsExp : Exp<AbsExp<T>>{
        private :
            const T &value;

        public :

            AbsExp (const T &_value) : value(_value) {}

            inline int operator [] (int i) const{
                return std::abs(value[i]);
            }
    };

    template <class T> inline AbsExp<T> abs (const Exp<T> &value){
        return AbsExp<T>(value.self());
    }

    template <class L, class R> inline AddExp<L, R> operator + (const Exp<L>& lvalue, const Exp<R>& rvalue){
        return AddExp<L, R>(lvalue.self(), rvalue.self());
    }

    template <class L, class R> inline SubExp<L, R> operator - (const Exp<L>& lvalue, const Exp<R>& rvalue){
        return SubExp<L, R>(lvalue.self(), rvalue.self());
    }

    using Point = Size_<2>;
    using pyPoint = array<int, 2>;
    using Point3D = Size_<3>;
    using pyPoint3D = array<int, 3>;

    inline long long cross2D (const Point &x, const Point &y){
        return (long long)x[0] * y[1] - (long long)x[1] * y[0];
    }

    template <int D> struct Box_: pair<Size_<D>, Size_<D>> {
        using pair<Size_<D>, Size_<D>>::first;
        using pair<Size_<D>, Size_<D>>::second;

        Box_ () {
            std::fill(first.begin(), first.end(), numeric_limits<int>::max() >> 1);
            std::fill(second.begin(), second.end(), numeric_limits<int>::min() / 2);
        }

        template <int D2> Box_ (Box_<D2> const &from) : Box_(){
            for (int i = 0; i < std::min(D, D2); ++i) {
                first[i] = from.first[i];
                second[i] = from.second[i];
            }
        }

        inline Size_<D> size () const {
            return second - first;
        }

        bool empty () const {
            for (int i = 0; i < D; ++i) {
                if (first[i] >= second[i]) return 1;
            }
            return 0;
        }

        bool overlap (Box_<D> const &box) const {
            bool flag = 1;
            for (int i = 0; i < D; ++i)
                flag &= std::max(first[i], box.first[i]) < std::min(second[i], box.second[i]);
            return flag;
        }

        inline bool contain(Size_<D> const &p) const{
            bool flag = 1;
            for (int i = 0; i < D; ++i)
                flag &= first[i] <= p[i] && p[i] <= second[i];
            return flag;
        }

        void expand (Size_<D> const &p) {
            for (int i = 0; i < D; ++i) {
                first[i] = std::min(p[i], first[i]);
                second[i] = std::max(p[i], second[i]);
            }
        }

        void expand (Box_<D> const &box) {
            if (box.empty()) return;
            expand(box.first); expand(box.second);
        }

        void shrink (Box_<D> const &box){
            for (int i = 0; i < D; ++i) {
                first[i] = std::max(box.first[i], first[i]);
                second[i] = std::min(box.second[i], second[i]);
            }
        }

        Box_ (Size_<D> const &p1, Size_<D> const &p2) {
            first = p1; second = p2;
        }

        Box_ (pair<array<int, D>, array<int, D>> const &x){
            first = x.first; second = x.second;
        }

        Box_<D+1> lift (int zmin, int zmax) const {
            Box_<D+1> box(*this);
            box.first[D] = zmin; box.second[D] = zmax;
            return box;
        }

        Box_ (py::list points) : Box_(){
            for (auto _point : points) expand(_point.cast<array<int, D>>());
        }

        inline Box_<D-1> drop () const {
            return Box_<D-1>(*this);
        }

        Box_<D-1> drop (int d) const {
            Box_<D-1> b;
            for (int i = 0, j = 0; i < D; ++i) {
                if (i != d) {
                    b.first[j] = first[i];
                    b.second[j] = second[i];
                    ++j;
                }
            }
            return b;
        }

        Box_<D> abs () const{
            Box_<D> ret = *this;
            for (int i = 0; i < D; i++)
                if (ret.first[i] > ret.second[i]) std::swap(ret.first[i], ret.second[i]);
            return ret;
        }

        inline Size_<2> bl () const {
            return Size_<2>{first[0], first[1]};
        }

        inline Size_<2> br () const {
            return Size_<2>{second[0], first[1]};
        }

        inline Size_<2> tl () const {
            return Size_<2>{first[0], second[1]};
        }

        inline Size_<2> tr () const {
            return Size_<2>{second[0], second[1]};
        }

    };

    using Box = Box_<2>;
    using pyBox = pair<pyPoint, pyPoint>;
    using Box3D = Box_<3>;
    using pyBox3D = pair<pyPoint3D, pyPoint3D>;

    inline double yFromLineX(Point const &x, Point const &y, int pos){
        double prop = double(pos - x[0]) / (y[0] - x[0]);
        return x[1] * (1 - prop) + y[1] * prop;
    }

    using Matrix = array<array<int, 2>, 2>;

    static Matrix operator * (const Matrix &x, const Matrix &y){
        Matrix ret{0, 0, 0, 0};
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
                for (int k = 0; k < 2; k++)
                    ret[i][j] += x[i][k] * y[k][j];
        return ret;
    }

    static Size_<2> operator * (const Matrix &x, const Size_<2> &y){
        Size_<2> ret{0, 0};
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
                ret[i] += x[i][j] * y[j];
        return ret;
    }

    //坐标转换的目标必须是非空的
    struct Local2D{

        Box box = {{0, 0}, {0, 0}};
        Matrix a{1, 0, 0, 1}, a_{1, 0, 0, 1};

        Local2D () {}
        Local2D (Box _region) : box(_region) {}

        inline Size_<2> unsize() const {
            return box.size();
        }

        inline Size_<2> fsize () const{
            return abs(a * box.size());
        }

        template<int D> Size_<D> fmap (Size_<D> p) const{
            p = a * (p - box.first);
            Size_<2> my_size = a * unsize();
            if (my_size[0] < 0) p[0] -= my_size[0];
            if (my_size[1] < 0) p[1] -= my_size[1];
            return p;
        }

        template<int D> Size_<D> unmap (Size_<D> p) const {
            Size_<2> my_size = a * unsize();
            if (my_size[0] < 0) p[0] += my_size[0];
            if (my_size[1] < 0) p[1] += my_size[1];
            return p = Point(box.first + a_ * p);
        }

        template<int D> Box_<D> fmap (Box_<D> b) const{
            if (b.empty()) return b;
            b.first = fmap(b.first);
            b.second = fmap(b.second);
            return b.abs();
        }

        template<int D> Box_<D> unmap (Box_<D> b) const{
            if (b.empty()) return b;
            b.first = unmap(b.first);
            b.second = unmap(b.second);
            return b.abs();
        }
     };

    struct RecursiveBox3D : Box3D, vector<RecursiveBox3D>{
        using vector<RecursiveBox3D>::size;
        using vector<RecursiveBox3D>::empty;

        RecursiveBox3D () {}
        RecursiveBox3D (Box3D const &x) : Box3D(x) {}
        
        void update (){
            if (!size()) return;
            for (auto &ch : *this){
                ch.update(); expand(ch); 
            }
        }

        void fmap (Local2D const &region){
            *static_cast<Box3D*>(this) = region.fmap(*this);
            for (auto &ch : *this) ch.fmap(region); 
        }

        void unmap (Local2D const &region){
            *static_cast<Box3D*>(this) = region.unmap(*this);
            for (auto &ch : *this) ch.unmap(region);
        }

        py::object python () const{
            py::dict ret;
            ret["box"] = reinterpret_cast<const pyBox3D&>(*static_cast<const Box3D*>(this));
            if (!size()) return ret;
            py::list children;
            for (auto const &ch : *this)
                children.append(ch.python());
            ret["children"] = children;
            return ret;
        }
    };

}

