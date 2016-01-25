#include <iostream>
#include <type_traits>
#include <cmath>
#include <tuple>

struct Expr {};

template<int n, typename T, std::enable_if_t<(n == 1), int> = 0>
constexpr inline auto power(T num) {
	return num;
}

template<int n, typename T, std::enable_if_t<(n > 1), int> = 0>
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

template<typename E, typename... Args>
constexpr double eval(E e, Args... args) {
	return e(args...);
}

template<typename Var, std::size_t level = 1, typename E, std::enable_if_t<(level == 1), int> = 0>
constexpr auto derivative(E e) {
	return e.template derivative<Var>();
}

template<typename Var, std::size_t level, typename E, std::enable_if_t<(level > 1), int> = 0>
constexpr auto derivative(E e) {
	return derivative<Var, 1>(derivative<Var, level - 1>(e));
}

template<bool b>
using bool_type = typename std::conditional<b, std::true_type, std::false_type>::type;

template<typename E1, typename E2>
struct BinaryExpr : Expr {
	constexpr explicit BinaryExpr(E1 e1, E2 e2) : _e1{e1}, _e2{e2} {}
	
	template<typename Var>
	static constexpr bool dependent() {
		return E1::template dependent<Var>() || E2::template dependent<Var>();
	}
	
protected:
	E1 _e1;
	E2 _e2;
};

template<typename E>
struct UnaryExpr : Expr {
	constexpr explicit UnaryExpr(E e) : _e{e} {}
	
	template<typename Var>
	static constexpr bool dependent() {
		return E::template dependent<Var>();
	}
	
protected:
	E _e;
};

struct Value : Expr {
	constexpr Value() = default;
	constexpr explicit Value(double value) : _value{value} {}
	
	template<typename... Args>
	constexpr double operator()(Args...) const {
		return _value;
	}
	
	template<typename Var>
	static constexpr bool dependent() {
		return false;
	}
	
	template<typename Var>
	constexpr auto derivative() const {
		return Value{0};
	}
	
private:
	double _value = 0;
};

template<int n>
struct Variable : Expr {
	constexpr Variable() = default;
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return std::get<n - 1>(std::make_tuple(args...));
	}
	
	template<typename Var>
	static constexpr bool dependent() {
		return std::is_same<Var, Variable<n>>::value;
	}
	
	template<typename Var>
	constexpr auto derivative() const {
		return Value{dependent<Var>() ? 1:0};
	}
};

template<typename E1, typename E2>
struct Multiplication : BinaryExpr<E1, E2> {
	constexpr explicit Multiplication(E1 e1, E2 e2) : BinaryExpr<E1, E2>{e1, e2} {}
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return this->_e1(args...) * this->_e2(args...);
	}
	
	template<typename Var>
	constexpr auto derivative() const {
		return derivative<Var>(
			bool_type<E1::template dependent<Var>()>{},
			bool_type<E2::template dependent<Var>()>{}
		);
	}
	
private:
	template<typename Var>
	constexpr auto derivative(std::true_type, std::true_type) const {
		return this->_e1 * ::derivative<Var>(this->_e2) + ::derivative<Var>(this->_e1) * this->_e2;
	}
	
	template<typename Var>
	constexpr auto derivative(std::false_type, std::true_type) const {
		return this->_e1 * ::derivative<Var>(this->_e2);
	}
	
	template<typename Var>
	constexpr auto derivative(std::true_type, std::false_type) const {
		return ::derivative<Var>(this->_e1) * this->_e2;
	}
	
	template<typename Var>
	constexpr auto derivative(std::false_type, std::false_type) const {
		return Value{0};
	}
};

template<typename E1, typename E2>
struct Addition : BinaryExpr<E1, E2> {
	constexpr explicit Addition(E1 e1, E2 e2) : BinaryExpr<E1, E2>{e1, e2} {}
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return this->_e1(args...) + this->_e2(args...);
	}
	
	template<typename Var>
	constexpr auto derivative() const {
		return derivative<Var>(
			bool_type<E1::template dependent<Var>()>{},
			bool_type<E2::template dependent<Var>()>{}
		);
	}
	
