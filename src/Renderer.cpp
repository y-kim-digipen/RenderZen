//#include "IRenderer.h"
//
//#include "RendererBackend.h"
//
//namespace IRenderer
//{
//	bool InitRenderer()
//	{
//		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//		{
//			//std::cout << "Failed to initialize GLAD" << std::endl;
//			return false;
//		}
//
//		glEnable(GL_MULTISAMPLE);
//
//		return true;
//	}
//
//	void SetClearColor(glm::vec4 color)
//	{
//		glClearColor(color.r, color.g, color.b, color.a);
//	}
//}
