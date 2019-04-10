#include "UIManager.h"
#include "ImageComponent.h"
#include "TextComponent.h"
#include <stack>
#include <sstream>

using namespace tinyxml2;

UIComponent* UIManager::_root;
std::map<const std::string, void(*)()> UIManager::_clickFunctions;

UIManager::UIManager(float width, float height) {
    // Create a transparent root element of the UI layout that covers the screen
    _root = new UIComponent(100, 100, 0, 0);
    _root->color = {0,0,0,0};
    _root->screenSize = {width, height};
    _root->screenPosition = {0, 0};

	UIManager::LoadFromXML("../assets/layouts/TestUI.xml");

    Resize();
}

UIManager::~UIManager() {
    delete _root;
}

void UIManager::Resize() {
    _root->Resize();
}

void UIManager::AddToRoot(UIComponent *component) {
    _root->Add(component);
}

UIComponent* UIManager::Root() { return _root; }

UIComponent* UIManager::GetComponentById(std::string id) {
	UIComponent* component = nullptr;

    if (_root != nullptr) {
        std::stack<UIComponent*> stack;
        stack.push(_root);
        while (!stack.empty()) {
            UIComponent* node = stack.top();
            stack.pop();

            if (node->id == id) {
                component = node;
                break;
            }

            for (UIComponent* child : node->children)
                stack.push(child);
        }
    }

	return component;
}

void UIManager::LoadFromXML(const char *filePath) {
    tinyxml2::XMLDocument doc;
    XMLError err = doc.LoadFile(filePath);
    if (err == XML_SUCCESS) {
        XMLElement *root = doc.RootElement();

        XMLElement *child = root->FirstChildElement();
        while (child != nullptr) {
            UIComponent *childComp = readChild(child);
            childComp->parent = _root;
            _root->children.push_back(childComp);
            child = child->NextSiblingElement();
        }
    }
}

UIComponent* UIManager::readChild(const XMLElement* element) {
    std::string type = element->Name();

    const XMLAttribute *offset = element->FindAttribute("offset"),
        *anchor = element->FindAttribute("anchor"),
        *visible = element->FindAttribute("visible"),
        *color = element->FindAttribute("color"),
        *alpha = element->FindAttribute("alpha"),
        *ar = element->FindAttribute("ar"),
        *id = element->FindAttribute("id");

    float offX = 0, offY = 0, w = 0, h = 0;
    AnchorType aTypeX = ANCHOR_PERCENT, aTypeY = ANCHOR_PERCENT;
    VerticalAnchor vA = ANCHOR_TOP;
    HorizontalAnchor hA = ANCHOR_LEFT;
    glm::vec4 col = {0, 0, 0, 0};
    std::string idVal = "";
    bool vis = true;
    std::string clickFunc = "";
    float aspectRatio = 1.0f;

    if (visible != nullptr) {
        vis = visible->BoolValue();
    }

    if (id != nullptr) {
        idVal = id->Value();
    }

    if (ar != nullptr) {
        aspectRatio = ar->FloatValue();
    }

    if (anchor != nullptr) {
        std::istringstream a(anchor->Value());
        std::string val;

        a >> val;
        if (val == "center") {
            vA = ANCHOR_VCENTER;
        } else if (val == "bottom") {
            vA = ANCHOR_BOTTOM;
        }

        a >> val;
        if (val == "center") {
            hA = ANCHOR_HCENTER;
        } else if (val == "right") {
            hA = ANCHOR_RIGHT;
        }
    }

    if (offset != nullptr) {
        std::istringstream off(offset->Value());
        std::string val;

        off >> val;
        offX = std::stof(val);
        if (offX != 0 && val.back() == 'x') {
            aTypeX = ANCHOR_PIXEL;
        }

        off >> val;
        offY = std::stof(val);
        if (offY != 0 && val.back() == 'x') {
            aTypeY = ANCHOR_PIXEL;
        }
    }

    UIComponent *newComponent = nullptr;

    if (type == "component") {
        const XMLAttribute  *width = element->FindAttribute("width"),
            *height = element->FindAttribute("height");

        col = {0, 0, 0, 0};

        UnitType xUnit = UNIT_PERCENT, yUnit = UNIT_PERCENT;

        if (width != nullptr) {
            std::string val = width->Value();
            if (val == "scale") {
                xUnit = UNIT_SCALE;
            } else {
                w = std::stof(val);
                if (val.back() == 'x')
                    xUnit = UNIT_PIXEL;
            }
        }

        if (height != nullptr) {
            std::string val = height->Value();
            if (val == "scale") {
                yUnit = UNIT_SCALE;
            } else {
                h = std::stof(val);
                if (val.back() == 'x')
                    yUnit = UNIT_PIXEL;
            }
        }

        newComponent = new UIComponent(w, h, offX, offY);
        newComponent->xType = xUnit;
        newComponent->yType = yUnit;
        newComponent->aspectRatio = aspectRatio;
    } else if (type == "image") {
        const XMLAttribute  *width = element->FindAttribute("width"),
            *height = element->FindAttribute("height"),
            *src = element->FindAttribute("src");

        col = {1.0, 1.0, 1.0, 1.0};

        std::string srcString = "";
        UnitType xUnit = UNIT_PERCENT, yUnit = UNIT_PERCENT;

        if (width != nullptr) {
            std::string val = width->Value();
            if (val == "scale") {
                xUnit = UNIT_SCALE;
            } else {
                w = std::stof(val);
                if (val.back() == 'x')
                    xUnit = UNIT_PIXEL;
            }
        }

        if (height != nullptr) {
            std::string val = height->Value();
            if (val == "scale") {
                yUnit = UNIT_SCALE;
            } else {
                h = std::stof(val);
                if (val.back() == 'x')
                    yUnit = UNIT_PIXEL;
            }
        }

        if (src != nullptr) {
            srcString = "../UI/images/" + std::string(src->Value());
        }

        newComponent = new ImageComponent(srcString, w, h, offX, offY);
        newComponent->xType = xUnit;
        newComponent->yType = yUnit;
    } else if (type == "text") {
        const XMLAttribute *size = element->FindAttribute("size");
        const char *text = element->GetText();
        if (text == nullptr) {
            text = "";
        }

        col = {1.0, 1.0, 1.0, 1.0};

        UnitType sType = UNIT_PIXEL;
        float s = 0;
        if (size != nullptr) {
            std::string val = size->Value();
            s = std::stof(val);
            if (val.back() == '%')
                sType = UNIT_PERCENT;
        }

        newComponent = new TextComponent(text, s, offX, offY);
        newComponent->yType = sType;
    }

    if (color != nullptr) {
        std::istringstream cStream(color->Value());
        int c;
        cStream >> std::hex >> c;

        col.r = ((c >> 16) & 0xFF) / 255.0f;
        col.g = ((c >> 8) & 0xFF) / 255.0f;
        col.b = (c & 0xFF) / 255.0f;
        col.a = 1.0f;
    }

    if (alpha != nullptr) {
        col.a = std::stof(alpha->Value());

    }

    if (newComponent != nullptr) {
        newComponent->id = idVal;
        newComponent->anchorXType = aTypeX;
        newComponent->anchorYType = aTypeY;
        newComponent->vAnchor = vA;
        newComponent->hAnchor = hA;
        newComponent->visible = vis;
        newComponent->color = col;
    }

    const XMLElement *child = element->FirstChildElement();
    while (child != nullptr) {
        UIComponent *childComponent = readChild(child);
        childComponent->parent = newComponent;
        newComponent->children.push_back(childComponent);
        child = child->NextSiblingElement();
    }

    return newComponent;
}
