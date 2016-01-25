#include <iostream>

#include "differential.hpp"

int main(int, char**) {
	using namespace std;
	using namespace diff;
	
	constexpr auto expr1 = square(var1) + 10 / var1;
	constexpr auto expr2 = derivative<var1_t>(expr1);
	
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
	
	std::cout << derivative<var1_t, 4>(power<5>(var1))(1) << std::endl;
	
	return 0;
}
