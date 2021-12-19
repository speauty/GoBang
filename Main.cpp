#include <iostream>
#include "src/GoBang.h"

int main()
{
	GoBang* gb = new GoBang();
	gb->GameInit();
	while (!gb->GameExec()) {
		gb->GameUpdate();
		gb->GameRender();
	}
	delete gb;
}