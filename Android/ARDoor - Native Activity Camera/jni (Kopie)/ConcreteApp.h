#include "AppEngine.h"

class ConcreteApp : public AppEngine
{
private:
	void DrawBackground();

public:
	ConcreteApp(AndroidApp* app) : AppEngine(app) {};

	int Process();
	void OnInitialize();
	void OnInitDisplay();
	void OnFocusGained();
	void OnFocusLost();
};
