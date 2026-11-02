#ifndef VEC3_H //if note defined
#define VEC3_H  //creates the identifier

#include <cmath> //Ex std:sqrt
#include <iostream>  //?input and output stream utilities, std::cout

class vec3 { 
	public:
		float e[3];

		constexpr vec3() : e{0,0,0} {} // Member initializer list sets all 3 components to 0
		constexpr vec3(float e0, float e1, float e2) : e{e0,e1,e2} {}

		float x() const { return e[0]; }
		float y() const { return e[1]; }
		float z() const { return e[2]; }

		vec3 operator -() const { return vec3(-e[0], -e[1], -e[2]); } // Negates vector (-v)
		float operator[](int i) const { return e[i]; }
		float& operator[](int i) { return e[i]; } // Returns reference for write access, e.g. v[0] = 5.0;

		vec3& operator+=(const vec3& v) {
			e[0] += v.e[0];
			e[1] += v.e[1];
			e[2] += v.e[2];
			return *this; // Returns reference to current object so calls can be chained
		}

		vec3& operator*=(float t) {
			e[0] *= t;
			e[1] *= t;
			e[2] *= t;
			return *this;
		}

		vec3& operator/=(float t) {
			return *this *= (1.0 / t);
		}

		float length() const {
			return std::sqrt(length_squared());
		}

		float length_squared() const {
			return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
		}

		static const vec3 ONE;
};

// color is an alias for vec3 (geometric clarity)
using color = vec3;

// Vector Utility Functions (inline prevents duplicate symbol linker errors)

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
	return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
	return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
	return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
	return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(float t, const vec3& v) {
	return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline vec3 operator*(const vec3& v, float t) {
	return t * v;
}

inline vec3 operator/(const vec3& v, float t) {
	return (1/t) * v;
}

inline float dot(const vec3& u, const vec3& v) {
	return u.e[0] * v.e[0]
		+ u.e[1] * v.e[1]
		+ u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
	return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
			u.e[2] * v.e[0] - u.e[0] * v.e[2],
			u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 normalize(const vec3& v) {
	return v / v.length();
}

inline constexpr vec3 vec3::ONE{1.0f,1.0f,1.0f};

#endif
