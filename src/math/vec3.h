#ifndef VEC3_H //if note defined
#define VEC3_H  //creates the identifier

#include <cmath> //Ex std:sqrt
#include <iostream>  //?input and output stream utilities, std::cout

class vec3 { 
	public:
		double e[3];

		vec3() : e {0,0,0} {} //  member initializer list?
		vec3(double e0, double e1, double e2) : e{e0,e1,e2} {}

		double x() const { return e[0]; }
		double y() const { return e[1]; }
		double z() const { return e[2]; }


		vec3 operator -() const { return vec3(-e[0], -e[1], -e[2]); } // -negates the vector, inverts it
		double operator[](int i) const { return e[i];}
		double& operator[](int i) {retunr e[i]; } // retrns a refernce, & allows write access, ex v[0] = 5.0;
		vec3& operator+=(const vec3& v) { //vec3& ?
			e[0] += v.e[0]; //this.e[0] += v.e[0]
			e[1] += v.e[1];
			e[2] += v.e[2];
			return *this; //*this? * dereferences it, this is the e[0],e[1] 
		}

		vec3& operator*=(double t) { //why is this only *= but the last one is different
			e[0] *= t;
			e[1] *= t;
			e[2] *= t;
			return *this;
		}

		vec3& operator/=(double t) {
			return *this *= 1/t; //multiplyin the reciprical
		}

		double length() const {
			return std::sqrt(length_squared()); //std = standard, as in standard library
		}

		double length_squared() const {
			return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
		}
};

// point3 is just an alias for vec3, but useful for geometric clarity in the code.
using point3 = vec3;


// Vector Utility Functions

inline std::ostream& operator<<(std::ostream& out, const vec3& v) { //again std::ostream& what does that mean
	return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2]; //std::cout << my_vec; == print(myvec);
} //formated like this so its x_y_z with the spaces

inline vec3 operator+(const vec3& u, const vec3& v) {
	return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
} //inline saves duplicate code I think? like in the compiler??

inline vec3 operator-(const vec3& u, const vec3& v) {
	return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
	return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(double t, const vec3& v) { //what is the inline flag
	return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline vec3 operator*(const vec3& v, double t) {
	return t * v;
}

inline vec3 operator/(const vec3& v, double t) {
	return (1/t) * v;
}

inline double dot(const vec3& u, const vec3& v) {
	return u.e[0] * v.e[0]
		+ u.e[1] * v.e[1]
		+ u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
	return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
			u.e[2] * v.e[0] - u.e[0] * v.e[2],
			u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(const vec3& v) {
	return v / v.length();
}

#endif
