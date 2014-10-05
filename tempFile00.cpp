#include <stdlib.h>
#include <string>
using namespace std;int main(int argc, char** argv) {
	int i = atoi(argv[1]);
    int r = 1;
	for (int k = 1; k <= i; k++) {
		r <<=1;
	}
	return r;
}
