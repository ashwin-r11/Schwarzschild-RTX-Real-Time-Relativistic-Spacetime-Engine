/**
 *  |+++++++++++++++++++++++++++|
 *  |   Linear Algebra Core     |
 *  |===========================|
 *  |        ~Written by:       |
 *  |         Ashwin on feb"2026|
 *  |===========================|
 */


#include <iostream>
#include <cmath>

struct vec3{

    //parameters
    double x,y,z;

    //Constructor    
    vec3(double _x=0, double _y=0, double _z=0): x(_x), y(_y), z(_z) {}

    //Operator Overloading
    inline vec3 operator+(const vec3& other) const{     //Addition
        return vec3(x+other.x, y+other.y, z+other.z);
    }

    inline vec3 operator-(const vec3& other) const{     //Subraction
        return vec3(x-other.x, y-other.y, z-other.z);
    }
    
    
    inline vec3 operator*(double s) const{              //Scalar Multiplication
        return vec3(x*s, y*s, z*s);
    }

    inline vec3 operator/(double s) const{              //Division
        
        double inv = 1.0/s;                             //div(10-40cc)>>>>>mul(2-3cc)
        return vec3(x*inv, y*inv, z*inv);
    }
    
    //Essential Functions

    inline double dot(const vec3& other)const{          //Dot Prod
        return (x*other.x+y*other.y+z*other.z);
    }
 
    inline vec3 cross(const vec3& other)const{          //Cross Prod
        double res_x = (y*other.z)-(z*other.y);
        double res_y = (z*other.x)-(x*other.z);
        double res_z = (x*other.y)-(y*other.x);

        return vec3(res_x,res_y,res_z);   
    }

    inline double length()const{                        //Vector length
        return(std::sqrt(x*x+y*y+z*z));
    }

    inline vec3 normalize() const{
        double l = 1/length();
        return vec3(x*l,y*l,z*l);
    }
};







