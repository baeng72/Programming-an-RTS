#include "ImGuiWrapper.h"
#include "../Vulkan/VulkanImGuiWrapper.h"
#include "../GL/GLImGuiWrapper.h"
#include "../../Core/Api.h"
namespace ImGui {
	ImGuiWrapper* ImGuiWrapper::Create(Renderer::RenderDevice* pdevice, Core::Window* pwindow)
	{
		switch (Core::GetAPI()) {
		case Core::API::GL:
			return new GL::GLImGuiWrapper(pdevice, pwindow);
		case Core::API::Vulkan:
			return new Vulkan::VulkanImGuiWrapper(pdevice, pwindow);
		}
		assert(0);
		return nullptr;
	}
	ImGuiKey KeycodeToImGuiKey(int32_t keycode) {
		switch (keycode)
		{
		case KEY_TAB: return ImGuiKey_Tab;
		case KEY_LEFT: return ImGuiKey_LeftArrow;
		case KEY_RIGHT: return ImGuiKey_RightArrow;
		case KEY_UP: return ImGuiKey_UpArrow;
		case KEY_DOWN: return ImGuiKey_DownArrow;
		case KEY_PAGE_UP: return ImGuiKey_PageUp;
		case KEY_PAGE_DOWN: return ImGuiKey_PageDown;
		case KEY_HOME: return ImGuiKey_Home;
		case KEY_END: return ImGuiKey_End;
		case KEY_INSERT: return ImGuiKey_Insert;
		case KEY_DELETE: return ImGuiKey_Delete;
		case KEY_BACKSPACE: return ImGuiKey_Backspace;
		case KEY_SPACE: return ImGuiKey_Space;
		case KEY_ENTER: return ImGuiKey_Enter;
		case KEY_ESCAPE: return ImGuiKey_Escape;
		case KEY_APOSTROPHE: return ImGuiKey_Apostrophe;
		case KEY_COMMA: return ImGuiKey_Comma;
		case KEY_MINUS: return ImGuiKey_Minus;
		case KEY_PERIOD: return ImGuiKey_Period;
		case KEY_SLASH: return ImGuiKey_Slash;
		case KEY_SEMICOLON: return ImGuiKey_Semicolon;
		case KEY_EQUAL: return ImGuiKey_Equal;
		case KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket;
		case KEY_BACKSLASH: return ImGuiKey_Backslash;
		case KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
		case KEY_GRAVE_ACCENT: return ImGuiKey_GraveAccent;
		case KEY_CAPS_LOCK: return ImGuiKey_CapsLock;
		case KEY_SCROLL_LOCK: return ImGuiKey_ScrollLock;
		case KEY_NUM_LOCK: return ImGuiKey_NumLock;
		case KEY_PRINT_SCREEN: return ImGuiKey_PrintScreen;
		case KEY_PAUSE: return ImGuiKey_Pause;
		case KEY_KP_0: return ImGuiKey_Keypad0;
		case KEY_KP_1: return ImGuiKey_Keypad1;
		case KEY_KP_2: return ImGuiKey_Keypad2;
		case KEY_KP_3: return ImGuiKey_Keypad3;
		case KEY_KP_4: return ImGuiKey_Keypad4;
		case KEY_KP_5: return ImGuiKey_Keypad5;
		case KEY_KP_6: return ImGuiKey_Keypad6;
		case KEY_KP_7: return ImGuiKey_Keypad7;
		case KEY_KP_8: return ImGuiKey_Keypad8;
		case KEY_KP_9: return ImGuiKey_Keypad9;
		case KEY_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
		case KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
		case KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
		case KEY_KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
		case KEY_KP_ADD: return ImGuiKey_KeypadAdd;
		case KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
		case KEY_KP_EQUAL: return ImGuiKey_KeypadEqual;
		case KEY_LEFT_CONTROL: return ImGuiKey_LeftCtrl;
		case KEY_LEFT_SHIFT: return ImGuiKey_LeftShift;
		case KEY_LEFT_ALT: return ImGuiKey_LeftAlt;
		case KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
		case KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
		case KEY_RIGHT_SHIFT: return ImGuiKey_RightShift;
		case KEY_RIGHT_ALT: return ImGuiKey_RightAlt;
		case KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
		case KEY_MENU: return ImGuiKey_Menu;
		case KEY_0: return ImGuiKey_0;
		case KEY_1: return ImGuiKey_1;
		case KEY_2: return ImGuiKey_2;
		case KEY_3: return ImGuiKey_3;
		case KEY_4: return ImGuiKey_4;
		case KEY_5: return ImGuiKey_5;
		case KEY_6: return ImGuiKey_6;
		case KEY_7: return ImGuiKey_7;
		case KEY_8: return ImGuiKey_8;
		case KEY_9: return ImGuiKey_9;
		case KEY_A: return ImGuiKey_A;
		case KEY_B: return ImGuiKey_B;
		case KEY_C: return ImGuiKey_C;
		case KEY_D: return ImGuiKey_D;
		case KEY_E: return ImGuiKey_E;
		case KEY_F: return ImGuiKey_F;
		case KEY_G: return ImGuiKey_G;
		case KEY_H: return ImGuiKey_H;
		case KEY_I: return ImGuiKey_I;
		case KEY_J: return ImGuiKey_J;
		case KEY_K: return ImGuiKey_K;
		case KEY_L: return ImGuiKey_L;
		case KEY_M: return ImGuiKey_M;
		case KEY_N: return ImGuiKey_N;
		case KEY_O: return ImGuiKey_O;
		case KEY_P: return ImGuiKey_P;
		case KEY_Q: return ImGuiKey_Q;
		case KEY_R: return ImGuiKey_R;
		case KEY_S: return ImGuiKey_S;
		case KEY_T: return ImGuiKey_T;
		case KEY_U: return ImGuiKey_U;
		case KEY_W: return ImGuiKey_W;
		case KEY_X: return ImGuiKey_X;
		case KEY_Y: return ImGuiKey_Y;
		case KEY_Z: return ImGuiKey_Z;
		case KEY_F1: return ImGuiKey_F1;
		case KEY_F2: return ImGuiKey_F2;
		case KEY_F3: return ImGuiKey_F3;
		case KEY_F4: return ImGuiKey_F4;
		case KEY_F5: return ImGuiKey_F5;
		case KEY_F6: return ImGuiKey_F6;
		case KEY_F7: return ImGuiKey_F7;
		case KEY_F8: return ImGuiKey_F8;
		case KEY_F9: return ImGuiKey_F9;
		case KEY_F10: return ImGuiKey_F10;
		case KEY_F11: return ImGuiKey_F11;
		case KEY_F12: return ImGuiKey_F12;
		}
		return ImGuiKey_None;
	}
}