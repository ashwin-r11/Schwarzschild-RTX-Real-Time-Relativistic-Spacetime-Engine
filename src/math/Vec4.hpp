/**
 *  |+++++++++++++++++++++++++++|
 *  |   Linear Algebra Core     |
 *  |===========================|
 *  |        ~Written by:       |
 *  |         Ashwin on feb"2026|
 *  |===========================|
 */

#pragma once

#include <iostream>
#include <cmath>


struct vec4 
{
    double x,y,z,w;

    vec4(double _x=0, double _y=0, double _z=0, double _w=1): x(_x), y(_y), z(_z), w(_w) {}

    inline vec4 operator+(const vec4& other) const{
        return vec4(x+other.x, y+other.y, z+other.z, w+other.w);
    }

    inline vec4 operator-(const vec4& other) const{
        return vec4(x-other.x, y-other.y, z-other.z, w-other.w);
    }

    inline vec4 operator*(double s) const{
        return vec4(x*s, y*s, z*s, w*s);
    }

    inline vec4 operator/(double s) const{
        double inv = 1.0/s;
        return vec4(x*inv, y*inv, z*inv, w*inv);
    }

    inline vec4 normalize() const{
        double l = 1.0/length();
        return vec4(x*l, y*l, z*l, w*l);
    }

    inline vec4 cross(const vec4& other) const{
        double res_x = (y*other.z)-(z*other.y);
        double res_y = (z*other.x)-(x*other.z);
        double res_z = (x*other.y)-(y*other.x);
        double res_w = 0; 

        return vec4(res_x, res_y, res_z, res_w);
    }

    inline double dot(const vec4& other) const{
        return (x*other.x + y*other.y + z*other.z + w*other.w);
    }

    inline  double length() const{
        return std::sqrt(x*x + y*y + z*z + w*w);
    }


};