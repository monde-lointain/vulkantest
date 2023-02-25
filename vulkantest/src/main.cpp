#include "Application.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

void run_application()
{
	Application app;

	app.initialize();
	app.run();
	app.destroy();
}

int main(int argc, char* args[])
{
#ifdef _MSC_VER
	// Setup the CRT automated memory leak checker
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// Send all reports to STDOUT
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);

	// Set this to the allocation number given by the leak checker to break at
	// it
	//_CrtSetBreakAlloc(163);
#endif

	run_application();

#ifdef _MSC_VER
	// Perform the leak check
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}