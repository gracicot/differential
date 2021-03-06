#pragma once

#ifndef GRACICOT_DIFFENTIAL_HPP
#define GRACICOT_DIFFENTIAL_HPP

#include <type_traits>
#include <cmath>
#include <tuple>

namespace diff {

/////////////////////////////////////////////
//    Base class to tag class category     //
/////////////////////////////////////////////
struct Expr { constexpr Expr() = default; };
struct AnyVariable : Expr { using Expr::Expr; };
struct AnyConstant : Expr { using Expr::Expr; };

/////////////////////////////////////////////
//             SFINAE utilities            //
/////////////////////////////////////////////
template<typename Condition>
using enable_if = typename std::enable_if<Condition::value, int>::type;

template<bool b>
using bool_constant = std::integral_constant<bool, b>;

template<typename Var>
using is_variable = std::is_base_of<AnyVariable, Var>;

template<typename E>
using is_expr = std::is_base_of<Expr, E>;

template<typename E>
using is_constant = std::is_base_of<AnyConstant, E>;

template<typename E>
using is_expr_not_constant = bool_constant<is_expr<E>::value && !is_constant<E>::value>;

/////////////////////////////////////////////
//            Expression utility           //
/////////////////////////////////////////////
template<int n, typename T, enable_if<bool_constant<n == 1>> = 0>
constexpr inline auto power(T num) {
	return num;
}

template<int n, typename T, enable_if<bool_constant<(n > 1)>> = 0>
constexpr inline auto power(T num) {
	return num * power<n - 1>(num);
}

template<typename T>
constexpr inline auto square(T num) {
	return power<2>(num);
}

template<typename T>
constexpr inline auto cube(T num) {
	return power<3>(num);
}

template<typename E, typename... Args, enable_if<is_expr<E>> = 0>
constexpr double eval(E e, Args... args) {
	return e(args...);
}

template<typename Var, std::size_t level = 1, typename E, enable_if<bool_constant<level == 1>> = 0, enable_if<is_variable<Var>> = 0>
constexpr auto derivative(E e) {
	return e.template derivative<Var>();
}

template<typename Var, std::size_t level, typename E, enable_if<bool_constant<(level > 1)>> = 0, enable_if<is_variable<Var>> = 0>
constexpr auto derivative(E e) {
	return derivative<Var>(derivative<Var, level - 1>(e));
}

/////////////////////////////////////////////
//            Expression helpers           //
/////////////////////////////////////////////
template<typename E1, typename E2>
struct BinaryExpr : Expr {
	constexpr explicit BinaryExpr(E1 e1, E2 e2) : _e1{e1}, _e2{e2} {}
	
protected:
	E1 _e1;
	E2 _e2;
};

template<typename E>
struct UnaryExpr : Expr {
	constexpr explicit UnaryExpr(E e) : _e{e} {}
	
protected:
	E _e;
};

/////////////////////////////////////////////
//          Expression definitions         //
/////////////////////////////////////////////

// Represent a constant integer known at compilation
template<int val>
struct Constant : AnyConstant {
	using AnyConstant::AnyConstant;
	
	constexpr static int value = val;
	
	template<typename... Args>
	constexpr double operator()(Args...) const {
		return value;
	}
	
	template<typename Var>
	constexpr auto derivative() const {
		return Constant<0>{};
	}
};

// Represent a constant value that may be only known at runtime
struct Value : Expr {
	constexpr explicit Value(double value) : _value{value} {}
	
	template<typename... Args>
	constexpr double operator()(Args...) const {
		return _value;
	}
	
	template<typename Var, enable_if<is_variable<Var>> = 0>
	constexpr auto derivative() const {
		return Constant<0>{};
	}
	
private:
	double _value;
};

// Represent a varible in the expression
template<int n>
struct Variable : AnyVariable {
	using AnyVariable::AnyVariable;
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return std::get<n - 1>(std::make_tuple(args...));
	}
	
	template<typename Var, enable_if<is_variable<Var>> = 0>
	constexpr auto derivative() const {
		return Constant<std::is_same<Var, Variable<n>>::value ? 1:0>{};
	}
};

