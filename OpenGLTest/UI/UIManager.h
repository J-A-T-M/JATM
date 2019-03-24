#pragma once

#include <map>
#include <string>
#include <tinyxml2/tinyxml2.h>
#include "AssetLoader.h"
#include "UIComponent.h"

/**
Static structure used to store the UI data, and methods to interact with UI
*/
class UIManager {
public:
    UIManager(float width, float height);
    ~UIManager();
	// Resize the root element (and consequently every element)
    void Resize();
	// Add a new UIComponent to the root component
    static void AddToRoot(UIComponent *component);
	// Get pointer to the root element
    static UIComponent* Root();
	// Search for a component by it's ID string, and return it
	static UIComponent* GetComponentById(std::string id);
	// Reads an XML file and populates the UI system with elements generated from it
    static void LoadFromXML(const char *filePath);
private:
	// Recursive call for reading an XML element into a UIComponent
    static UIComponent* readChild(const tinyxml2::XMLElement* element);
    static UIComponent *_root;
    static std::map<const std::string, void(*)()> _clickFunctions;
};