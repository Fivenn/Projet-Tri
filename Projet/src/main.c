#include "system_utils.h"
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	SU_removeFile("../tmp/test.txt");
	return 1;
}