// Represent a multiplication expression
template<typename E1, typename E2>
struct Multiplication : BinaryExpr<E1, E2> {
	using BinaryExpr<E1, E2>::BinaryExpr;
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return this->_e1(args...) * this->_e2(args...);
	}
	
	template<typename Var, enable_if<is_variable<Var>> = 0>
	constexpr auto derivative() const {
		return this->_e1 * diff::derivative<Var>(this->_e2) + diff::derivative<Var>(this->_e1) * this->_e2;
	}
};

// Represent an addition expression
template<typename E1, typename E2>
struct Addition : BinaryExpr<E1, E2> {
	using BinaryExpr<E1, E2>::BinaryExpr;
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return this->_e1(args...) + this->_e2(args...);
	}
	
	template<typename Var, enable_if<is_variable<Var>> = 0>
	constexpr auto derivative() const {
		return diff::derivative<Var>(this->_e1) + diff::derivative<Var>(this->_e2);
	}
};

// Represent a substraction expression
template<typename E1, typename E2>
struct Substraction : BinaryExpr<E1, E2> {
	using BinaryExpr<E1, E2>::BinaryExpr;
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return this->_e1(args...) - this->_e2(args...);
	}
	
	template<typename Var, enable_if<is_variable<Var>> = 0>
	constexpr auto derivative() const {
		return diff::derivative<Var>(this->_e1) - diff::derivative<Var>(this->_e2);
	}
};

// Represent a division expression
template<typename E1, typename E2>
struct Division : BinaryExpr<E1, E2> {
	using BinaryExpr<E1, E2>::BinaryExpr;
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return this->_e1(args...) / this->_e2(args...);
	}
	
	template<typename Var, enable_if<is_variable<Var>> = 0>
	constexpr auto derivative() const {
		return (this->_e2 * diff::derivative<Var>(this->_e1) - this->_e1 * diff::derivative<Var>(this->_e2)) / power<2>(this->_e2);
	}
};

} // namespace diff

namespace std {

template<typename E, diff::enable_if<diff::is_expr<E>> = 0>
constexpr auto sin(E e);

template<typename E, diff::enable_if<diff::is_expr<E>> = 0>
constexpr auto cos(E e);

}

namespace diff {

template<typename>
struct Cosine;

// Represent a sine expression
template<typename E>
struct Sine : UnaryExpr<E> {
	using UnaryExpr<E>::UnaryExpr;
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return std::sin(this->_e(args...));
	}
	
	template<typename Var, enable_if<is_variable<Var>> = 0>
	constexpr auto derivative() const {
		return Cosine<E>{this->_e} * diff::derivative<Var>(this->_e);
	}
};

// Represent a cosine expression
template<typename E>
struct Cosine : UnaryExpr<E> {
	using UnaryExpr<E>::UnaryExpr;
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return std::cos(this->_e(args...));
	}
	
	template<typename Var, enable_if<is_variable<Var>> = 0>
	constexpr auto derivative() const {
		return -1 * Sine<E>{this->_e} * diff::derivative<Var>(this->_e);
	}
};

} // namespace diff

namespace std {

template<typename E, diff::enable_if<diff::is_expr<E>>>
constexpr auto sin(E e) {
	return diff::Sine<E>{e};
}

template<typename E, diff::enable_if<diff::is_expr<E>>>
constexpr auto cos(E e) {
	return diff::Cosine<E>{e};
}

}

