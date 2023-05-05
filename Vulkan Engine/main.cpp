#include "apps/application.hpp"
#include "apps/gravity_vec_field_app.hpp"

#include <iostream>

int main()
{
	vk_engine::application app{};
	//vk_engine::gravity_vec_field_app app2{};

	try
	{
		app.run();
		//app2.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
