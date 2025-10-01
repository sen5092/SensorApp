#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>



// Add dummy usage to ensure they are linked
void force_link_sources() {
    //(void)sizeof(Point);
    //(void)sizeof(HashMap);
    //(void)sizeof(CanGoWrong);
    //(void)sizeof(ExceptionPractice);
    //(void)sizeof(STLPractice);
    //(void)sizeof(BinaryFiles);
}


TEST_CASE("Force link", "[force_link]") {
    force_link_sources();
}