namespace diff {

/////////////////////////////////////////////
//           Operator Overloading          //
/////////////////////////////////////////////
template<typename E1, typename E2, enable_if<is_expr<E1>> = 0, enable_if<is_expr<E2>> = 0>
constexpr auto operator+(E1 e1, E2 e2) {
	return Addition<E1, E2>{e1, e2};
}

template<typename E, enable_if<is_expr_not_constant<E>> = 0>
constexpr auto operator+(Constant<0>, E e) {
	return e;
}

template<typename E, enable_if<is_expr_not_constant<E>> = 0>
constexpr auto operator+(E e, Constant<0>) {
	return e;
}

template<int n1, int n2>
constexpr auto operator+(Constant<n1>, Constant<n2>) {
	return Constant<n1 + n2>{};
}

template<typename E1, typename E2, enable_if<is_expr<E1>> = 0, enable_if<is_expr<E2>> = 0>
constexpr auto operator-(E1 e1, E2 e2) {
	return Substraction<E1, E2>{e1, e2};
}

template<typename E, enable_if<is_expr_not_constant<E>> = 0>
constexpr auto operator-(Constant<0>, E e) {
	return Constant<-1>{} * e;
}

template<typename E, enable_if<is_expr_not_constant<E>> = 0>
constexpr auto operator-(E e, Constant<0>) {
	return e;
}

template<int n1, int n2>
constexpr auto operator-(Constant<n1>, Constant<n2>) {
	return Constant<n1 - n2>{};
}

template<typename E1, typename E2, enable_if<is_expr<E1>> = 0, enable_if<is_expr<E2>> = 0>
constexpr auto operator*(E1 e1, E2 e2) {
	return Multiplication<E1, E2>{e1, e2};
}

template<typename E, enable_if<is_expr_not_constant<E>> = 0>
constexpr auto operator*(Constant<0>, E) {
	return Constant<0>{};
}

template<typename E, enable_if<is_expr_not_constant<E>> = 0>
constexpr auto operator*(E, Constant<0>) {
	return Constant<0>{};
}

template<typename E, enable_if<is_expr_not_constant<E>> = 0>
constexpr auto operator*(Constant<1>, E e) {
	return e;
}

template<typename E, enable_if<is_expr_not_constant<E>> = 0>
constexpr auto operator*(E e, Constant<1>) {
	return e;
}

template<int n1, int n2>
constexpr auto operator*(Constant<n1>, Constant<n2>) {
	return Constant<n1 * n2>{};
}

template<typename E1, typename E2, enable_if<is_expr<E1>> = 0, enable_if<is_expr<E2>> = 0>
constexpr auto operator/(E1 e1, E2 e2) {
	return Division<E1, E2>{e1, e2};
}

template<typename E, enable_if<is_expr_not_constant<E>> = 0>
constexpr auto operator/(Constant<0>, E) {
	return Constant<0>{};
}

template<typename E, enable_if<is_expr_not_constant<E>> = 0>
constexpr auto operator/(E e, Constant<1>) {
	return e;
}

template<int n1, int n2>
constexpr auto operator/(Constant<n1>, Constant<n2>) {
	return Constant<n1 / n2>{};
}

template<typename E, enable_if<is_expr<E>> = 0>
constexpr auto operator+(E e, double value) {
	return Addition<E, Value>{e, Value{value}};
}

template<typename E, enable_if<is_expr<E>> = 0>
constexpr auto operator-(E e, double value) {
	return Substraction<E, Value>{e, Value{value}};
}

template<typename E, enable_if<is_expr<E>> = 0>
constexpr auto operator*(E e, double value) {
	return Multiplication<E, Value>{e, Value{value}};
}

template<typename E, enable_if<is_expr<E>> = 0>
constexpr auto operator/(E e, double value) {
	return Division<E, Value>{e, Value{value}};
}

template<typename E, enable_if<is_expr<E>> = 0>
constexpr auto operator+(double value, E e) {
	return Addition<Value, E>{Value{value}, e};
}

template<typename E, enable_if<is_expr<E>> = 0>
constexpr auto operator-(double value, E e) {
	return Substraction<Value, E>{Value{value}, e};
}

template<typename E, enable_if<is_expr<E>> = 0>
constexpr auto operator*(double value, E e) {
	return Multiplication<Value, E>{Value{value}, e};
}

template<typename E, enable_if<is_expr<E>> = 0>
constexpr auto operator/(double value, E e) {
	return Division<Value, E>{Value{value}, e};
}

/////////////////////////////////////////////
//        Short variable names/types       //
/////////////////////////////////////////////
using var1_t = Variable<1>;
using var2_t = Variable<2>;
using var3_t = Variable<3>;
using var4_t = Variable<4>;
using var5_t = Variable<5>;
using var6_t = Variable<6>;
using var7_t = Variable<7>;
using var8_t = Variable<8>;
using var9_t = Variable<9>;

constexpr var1_t var1{};
constexpr var2_t var2{};
constexpr var3_t var3{};
constexpr var4_t var4{};
constexpr var5_t var5{};
constexpr var6_t var6{};
constexpr var7_t var7{};
constexpr var8_t var8{};
constexpr var9_t var9{};

} // namespace diff

#endif // GRACICOT_DIFFENTIAL_HPP
