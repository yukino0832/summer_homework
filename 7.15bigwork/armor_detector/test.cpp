#include <iostream>
#include <vector>
#include <cstring>

int main()
{
    std::vector<int> a = {5};
    int *c = &a[0];
    std::cout << *c << std::endl;
    return 0;
}
