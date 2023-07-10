# Resource

A list of some of the resources I've read:

- [OpenGL Matrix stack](https://stackoverflow.com/questions/13647108/matrix-stacks-in-opengl-deprecated)
- [OpenGL feature level](https://www.quora.com/How-do-you-find-out-which-version-of-OpenGL-your-graphics-card-supports)
- [Uniform Buffer Object](https://gist.github.com/jialiang/2880d4cc3364df117320e8cb324c2880)
- [Uniform Buffet Object - Youtube](https://www.youtube.com/watch?v=JPvbRko9lBg)
- [Learnopengl - Advanced GLSL, Uniform Buffer Object](https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL)
- []()

## OpenGL version fallback with GLFW

OpenGL Context Creation: You can create an OpenGL context using the desired version and feature level and check if it is successfully created. If the context creation fails, it indicates that the requested version or feature level is not supported. You can iterate through different versions and feature levels starting from the highest you require and gradually downgrade until you find a supported configuration.

For example, with GLFW, you can use the glfwWindowHint function to specify the desired OpenGL version and feature level before creating the window and OpenGL context. After creating the context, you can check if it was successfully created and fallback to a lower version or feature level if needed.

```cpp
// Set desired OpenGL version and feature level
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// Create a GLFW window and OpenGL context
GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Context Test", nullptr, nullptr);
if (!window) {
    // Failed to create the requested context, try a lower version or feature level
    // ...
}

// Check the actual OpenGL version and feature level
int major, minor, profile;
glfwGetVersion(&major, &minor, &profile);
std::cout << "OpenGL version: " << major << "." << minor << "\n";
```
