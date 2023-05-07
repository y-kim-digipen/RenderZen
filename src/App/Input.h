#pragma once
#pragma once
#include <map>
#include <glm/vec2.hpp>
#include <GLFW/glfw3.h>

namespace Input {
	class Button
	{
		friend class Input;
		friend class Mouse;
	public:
		bool IsDown () const {
			return current;
		}
		bool IsReleased () const {
			return !current && old;
		}
		bool IsPressed () const {
			return !old && current;
		}

	private:
		void Update () {
			old = current;
		}
		bool old, current;
	};

	class Mouse
	{
		friend class Input;
	public:
		glm::ivec2 GetCursorPos () const {
			return current_cursor_pos;
		}
		glm::ivec2 GetCursorMove () const {
			return current_cursor_pos - old_cursor_pos;
		}
	private:
		void Update ();
		std::map<uint64_t, Button> mouse_buttons;

		glm::ivec2 old_cursor_pos {};
		glm::ivec2 current_cursor_pos {};
	};

	class Input
	{
		friend class Engine;
	public:
		enum Modes
		{
			PRESSED, RELEASED
		};

		static const Mouse& GetMouse () {
			return mouse;
		}
		static const Button& GetKeyboardButton ( int32_t key ) {
			return keyboard_buttons[ key ];
		}
		static const Button& GetMouseButton ( int32_t key ) {
			return mouse.mouse_buttons[ key ];
		}

		static void GLFW_KeyboardCallback ( GLFWwindow* window, int key, int scancode, int action, int mods );
		static void GLFW_CursorCallback ( GLFWwindow* window, double xpos, double ypos );
		static void GLFW_MouseButtonCallback ( GLFWwindow* window, int button, int action, int mods );
		static void Init ( GLFWwindow* window );
		static void Update ();
	private:
		static inline std::map<int32_t, Button> keyboard_buttons;
		static inline Mouse mouse;
	};
}
