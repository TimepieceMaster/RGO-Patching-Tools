#include <stdio.h>

int main(void)
{
	int ret = 0;
	char msg[50] = "";

	printf("Let's make RGO in English! What you gotta say to that!?\n");
	ret = scanf("%s", &msg);
	printf("Wow! \"%s\" indeed!\n", msg);

	return 0;
}
