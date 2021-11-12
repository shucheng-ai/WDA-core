#include "common/geometry.hpp"

using namespace layout;

py::object pointTest(){
    py::dict ret;
    Point t1 = pyPoint{10, 10};
    Point t2{1, 2};
    Point t3 = t1 + t2;
    ret["2Dadd"] = (pyPoint)t3;
    ret["2Dminus"] = (pyPoint)Point(t3 - t1);

    Point3D t4({10, 10, 10});
    Point3D t5({1, 2, 3});
    Point3D t6 = t4 + t5;
    ret["3Dadd"] = (pyPoint3D)t6;
    ret["3Dminus"] = (pyPoint3D)Point3D(abs(t5 - t4));

    return ret;
}

py::object boxTest(){
    py::dict ret;
    Box t1 = pyBox{{0, 0}, {1000, 1000}};
    Box t2{{0, 0}, {-1000, -1000}};
    ret["size"] = (pyPoint)t1.size();
    ret["empty"] = !t1.empty() && t2.empty();
    Box t3{{900, 900}, {1000, 1000}};
    Box t4{{1000, 1000}, {1100, 1100}};
    ret["overlap"] = t1.overlap(t3) && !t1.overlap(t4);
    ret["contain"] = t1.contain({500,500}) && !t1.contain({-500, -500});
    t2 = t2.abs();
    ret["abs"] = (pyBox)t2;
    t1.expand(t2); ret["expand"] = (pyBox)t1;
    t1.shrink(t2); ret["shrink"] = (pyBox)t1;
    Box3D t5{{0,0,0} , {100,100,100}};
    ret["3D"] = (pyBox3D)t5.drop().lift(1,6);
    return ret;
}

py::object regionTest(){
    Matrix right_rotation{0, 1, -1, 0};
    Matrix left_rotation{0, -1, 1, 0};
    Matrix mirror{-1, 0, 0, 1};

    Box region_box{{10, 10}, {100, 200}};
    Local2D region(region_box); 
    py::dict ret;

    ret["size1"] = (pyPoint)region.unsize();
    Point temp = region.fmap(Point{10, 20});
    ret["fpoint1"] = (pyPoint)temp;
    ret["unpoint1"] = (pyPoint)region.unmap(temp);

    region.a = left_rotation * region.a; region.a_ = region.a_ * right_rotation;
    ret["size2"] = (pyPoint)region.unsize();
    ret["fsize2"] = (pyPoint)region.fsize();
    temp = region.fmap(Point{10, 20});
    ret["fpoint2"] = (pyPoint)temp;
    ret["unpoint2"] = (pyPoint)region.unmap(temp);
    
    region.a = left_rotation * region.a; region.a_ = region.a_ * right_rotation;
    ret["size3"] = (pyPoint)region.unsize();
    ret["fsize3"] = (pyPoint)region.fsize();
    temp = region.fmap(Point{10, 20});
    ret["fpoint3"] = (pyPoint)temp;
    ret["unpoint3"] = (pyPoint)region.unmap(temp);
    
    region.a = left_rotation * region.a; region.a_ = region.a_ * right_rotation;
    ret["size4"] = (pyPoint)region.unsize();
    ret["fsize4"] = (pyPoint)region.fsize();
    temp = region.fmap(Point{10, 20});
    ret["fpoint4"] = (pyPoint)temp;
    ret["unpoint4"] = (pyPoint)region.unmap(temp);
    
    region.a = mirror * region.a; region.a_ = region.a_ * mirror;
    ret["size5"] = (pyPoint)region.unsize();
    ret["fsize5"] = (pyPoint)region.fsize();
    temp = region.fmap(Point{10, 20});
    ret["fpoint5"] = (pyPoint)temp;
    ret["unpoint5"] = (pyPoint)region.unmap(temp);

    Box3D temp6 = region.fmap(Box3D{{10, 20, 100}, {20, 30, 200}});
    ret["fbox"] = (pyBox3D)temp6;
    ret["unbox"] = (pyBox3D)region.unmap(temp6);

    pyBox3D &temp7 = (pyBox3D&)temp6;
    pyBox3D &temp8 = reinterpret_cast<pyBox3D&>(temp6);

    return ret;
}

PYBIND11_MODULE(test_geometry, m){
    m.doc() = "test geometry.hpp";
    m.def("pointTest", &pointTest, "");
    m.def("boxTest", &boxTest, "");
    m.def("regionTest", &regionTest, "");
}