private:
	template<typename Var>
	constexpr auto derivative(std::true_type, std::true_type) const {
		return ::derivative<Var>(this->_e1) + ::derivative<Var>(this->_e2);
	}
	
	template<typename Var>
	constexpr auto derivative(std::false_type, std::true_type) const {
		return ::derivative<Var>(this->_e2);
	}
	
	template<typename Var>
	constexpr auto derivative(std::true_type, std::false_type) const {
		return ::derivative<Var>(this->_e1);
	}
	
	template<typename Var>
	constexpr auto derivative(std::false_type, std::false_type) const {
		return Value{0};
	}
};

template<typename E1, typename E2>
struct Substraction : BinaryExpr<E1, E2> {
	constexpr explicit Substraction(E1 e1, E2 e2) : BinaryExpr<E1, E2>{e1, e2} {}
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return this->_e1(args...) - this->_e2(args...);
	}
	
	template<typename Var>
	constexpr auto derivative() const {
		return derivative<Var>(
			bool_type<E1::template dependent<Var>()>{},
			bool_type<E2::template dependent<Var>()>{}
		);
	}
	
private:
	template<typename Var>
	constexpr auto derivative(std::true_type, std::true_type) const {
		return ::derivative<Var>(this->_e1) - ::derivative<Var>(this->_e2);
	}
	
	template<typename Var>
	constexpr auto derivative(std::false_type, std::true_type) const {
		return -1 * ::derivative<Var>(this->_e2);
	}
	
	template<typename Var>
	constexpr auto derivative(std::true_type, std::false_type) const {
		return ::derivative<Var>(this->_e1);
	}
	
	template<typename Var>
	constexpr auto derivative(std::false_type, std::false_type) const {
		return Value{0};
	}
};

template<typename E1, typename E2>
struct Division : BinaryExpr<E1, E2> {
	constexpr explicit Division(E1 e1, E2 e2) : BinaryExpr<E1, E2>{e1, e2} {}
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return this->_e1(args...) / this->_e2(args...);
	}
	
	template<typename Var>
	constexpr auto derivative() const {
		return derivative<Var>(
			bool_type<E1::template dependent<Var>()>{},
			bool_type<E2::template dependent<Var>()>{}
		);
	}
	
private:
	template<typename Var>
	constexpr auto derivative(std::true_type, std::true_type) const {
		return (this->_e2 * ::derivative<Var>(this->_e1) - this->_e1 * ::derivative<Var>(this->_e2)) / power<2>(this->_e2);
	}
	
	template<typename Var>
	constexpr auto derivative(std::false_type, std::true_type) const {
		return (-1 * this->_e1 * ::derivative<Var>(this->_e2)) / power<2>(this->_e2);
	}
	
	template<typename Var>
	constexpr auto derivative(std::true_type, std::false_type) const {
		return this->_e2 * ::derivative<Var>(this->_e1) / power<2>(this->_e2);
	}
	
	template<typename Var>
	constexpr auto derivative(std::false_type, std::false_type) const {
		return Value{0};
	}
};

namespace std {

template<typename E, std::enable_if_t<std::is_base_of<Expr, E>::value, int> = 0>
constexpr auto sin(E e);

template<typename E, std::enable_if_t<std::is_base_of<Expr, E>::value, int> = 0>
constexpr auto cos(E e);

}

template<typename>
struct Cosine;

template<typename E>
struct Sine : UnaryExpr<E> {
	explicit Sine(E e) : UnaryExpr<E>{e} {}
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return std::sin(this->_e(args...));
	}
	
	template<typename Var>
	constexpr auto derivative() const {
		return Cosine<E>{this->_e} * ::derivative<Var>(this->_e);
	}
};

template<typename E>
struct Cosine : UnaryExpr<E> {
	explicit Cosine(E e) : UnaryExpr<E>{e} {}
	
	template<typename... Args>
	constexpr double operator()(Args... args) const {
		return std::cos(this->_e(args...));
	}
	
	template<typename Var>
	constexpr auto derivative() const {
		return -1 * Sine<E>{this->_e} * ::derivative<Var>(this->_e);
	}
};

namespace std {

template<typename E, std::enable_if_t<std::is_base_of<Expr, E>::value, int>>
constexpr auto sin(E e) {
	return Sine<E>{e};
}

template<typename E, std::enable_if_t<std::is_base_of<Expr, E>::value, int>>
constexpr auto cos(E e) {
	return Cosine<E>{e};
}

}

template<typename E1, typename E2, std::enable_if_t<std::is_base_of<Expr, E1>::value, int> = 0, std::enable_if_t<std::is_base_of<Expr, E2>::value, int> = 0>
constexpr auto operator+(E1 e1, E2 e2) {
	return Addition<E1, E2>{e1, e2};
}

template<typename E1, typename E2, std::enable_if_t<std::is_base_of<Expr, E1>::value, int> = 0, std::enable_if_t<std::is_base_of<Expr, E2>::value, int> = 0>
constexpr auto operator-(E1 e1, E2 e2) {
	return Substraction<E1, E2>{e1, e2};
}

template<typename E1, typename E2, std::enable_if_t<std::is_base_of<Expr, E1>::value, int> = 0, std::enable_if_t<std::is_base_of<Expr, E2>::value, int> = 0>
constexpr auto operator*(E1 e1, E2 e2) {
	return Multiplication<E1, E2>{e1, e2};
}

template<typename E1, typename E2, std::enable_if_t<std::is_base_of<Expr, E1>::value, int> = 0, std::enable_if_t<std::is_base_of<Expr, E2>::value, int> = 0>
constexpr auto operator/(E1 e1, E2 e2) {
	return Division<E1, E2>{e1, e2};
}

template<typename E, std::enable_if_t<std::is_base_of<Expr, E>::value, int> = 0>
constexpr auto operator+(E e, double value) {
	return Addition<E, Value>{e, Value{value}};
}

template<typename E, std::enable_if_t<std::is_base_of<Expr, E>::value, int> = 0>
constexpr auto operator-(E e, double value) {
	return Substraction<E, Value>{e, Value{value}};
}

template<typename E, std::enable_if_t<std::is_base_of<Expr, E>::value, int> = 0>
constexpr auto operator*(E e, double value) {
	return Multiplication<E, Value>{e, Value{value}};
}

template<typename E, std::enable_if_t<std::is_base_of<Expr, E>::value, int> = 0>
constexpr auto operator/(E e, double value) {
	return Division<E, Value>{e, Value{value}};
}

template<typename E, std::enable_if_t<std::is_base_of<Expr, E>::value, int> = 0>
constexpr auto operator+(double value, E e) {
	return Addition<Value, E>{Value{value}, e};
}

template<typename E, std::enable_if_t<std::is_base_of<Expr, E>::value, int> = 0>
constexpr auto operator-(double value, E e) {
	return Substraction<Value, E>{Value{value}, e};
}

template<typename E, std::enable_if_t<std::is_base_of<Expr, E>::value, int> = 0>
constexpr auto operator*(double value, E e) {
	return Multiplication<Value, E>{Value{value}, e};
}

template<typename E, std::enable_if_t<std::is_base_of<Expr, E>::value, int> = 0>
constexpr auto operator/(double value, E e) {
	return Division<Value, E>{Value{value}, e};
}

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

int main(int, char**) {
	using namespace std;
	auto expr1 = square(var1) + 10 / var1;
	auto expr2 = derivative<var1_t>(expr1);
	
	auto expr3 = var1 * var1 + 10 / var1;
	auto expr4 = derivative<var1_t>(expr3);
	
	std::cout << expr1(5) << std::endl;
	std::cout << expr2(5) << std::endl;
	std::cout << expr3(5) << std::endl;
	std::cout << expr4(5) << std::endl;
	
	std::cout << std::endl;
	
	auto expr5 = square(var1) + std::sin(var2) / 4 * var1;
	auto expr6 = derivative<var1_t, 1>(expr5);
	auto expr7 = derivative<var1_t, 2>(expr5);
	auto expr8 = derivative<var2_t, 1>(expr5);
	auto expr9 = derivative<var2_t, 2>(expr5);
	
	std::cout << expr5(14, 3) << std::endl;
	std::cout << expr6(14, 3) << std::endl;
	std::cout << expr7(14, 3) << std::endl;
	std::cout << expr8(14, 3) << std::endl;
	std::cout << expr9(14, 3) << std::endl;
	
	std::cout << std::endl;
	
	std::cout << eval(var1 * var1 / 2, 6) << std::endl;
	
	std::cout << derivative<var1_t, 4>(power<5>(var1) + var1 * 1 * std::sin(var1))(5) << std::endl;
	
	return 0;
}
